// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2026 Chao Peng, Sylvester Joosten, Wouter Deconinck, Chao, Whitney Armstrong, Aiden Wu

#include "CalorimeterCALOROCSingleEndedReco.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <DD4hep/Shapes.h>
#include <DD4hep/VolumeManager.h>
#include <DD4hep/Volumes.h>
#include <DD4hep/detail/SegmentationsInterna.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <DDSegmentation/MultiSegmentation.h>
#include <DDSegmentation/Segmentation.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <algorithms/service.h>
#include <edm4eic/CALOROC1ASample.h>
#include <edm4eic/CALOROC1BSample.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector3f.h>
#include <fmt/ranges.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/digi/CALOROCDigitizationConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

namespace eicrecon {

void CalorimeterCALOROCSingleEndedReco::init() {
  if (m_cfg.calorocType != "1A" && m_cfg.calorocType != "1B") {
    throw std::runtime_error("calorocType must be 1A or 1B");
  }
  if (m_cfg.totEnergyParameters.size() != 3) {
    throw std::runtime_error("totEnergyParameters requires three coefficients");
  }
  if (m_cfg.timeWalkCor && m_cfg.timeWalkCorrectionParameters.size() != 4) {
    throw std::runtime_error("timeWalkCorrectionParameters requires four coefficients");
  }

  // Load the readout fields.
  m_id_spec    = m_detector->readout(m_cfg.readout).idSpec();
  m_id_decoder = m_id_spec.decoder();
  if (!m_cfg.layerField.empty()) {
    m_layer_index = m_id_decoder->index(m_cfg.layerField);
  }

  // --------------- Adapted from CalorimeterHitReco.cc ---------------
  std::function hit_to_map = [this](std::uint64_t cellID) {
    std::unordered_map<std::string, double> params;
    for (const auto& field : m_id_spec.fields()) {
      params.emplace(field.first, field.second->value(cellID));
    }
    return params;
  };
  auto& serviceSvc = algorithms::ServiceSvc::instance();
  sampFrac = serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")->compile(m_cfg.sampFrac, hit_to_map);
  // ------------------------------------------------------------------
}

void CalorimeterCALOROCSingleEndedReco::process(
    const CalorimeterCALOROCSingleEndedReco::Input& input,
    const CalorimeterCALOROCSingleEndedReco::Output& output) const {
  const auto [caloroc_hits, pulses, source_hits]  = input;
  auto [reco_hits, raw_hits, links, associations] = output;

  // Loop over digitized hits and their source pulses.
  for (std::size_t hit_index = 0; hit_index < caloroc_hits->size(); hit_index++) {
    const auto caloroc_hit = (*caloroc_hits)[hit_index];
    const auto pulse       = (*pulses)[hit_index];
    const auto cellID      = caloroc_hit.getCellID();

    double adc_sum                 = 0;
    double tot                     = 0;
    bool adc_saturated             = false;
    double time                    = pulse.getTime();
    bool found_time                = false;
    const double sample_phase      = caloroc_hit.getSamplePhase() /
                                     static_cast<double>(m_cfg.caloroc.capTOA) *
                                     m_cfg.caloroc.dyRangeTOA;
    const std::size_t sample_count = m_cfg.calorocType == "1A" ? caloroc_hit.getASamples().size()
                                                               : caloroc_hit.getBSamples().size();

    // Loop over samples.
    for (std::size_t sample_index = 0; sample_index < sample_count; sample_index++) {
      unsigned int toa = 0;

      // 1A: sum pedestal-subtracted ADC amplitudes and read the waveform ToT.
      if (m_cfg.calorocType == "1A") {
        const auto sample = caloroc_hit.getASamples(sample_index);
        adc_sum += sample.ADC - m_cfg.pedestalADC;
        if (tot == 0) {
          tot = sample.timeOverThreshold;
        }
        adc_saturated |= sample.ADC >= m_cfg.caloroc.capADC - 1;
        toa = sample.timeOfArrival;
      } else {

        // 1B: convert the selected gain to single-gain ADC counts, then subtract the pedestal.
        const auto sample = caloroc_hit.getBSamples(sample_index);
        if (sample.highGainADC < m_cfg.caloroc.capADC - 1) {
          adc_sum += sample.highGainADC * m_cfg.caloroc.dyRangeHighGainADC /
                         m_cfg.caloroc.dyRangeSingleGainADC -
                     m_cfg.pedestalADC;
        } else {
          adc_sum += sample.lowGainADC * m_cfg.caloroc.dyRangeLowGainADC /
                         m_cfg.caloroc.dyRangeSingleGainADC -
                     m_cfg.pedestalADC;
        }
        toa = sample.timeOfArrival;
      }

      // Set the hit time from the first measured arrival.
      if (!found_time && toa > 0) {
        time       = sample_phase +
                     (caloroc_hit.getTimeStamp() + sample_index) * m_cfg.caloroc.time_window -
                     toa / static_cast<double>(m_cfg.caloroc.capTOA) * m_cfg.caloroc.dyRangeTOA;
        found_time = true;
      }
    }

    // Correct measured arrival times using an unsaturated ADC sum.
    if (m_cfg.timeWalkCor && found_time && !adc_saturated) {
      const auto& parameters = m_cfg.timeWalkCorrectionParameters;
      const double charge    = adc_sum - parameters[2];
      if (charge > 0) {
        time -=
            (parameters[0] + parameters[1] * std::pow(charge, parameters[3])) * edm4eic::unit::ns;
      }
    }

    // Convert the ADC sum or waveform ToT to energy.
    double energy;
    if (adc_saturated) {

      // Decode an unclipped ToT and apply the visible-energy calibration.
      if (tot <= 0 || tot >= m_cfg.caloroc.capTOT - 1) {
        continue;
      }
      const double tot_time =
          tot / static_cast<double>(m_cfg.caloroc.capTOT) * m_cfg.caloroc.dyRangeTOT;
      energy = m_cfg.totEnergyParameters[0] + m_cfg.totEnergyParameters[1] * tot_time +
               m_cfg.totEnergyParameters[2] * tot_time * tot_time;
    } else {
      energy = adc_sum * m_cfg.adcToEnergy;
    }

    if (energy <= 0) {
      continue;
    }

    // Apply the sampling correction.
    energy /= sampFrac(cellID);

    // --------------- Adapted from CalorimeterHitReco.cc ---------------
    // get layer ID
    const int lid = m_id_decoder != nullptr && !m_cfg.layerField.empty()
                        ? static_cast<int>(m_id_decoder->get(cellID, m_layer_index))
                        : -1;

    dd4hep::DetElement local;
    dd4hep::Position gpos;
    // global positions
    gpos = m_converter->position(cellID);

    // local positions
    auto volman = m_detector->volumeManager();
    local       = volman.lookupDetElement(cellID);

    const auto pos = local.nominal().worldToLocal(gpos);
    std::vector<double> cdim;
    // get segmentation dimensions

    const dd4hep::DDSegmentation::Segmentation* segmentation =
        m_converter->findReadout(local).segmentation()->segmentation;
    auto segmentation_type = segmentation->type();

    while (segmentation_type == "MultiSegmentation") {
      const auto* multi_segmentation =
          dynamic_cast<const dd4hep::DDSegmentation::MultiSegmentation*>(segmentation);
      const dd4hep::DDSegmentation::Segmentation& sub_segmentation =
          multi_segmentation->subsegmentation(cellID);

      segmentation      = &sub_segmentation;
      segmentation_type = segmentation->type();
    }

    if (segmentation_type == "CartesianGridXY" || segmentation_type == "HexGridXY" ||
        segmentation_type == "CartesianGridXYStaggered") {
      auto cell_dim = m_converter->cellDimensions(cellID);
      cdim.resize(3);
      cdim[0] = cell_dim[0];
      cdim[1] = cell_dim[1];
      debug("Using segmentation for cell dimensions: {}", fmt::join(cdim, ", "));
    } else if (segmentation_type == "CartesianStripZ") {
      auto cell_dim = m_converter->cellDimensions(cellID);
      cdim.resize(3);
      cdim[2] = cell_dim[0];
      debug("Using segmentation for cell dimensions: {}", fmt::join(cdim, ", "));
    } else {
      if ((segmentation_type != "NoSegmentation") && (!warned_unsupported_segmentation)) {
        warning("Unsupported segmentation type \"{}\"", segmentation_type);
        warned_unsupported_segmentation = true;
      }

      // Using bounding box instead of actual solid so the dimensions are always in dim_x, dim_y, dim_z
      cdim =
          m_converter->findContext(cellID)->volumePlacement().volume().boundingBox().dimensions();
      std::ranges::transform(cdim, cdim.begin(), [](auto&& PH1) {
        return std::multiplies<double>()(std::forward<decltype(PH1)>(PH1), 2);
      });
      debug("Using bounding box for cell dimensions: {}", fmt::join(cdim, ", "));
    }

    //create constant vectors for passing to hit initializer list
    //FIXME: needs to come from the geometry service/converter
    const decltype(edm4eic::CalorimeterHitData::position) position(
        gpos.x() / dd4hep::mm, gpos.y() / dd4hep::mm, gpos.z() / dd4hep::mm);
    const decltype(edm4eic::CalorimeterHitData::dimension) dimension(
        cdim.at(0) / dd4hep::mm, cdim.at(1) / dd4hep::mm, cdim.at(2) / dd4hep::mm);
    const decltype(edm4eic::CalorimeterHitData::local) local_position(
        pos.x() / dd4hep::mm, pos.y() / dd4hep::mm, pos.z() / dd4hep::mm);

    //-------------------------------------------------------------------

    // Create the raw hit with its ADC sum and time.
    auto raw_hit = raw_hits->create();
    raw_hit.setCellID(cellID);
    raw_hit.setAmplitude(std::llround(adc_sum));
    raw_hit.setTimeStamp(std::max(std::llround(time / edm4eic::unit::ns), 0LL));

    // Create the reconstructed hit and attach its raw hit.
    auto reco_hit =
        reco_hits->create(cellID, energy, 0, time, 0, position, dimension, -1, lid, local_position);
    reco_hit.setRawHit(raw_hit);

    // Loop over source hits to normalize their truth-link weights.
    double total_response = 0;
    for (const auto hit : pulse.getCalorimeterHits()) {
      total_response += hit.getEnergy();
    }
    if (total_response <= 0) {
      continue;
    }
    // Preserve response weights and link to the original energy-deposition hits.
    for (const auto hit : pulse.getCalorimeterHits()) {
      const double weight = hit.getEnergy() / total_response;
      const auto source   = source_hits->at(hit.getObjectID().index);
      auto link           = links->create();
      link.setFrom(raw_hit);
      link.setTo(source);
      link.setWeight(weight);

      auto association = associations->create();
      association.setRawHit(raw_hit);
      association.setSimHit(source);
      association.setWeight(weight);
    }
  }
}

} // namespace eicrecon
