// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim, Aiden Wu

#include "CalorimeterCALOROCReco.h"

#include <DD4hep/Alignments.h>
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
#include <cctype>
#include <cmath>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterCALOROCRecoConfig.h"
#include "algorithms/digi/CALOROCDigitizationConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

using namespace dd4hep;

namespace eicrecon {

void CalorimeterCALOROCReco::init() {
  if (m_cfg.calorocType != "1A" && m_cfg.calorocType != "1B") {
    throw std::runtime_error("calorocType must be 1A or 1B");
  }
  if (m_cfg.energyCalibration == CalorimeterCALOROCRecoConfig::EnergyCalibration::ADC) {
    if (m_cfg.calorocType == "1A" && m_cfg.totEnergyParameters.size() != 3) {
      throw std::runtime_error("totEnergyParameters requires three coefficients");
    }
    if (m_cfg.timeWalkCor && m_cfg.timeWalkCorrectionParameters.size() != 4) {
      throw std::runtime_error("timeWalkCorrectionParameters requires four coefficients");
    }
  }

  // do not get the layer/sector ID if no readout class provided
  if (m_cfg.readout.empty()) {
    error("You MUST provide the name of the readout to decode CellID.");
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
      debug("Find layer field {}, index = {}", m_cfg.layerField, layer_idx);
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

  // local detector name has higher priority
  if (!m_cfg.localDetElement.empty()) {
    try {
      m_local = m_detector->detector(m_cfg.localDetElement);
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

  // Compile the sampling fraction as a function of cell-ID fields.
  if (m_cfg.energyCalibration == CalorimeterCALOROCRecoConfig::EnergyCalibration::ADC) {
    std::function hit_to_map = [this](std::uint64_t cellID) {
      std::unordered_map<std::string, double> params;
      for (const auto& field : id_spec.fields()) {
        params.emplace(field.first, field.second->value(cellID));
      }
      return params;
    };
    auto& serviceSvc = algorithms::ServiceSvc::instance();
    sampFrac =
        serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")->compile(m_cfg.sampFrac, hit_to_map);
    return;
  }

  // Get the field indices and field-dependent conversion factors if necessary
  if (m_cfg.edep_to_npe_fields.empty() || m_cfg.edep_to_npe_filename.empty()) {
    error("You MUST provide edep_to_npe files and filename for layer-by-layer calibration.");
  }

  for (const auto& field : m_cfg.edep_to_npe_fields) {
    auto field_idx = id_dec->index(field);
    m_field_idxs.push_back(field_idx);
  }

  std::ifstream infile(m_cfg.edep_to_npe_filename);
  if (!infile) {
    error("Unable to open LUT file: {}", m_cfg.edep_to_npe_filename);
  }
  std::string line;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    std::vector<int> keys;
    for (std::size_t i = 0; i < m_cfg.edep_to_npe_fields.size(); i++) {
      int value;
      iss >> value;
      keys.push_back(value);
    }
    double factor = 0.0;
    if (!(iss >> factor)) {
      throw std::runtime_error("Malformed LUT file: " + m_cfg.edep_to_npe_filename);
    }
    m_edep_to_npe_lut[keys] = factor;
  }
}

double CalorimeterCALOROCReco::_toa(const edm4eic::RawCALOROCHit& hit, std::size_t arrival_index,
                                    unsigned int toa) const {
  const double toa_step     = m_cfg.caloroc.dyRangeTOA / m_cfg.caloroc.capTOA;
  const double sample_phase = hit.getSamplePhase() * toa_step;

  // A missing arrival uses the sample count as its index and zero ToA.
  return sample_phase + (hit.getTimeStamp() + arrival_index) * m_cfg.caloroc.time_window -
         toa * toa_step;
}

CalorimeterCALOROCReco::SignalMeasurements
CalorimeterCALOROCReco::_reconstruct1A(const edm4eic::RawCALOROCHit& caloroc_hit) const {
  SignalMeasurements result;
  const std::size_t sample_count = caloroc_hit.getASamples().size();
  std::size_t arrival_index      = sample_count;
  unsigned int toa               = 0;

  // Loop over samples.
  for (std::size_t sample_index = 0; sample_index < sample_count; sample_index++) {
    const auto sample = caloroc_hit.getASamples(sample_index);
    result.adc_sum += sample.ADC - m_cfg.pedestalADC;
    if (result.tot == 0) {
      result.tot = sample.timeOverThreshold;
    }
    result.adc_saturated |= sample.ADC >= m_cfg.caloroc.capADC - 1;

    // Decode the first measured arrival.
    if (!result.found_time && sample.timeOfArrival > 0) {
      arrival_index     = sample_index;
      toa               = sample.timeOfArrival;
      result.found_time = true;
    }
  }

  result.time = _toa(caloroc_hit, arrival_index, toa);
  return result;
}

CalorimeterCALOROCReco::SignalMeasurements
CalorimeterCALOROCReco::_reconstruct1B(const edm4eic::RawCALOROCHit& ADC) const {
  SignalMeasurements result;
  CALOROC1BParameters parameters{
      .high_gain_limit = m_cfg.caloroc.capADC - 1,
      .gain_ratio      = m_cfg.caloroc.dyRangeLowGainADC / m_cfg.caloroc.dyRangeHighGainADC,
      .charge_scale    = m_cfg.caloroc.dyRangeLowGainADC / m_cfg.caloroc.dyRangeSingleGainADC,
      .pedestal        = m_cfg.pedestalADC,
  };
  if (m_cfg.energyCalibration == CalorimeterCALOROCRecoConfig::EnergyCalibration::NpeLUT) {
    parameters = _getLUT1BParameters();
  }
  const std::size_t sample_count = ADC.getBSamples().size();
  std::size_t arrival_index      = sample_count;
  unsigned int toa               = 0;

  // Loop over dual-gain samples.
  for (std::size_t sample_index = 0; sample_index < sample_count; ++sample_index) {
    const auto bSample = ADC.getBSamples(sample_index);
    if (bSample.highGainADC >= parameters.high_gain_limit) {
      result.adc_saturated |= bSample.lowGainADC >= m_cfg.caloroc.capADC - 1;
      result.adc_sum += static_cast<double>(bSample.lowGainADC);
    } else {
      result.adc_sum += static_cast<double>(bSample.highGainADC) / parameters.gain_ratio;
    }

    // Decode the first measured arrival.
    if (!result.found_time && bSample.timeOfArrival > 0) {
      arrival_index     = sample_index;
      toa               = bSample.timeOfArrival;
      result.found_time = true;
    }
  }

  // Apply the charge scale and per-sample pedestal for the selected calibration.
  result.adc_sum *= parameters.charge_scale;
  result.adc_sum -= sample_count * parameters.pedestal;
  result.time = _toa(ADC, arrival_index, toa);
  return result;
}

CalorimeterCALOROCReco::CALOROC1BParameters CalorimeterCALOROCReco::_getLUT1BParameters() const {
  return {.high_gain_limit = m_cfg.highGainDR,
          .gain_ratio      = m_cfg.gainRatio,
          .charge_scale    = 1,
          .pedestal        = 0};
}

std::optional<double>
CalorimeterCALOROCReco::_calibrateEnergyADC(std::uint64_t cellID,
                                            const SignalMeasurements& samples) const {
  double energy = 0.0;

  // Recover saturated signals from an unclipped ToT measurement.
  if (samples.adc_saturated) {
    if (m_cfg.calorocType == "1B" || samples.tot <= 0 || samples.tot >= m_cfg.caloroc.capTOT - 1) {
      return std::nullopt;
    }
    const double tot_time =
        samples.tot / static_cast<double>(m_cfg.caloroc.capTOT) * m_cfg.caloroc.dyRangeTOT;
    energy = m_cfg.totEnergyParameters[0] + m_cfg.totEnergyParameters[1] * tot_time +
             m_cfg.totEnergyParameters[2] * tot_time * tot_time;
  } else {
    energy = samples.adc_sum * m_cfg.adcToEnergy;
  }
  if (energy <= 0) {
    return std::nullopt;
  }

  // Convert visible energy to deposited energy using the sampling fraction.
  return energy / sampFrac(cellID);
}

std::optional<double> CalorimeterCALOROCReco::_calibrateEnergyLUT(std::uint64_t cellID,
                                                                  double charge) const {

  // Decode the cell-ID fields used to select the calibration factor.
  std::vector<int> fields;
  fields.reserve(m_field_idxs.size());
  for (const auto index : m_field_idxs) {
    fields.push_back(static_cast<int>(id_dec->get(cellID, index)));
  }
  const auto factor = m_edep_to_npe_lut.find(fields);
  if (factor == m_edep_to_npe_lut.end() || factor->second == 0.0) {
    warning("Missing/invalid edep->npe factor for cellID={}", cellID);
    return std::nullopt;
  }

  // Apply the layer gain and linear energy calibration.
  return (charge / factor->second) * m_cfg.slope + m_cfg.intercept;
}

double CalorimeterCALOROCReco::_timeWalkCorrection(const SignalMeasurements& samples) const {
  if (!m_cfg.timeWalkCor) {
    return samples.time;
  }

  // ADC-based time walk requires a measured arrival and unsaturated charge.
  if (m_cfg.energyCalibration == CalorimeterCALOROCRecoConfig::EnergyCalibration::ADC &&
      (!samples.found_time || samples.adc_saturated)) {
    return samples.time;
  }
  const auto& parameters = m_cfg.timeWalkCorrectionParameters;
  const double charge    = samples.adc_sum - parameters[2];
  if (charge > 0) {
    return samples.time - (parameters[1] * std::pow(charge, parameters[3]) + parameters[0]);
  }
  return samples.time;
}

std::optional<CalorimeterCALOROCReco::HitGeometry>
CalorimeterCALOROCReco::_getHitGeometry(std::uint64_t cellID) const {

  // get layer and sector ID
  const int lid = id_dec != nullptr && !m_cfg.layerField.empty()
                      ? static_cast<int>(id_dec->get(cellID, layer_idx))
                      : -1;
  const int sid = id_dec != nullptr && !m_cfg.sectorField.empty()
                      ? static_cast<int>(id_dec->get(cellID, sector_idx))
                      : -1;

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
    if (m_cfg.energyCalibration == CalorimeterCALOROCRecoConfig::EnergyCalibration::ADC) {
      throw;
    }
    return std::nullopt;
  }

  const auto lpos = local.nominal().worldToLocal(gpos);
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
    cdim = m_converter->findContext(cellID)->volumePlacement().volume().boundingBox().dimensions();
    std::ranges::transform(cdim, cdim.begin(), [](auto&& PH1) {
      return std::multiplies<double>()(std::forward<decltype(PH1)>(PH1), 2);
    });
    debug("Using bounding box for cell dimensions: {}", fmt::join(cdim, ", "));
  }

  // Convert geometry coordinates and dimensions to EDM units.
  const decltype(edm4eic::CalorimeterHitData::position) position(
      gpos.x() * edm4eic::unit::mm / dd4hep::mm, gpos.y() * edm4eic::unit::mm / dd4hep::mm,
      gpos.z() * edm4eic::unit::mm / dd4hep::mm);
  const decltype(edm4eic::CalorimeterHitData::dimension) dimension(
      cdim.at(0) * edm4eic::unit::mm / dd4hep::mm, cdim.at(1) * edm4eic::unit::mm / dd4hep::mm,
      cdim.at(2) * edm4eic::unit::mm / dd4hep::mm);
  const decltype(edm4eic::CalorimeterHitData::local) local_position(
      lpos.x() * edm4eic::unit::mm / dd4hep::mm, lpos.y() * edm4eic::unit::mm / dd4hep::mm,
      lpos.z() * edm4eic::unit::mm / dd4hep::mm);

  return HitGeometry{.position  = position,
                     .dimension = dimension,
                     .local     = local_position,
                     .layer     = lid,
                     .sector    = sid};
}

edm4hep::MutableRawCalorimeterHit
CalorimeterCALOROCReco::_createHits(std::uint64_t cellID, double charge, double energy, double time,
                                    const HitGeometry& geometry, const Output& output) const {
  auto [recohits, rawhits] = output;
  auto rawhit              = rawhits->create();
  rawhit.setCellID(cellID);
  if (m_cfg.energyCalibration == CalorimeterCALOROCRecoConfig::EnergyCalibration::ADC) {
    rawhit.setAmplitude(std::llround(charge));
    rawhit.setTimeStamp(std::max(std::llround(time / edm4eic::unit::ns), 0LL));
  } else {
    rawhit.setAmplitude(charge);
    rawhit.setTimeStamp(time);
  }
  auto recohit = recohits->create(cellID, energy, 0, time, 0, geometry.position, geometry.dimension,
                                  geometry.sector, geometry.layer, geometry.local);
  recohit.setRawHit(rawhit);
  return rawhit;
}

void CalorimeterCALOROCReco::_createTruthLinks(
    const edm4hep::RawCalorimeterHit& raw_hit, const edm4eic::SimPulse& pulse,
    const edm4hep::SimCalorimeterHitCollection& source_hits, const ADCOutput& output) {
  auto [recohits, rawhits, links, associations] = output;
  double total_response                         = 0;

  // Loop over source responses to normalize the truth weights.
  for (const auto& hit : pulse.getCalorimeterHits()) {
    total_response += hit.getEnergy();
  }
  if (total_response <= 0) {
    return;
  }

  // Loop over source hits to create response-weighted truth links.
  for (const auto& hit : pulse.getCalorimeterHits()) {
    const double weight = hit.getEnergy() / total_response;
    const auto source   = source_hits.at(hit.getObjectID().index);
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

void CalorimeterCALOROCReco::process(const Input& input, const Output& output) const {
  const auto [npeHits, ADCs] = input;
  std::unordered_map<dd4hep::rec::CellID, size_t> cellID2ADCID;
  for (size_t i = 0; i < ADCs->size(); ++i) {
    cellID2ADCID[ADCs->at(i).getCellID()] = i;
  }

  // Loop over NPE hits and match their digitized samples.
  for (const auto& npeHit : *npeHits) {
    const auto cellID = npeHit.getCellID();
    const auto found  = cellID2ADCID.find(cellID);
    if (found == cellID2ADCID.end()) {
      continue;
    }
    const auto samples = _reconstruct1B(ADCs->at(found->second));
    double charge      = 0;
    switch (m_cfg.proxy_type) {
    case CalorimeterCALOROCRecoConfig::ProxyType::sum:
      charge = samples.adc_sum;
      break;
    default:
      error("Proxy type not implemented.");
    }
    const auto energy = _calibrateEnergyLUT(cellID, charge);
    if (!energy) {
      continue;
    }
    const double time   = _timeWalkCorrection(samples);
    const auto geometry = _getHitGeometry(cellID);
    if (!geometry) {
      continue;
    }
    _createHits(cellID, charge, *energy, time, *geometry, output);
  }
}

void CalorimeterCALOROCReco::process(const ADCInput& input, const ADCOutput& output) const {
  const auto [caloroc_hits, pulses, source_hits]  = input;
  auto [reco_hits, raw_hits, links, associations] = output;

  // Loop over aligned digitized hits and pulses.
  for (std::size_t i = 0; i < caloroc_hits->size(); ++i) {
    const auto hit     = (*caloroc_hits)[i];
    const auto pulse   = (*pulses)[i];
    const auto cellID  = hit.getCellID();
    const auto samples = m_cfg.calorocType == "1A" ? _reconstruct1A(hit) : _reconstruct1B(hit);
    const auto energy  = _calibrateEnergyADC(cellID, samples);
    if (!energy) {
      continue;
    }
    const double time   = _timeWalkCorrection(samples);
    const auto geometry = _getHitGeometry(cellID);
    if (!geometry) {
      continue;
    }
    const auto raw_hit =
        _createHits(cellID, samples.adc_sum, *energy, time, *geometry, {reco_hits, raw_hits});
    _createTruthLinks(raw_hit, pulse, *source_hits, output);
  }
}

} // namespace eicrecon
