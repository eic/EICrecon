// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Wouter Deconinck, Chao, Whitney Armstrong

// Reconstruct digitized outputs, paired with Jug::Digi::CalorimeterHitDigi
// Author: Chao Peng
// Date: 06/14/2021

#include "CalorimeterHitReco.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/Handle.h>
#include <DD4hep/IDDescriptor.h>
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
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <algorithm>
#include <cctype>
#include <gsl/pointers>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitRecoConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

using namespace dd4hep;

namespace eicrecon {

void CalorimeterHitReco::init() {

  // threshold for firing
  // Should set either m_cfg.thresholdFactor or m_cfg.thresholdValue, not both
  if (m_cfg.thresholdFactor * m_cfg.thresholdValue != 0) {
    error("thresholdFactor = {}, thresholdValue = {}. Only one of these should be non-zero.",
          m_cfg.thresholdFactor, m_cfg.thresholdValue);
    throw; // throw with an argument doesn't trigger abort
  }
  thresholdADC = m_cfg.thresholdFactor * m_cfg.pedSigmaADC + m_cfg.thresholdValue;
  // TDC channels to timing conversion
  stepTDC = dd4hep::ns / m_cfg.resolutionTDC;

  // do not get the layer/sector ID if no readout class provided
  if (m_cfg.readout.empty()) {
    return;
  }

  // First, try and get the IDDescriptor. This will throw an exception if it fails.
  try {
    id_spec = m_detector->readout(m_cfg.readout).idSpec();
  } catch (...) {
    warning("Failed to get idSpec for {}", m_cfg.readout);
    return;
  }
  // Next, try and get the readout fields. This will throw a different exception.
  try {
    id_dec = id_spec.decoder();
    if (!m_cfg.sectorField.empty()) {
      sector_idx = id_dec->index(m_cfg.sectorField);
      debug("Find sector field {}, index = {}", m_cfg.sectorField, sector_idx);
    }
    if (!m_cfg.layerField.empty()) {
      layer_idx = id_dec->index(m_cfg.layerField);
      debug("Find layer field {}, index = {}", m_cfg.layerField, sector_idx);
    }
    if (!m_cfg.maskPosFields.empty()) {
      std::size_t tmp_mask = 0;
      for (auto& field : m_cfg.maskPosFields) {
        tmp_mask |= id_spec.field(field)->mask();
      }
      // assign this mask if all fields succeed
      gpos_mask = tmp_mask;
    }
  } catch (...) {
    if (id_dec == nullptr) {
      warning("Failed to load ID decoder for {}", m_cfg.readout);
      std::stringstream readouts;
      for (auto r : m_detector->readouts()) {
        readouts << "\"" << r.first << "\", ";
      }
      warning("Available readouts: {}", readouts.str());
    } else {
      warning("Failed to find field index for {}.", m_cfg.readout);
      if (!m_cfg.sectorField.empty()) {
        warning(" -- looking for sector field \"{}\".", m_cfg.sectorField);
      }
      if (!m_cfg.layerField.empty()) {
        warning(" -- looking for layer field  \"{}\".", m_cfg.layerField);
      }
      if (!m_cfg.maskPosFields.empty()) {
        warning(" -- looking for masking fields  \"{}\".", fmt::join(m_cfg.maskPosFields, ", "));
      }
      std::stringstream fields;
      for (auto field : id_spec.decoder()->fields()) {
        fields << "\"" << field.name() << "\", ";
      }
      warning("Available fields: {}", fields.str());
      warning("n.b. The local position, sector id and layer id will not be correct for this.");
      warning("Position masking may not be applied.");
      warning("however, the position, energy, and time values should still be good.");
    }

    return;
  }

  id_spec = m_detector->readout(m_cfg.readout).idSpec();

  std::function hit_to_map = [this](const edm4hep::RawCalorimeterHit& h) {
    std::unordered_map<std::string, double> params;
    for (const auto& p : id_spec.fields()) {
      const std::string& name                  = p.first;
      const dd4hep::IDDescriptor::Field* field = p.second;
      params.emplace(name, field->value(h.getCellID()));
      trace("{} = {}", name, field->value(h.getCellID()));
    }
    return params;
  };

  auto& serviceSvc = algorithms::ServiceSvc::instance();
  sampFrac = serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")->compile(m_cfg.sampFrac, hit_to_map);

  // local detector name has higher priority
  if (!m_cfg.localDetElement.empty()) {
    try {
      m_local = m_detector->detector(m_cfg.localDetElement);
      info("local coordinate system from DetElement {}", m_cfg.localDetElement);
    } catch (...) {
      error("failed to load local coordinate system from DetElement {}", m_cfg.localDetElement);
      return;
    }
  } else {
    std::vector<std::pair<std::string, int>> fields;
    for (auto f : m_cfg.localDetFields) {
      fields.emplace_back(f, 0);
    }
    local_mask = id_spec.get_mask(fields);
    // use all fields if nothing provided
    if (fields.empty()) {
      local_mask = ~static_cast<decltype(local_mask)>(0);
    }
  }
}

void CalorimeterHitReco::process(const CalorimeterHitReco::Input& input,
                                 const CalorimeterHitReco::Output& output) const {

  const auto [rawhits] = input;
  auto [recohits]      = output;

  // For some detectors, the cellID in the raw hits may be broken
  // (currently this is the HcalBarrel). In this case, dd4hep
  // prints an error message and throws an exception. We catch
  // the exception and handle it, but the screen gets flooded
  // with these messages. Keep a count of these and if a max
  // number is encountered disable this algorithm. A useful message
  // indicating what is going on is printed below where the
  // error is detector.
  if (NcellIDerrors >= MaxCellIDerrors) {
    return;
  }

  for (const auto& rh : *rawhits) {

    //did not pass the zero-suppresion threshold
    const auto cellID = rh.getCellID();
    if (rh.getAmplitude() < m_cfg.pedMeanADC + thresholdADC) {
      continue;
    }

    if (rh.getAmplitude() > static_cast<int>(m_cfg.capADC)) {
      error("Encountered hit with amplitude {} outside of ADC capacity {}", rh.getAmplitude(),
            m_cfg.capADC);
      continue;
    }

    // get layer and sector ID
    const int lid = id_dec != nullptr && !m_cfg.layerField.empty()
                        ? static_cast<int>(id_dec->get(cellID, layer_idx))
                        : -1;
    const int sid = id_dec != nullptr && !m_cfg.sectorField.empty()
                        ? static_cast<int>(id_dec->get(cellID, sector_idx))
                        : -1;

    // convert ADC to energy
    float sampFrac_value = sampFrac(rh);
    float energy         = (((signed)rh.getAmplitude() - (signed)m_cfg.pedMeanADC)) /
                   static_cast<float>(m_cfg.capADC) * m_cfg.dyRangeADC / sampFrac_value;

    const float time = rh.getTimeStamp() / stepTDC;
    trace("cellID {}, \t energy: {},  TDC: {}, time: {}, sampFrac: {}", cellID, energy,
          rh.getTimeStamp(), time, sampFrac_value);

    dd4hep::DetElement local;
    dd4hep::Position gpos;
    try {
      // global positions
      gpos = m_converter->position(cellID);

      // masked position (look for a mother volume)
      if (gpos_mask != 0) {
        auto mpos = m_converter->position(cellID & ~gpos_mask);
        // replace corresponding coords
        for (const char& c : m_cfg.maskPos) {
          switch (std::tolower(c)) {
          case 'x':
            gpos.SetX(mpos.X());
            break;
          case 'y':
            gpos.SetY(mpos.Y());
            break;
          case 'z':
            gpos.SetZ(mpos.Z());
            break;
          default:
            break;
          }
        }
      }

      // local positions
      if (m_cfg.localDetElement.empty()) {
        auto volman = m_detector->volumeManager();
        local       = volman.lookupDetElement(cellID & local_mask);
      } else {
        local = m_local;
      }
    } catch (...) {
      // Error looking up cellID. Messages should already have been printed.
      // Also, see comment at top of this method.
      if (++NcellIDerrors >= MaxCellIDerrors) {
        error("Maximum number of errors reached: {}", MaxCellIDerrors);
        error("This is likely an issue with the cellID being unknown.");
        error("Note: local_mask={:X} example cellID={:x}", local_mask, cellID);
        error("Disabling this algorithm since it requires a valid cellID.");
        error("(See {}:{})", __FILE__, __LINE__);
      }
      continue;
    }

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

    if (segmentation_type == "CartesianGridXY" || segmentation_type == "HexGridXY") {
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
      std::transform(cdim.begin(), cdim.end(), cdim.begin(), [](auto&& PH1) {
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

    auto recohit = recohits->create(rh.getCellID(), energy, 0, time, 0, position, dimension, sid,
                                    lid, local_position);
    recohit.setRawHit(rh);
  }
}

} // namespace eicrecon
