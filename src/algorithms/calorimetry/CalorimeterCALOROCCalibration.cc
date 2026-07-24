// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim

#include "CalorimeterCALOROCCalibration.h"

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
#include <edm4eic/unit_system.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <algorithms/service.h>
#include <edm4hep/Vector3f.h>
#include <fmt/ranges.h>
#include <algorithm>
#include <cctype>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include "TVector3.h"

#include "algorithms/calorimetry/CalorimeterCALOROCCalibrationConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

using namespace dd4hep;

namespace eicrecon {

double CalorimeterCALOROCCalibration::_energyCor(double referencePos, double energy,
                                                 double z) const {
  double length = std::abs(referencePos - z);
  double factor =
      m_cfg.attenuationParameters[0] * std::exp(-length / m_cfg.attenuationParameters[1]) +
      (1 - m_cfg.attenuationParameters[0]) * std::exp(-length / m_cfg.attenuationParameters[2]);
  return energy / factor;
}

void CalorimeterCALOROCCalibration::init() {

  if (m_cfg.attenuationReferencePositionNamePos.empty() ||
      m_cfg.attenuationReferencePositionNameNeg.empty()) {
    error("You MUST provide the name of reference positions for attenuation correction");
  }

  m_reference_z_p = m_geo.detector()->constant<double>(m_cfg.attenuationReferencePositionNamePos) *
                    edm4eic::unit::mm / dd4hep::mm;
  m_reference_z_n = m_geo.detector()->constant<double>(m_cfg.attenuationReferencePositionNameNeg) *
                    edm4eic::unit::mm / dd4hep::mm;

  info("Pos reference z = {}", m_reference_z_p);
  info("Neg reference z = {}", m_reference_z_n);

  m_slope     = m_cfg.slope;
  m_intercept = m_cfg.intercept;

  info("calibration slope = {}, intercept = {}", m_slope, m_intercept);

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

  // copy and pasted from Minho's custom PulseGenerator class
  // Get the field indices and field-dependent conversion factors if necessary
  if (m_cfg.readout.empty() || m_cfg.edep_to_npe_fields.empty() ||
      m_cfg.edep_to_npe_filename.empty())
    error("You MUST provide edep_to_npe files and filename for layer-by-layer calibration.");

  for (const auto& field : m_cfg.edep_to_npe_fields) {
    auto field_idx = id_dec->index(field);
    m_field_idxs.push_back(field_idx);
  }

  std::string filename = fmt::format("calibrations/{}", m_cfg.edep_to_npe_filename);
  std::ifstream infile(filename);
  if (!infile) {
    error("Unable to open LUT file: {}", filename);
  }
  info("LUT file: {}", filename);
  std::string line;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    std::vector<int> keys;
    for (std::size_t i = 0; i < m_cfg.edep_to_npe_fields.size(); i++) {
      int value;
      iss >> value;
      keys.push_back(value);
    }
    double factor;
    if (!(iss >> factor))
      error("Malformed LUT file: {}", filename);
    m_edep_to_npe_lut[keys] = factor;
  }
}

double CalorimeterCALOROCCalibration::_sumADC(const edm4eic::RawCALOROCHit& ADC) const {
  double sum = 0;

  // check high gain ADC first. If it saturates, we switch to lowGainADC
  for (const auto& bSample : ADC.getBSamples()) {
    if (bSample.highGainADC >= m_cfg.highGainDR)
      sum += static_cast<double>(bSample.lowGainADC);
    else
      sum += static_cast<double>(bSample.highGainADC) / m_cfg.gainRatio;
  }
  return sum;
}

double CalorimeterCALOROCCalibration::_toa(const edm4eic::RawCALOROCHit& ADC) const {
  // find the relative toa
  int samplePhase        = ADC.getSamplePhase();
  int timeStamp          = ADC.getTimeStamp();
  const auto& bSamples   = ADC.getBSamples();
  size_t idx_toa         = 0;
  uint16_t timeOfArrival = 0;
  for (; idx_toa < bSamples.size(); ++idx_toa) {
    timeOfArrival = bSamples.at(idx_toa).timeOfArrival;
    if (timeOfArrival > 0)
      break;
  }

  return samplePhase * (25. / 1042.) + (timeStamp + idx_toa) * 25. - timeOfArrival * (25. / 1024.);
}

double CalorimeterCALOROCCalibration::_timeWalkCorrection(double toa, double lowGainADC) const {
  if (static_cast<double>(lowGainADC) - m_cfg.timeWalkCorrectionParameters[2] > 0)
    return toa - (m_cfg.timeWalkCorrectionParameters[1] *
                      pow(static_cast<double>(lowGainADC) - m_cfg.timeWalkCorrectionParameters[2],
                          m_cfg.timeWalkCorrectionParameters[3]) +
                  m_cfg.timeWalkCorrectionParameters[0]);
  else
    return toa;
}

void CalorimeterCALOROCCalibration::process(
    const CalorimeterCALOROCCalibration::Input& input,
    const CalorimeterCALOROCCalibration::Output& output) const {

  const auto [pulsesP, ADCPs, pulsesN, ADCNs]         = input;
  auto [recohits, rawhits, rawhitsLink, rawhitsAssoc] = output;

  // match pulses pair with the same cellID;
  std::unordered_map<dd4hep::rec::CellID, size_t> cellID2PulsesNID, cellID2PulsesPID, cellID2ADCNID,
      cellID2ADCPID;
  for (size_t i = 0; i < pulsesN->size(); ++i) {
    const auto& pulse                   = pulsesN->at(i);
    cellID2PulsesNID[pulse.getCellID()] = i;
  }

  for (size_t i = 0; i < pulsesP->size(); ++i) {
    const auto& pulse                   = pulsesP->at(i);
    cellID2PulsesPID[pulse.getCellID()] = i;
  }

  for (size_t i = 0; i < ADCPs->size(); ++i) {
    const auto& ADCP                = ADCPs->at(i);
    cellID2ADCPID[ADCP.getCellID()] = i;
  }

  for (size_t i = 0; i < ADCNs->size(); ++i) {
    const auto& ADCN                = ADCNs->at(i);
    cellID2ADCNID[ADCN.getCellID()] = i;
  }

  for (const auto& pulseP : *pulsesP) {
    auto cellID = pulseP.getCellID();

    // ignore data point if the hit is not showing up on all ADCs and Pulses classes
    auto it = cellID2PulsesNID.find(cellID);
    if (it == cellID2PulsesNID.end())
      continue;
    const auto& pulseN = pulsesN->at(it->second);
    it                 = cellID2ADCPID.find(cellID);
    if (it == cellID2ADCPID.end())
      continue;
    const auto& ADCP = ADCPs->at(it->second);
    it               = cellID2ADCNID.find(cellID);
    if (it == cellID2ADCNID.end())
      continue;
    const auto& ADCN = ADCNs->at(it->second);

    // get layer and sector ID
    const int lid = id_dec != nullptr && !m_cfg.layerField.empty()
                        ? static_cast<int>(id_dec->get(cellID, layer_idx))
                        : -1;
    const int sid = id_dec != nullptr && !m_cfg.sectorField.empty()
                        ? static_cast<int>(id_dec->get(cellID, sector_idx))
                        : -1;

    auto tP     = this->_toa(ADCP);
    auto tN     = this->_toa(ADCN);
    double time = 0.5 * (tP + tN);

    // get position of the hit;
    double zpos;

    if (m_cfg.usePulsePos) {
      const auto& pos = pulseP.getPosition();
      zpos            = static_cast<double>(pos.z);
    } else {
      if (m_cfg.timeWalkCor) {
        tP = this->_timeWalkCorrection(tP, this->_sumADC(ADCP));
        tN = this->_timeWalkCorrection(tN, this->_sumADC(ADCN));
      }
      auto dt = tN - tP;
      zpos    = dt * m_cfg.lightSpeedParameters[0] + m_cfg.lightSpeedParameters[1];
    }
    // bound zpos by sipm location
    zpos = std::max(m_reference_z_n, std::min(m_reference_z_p, zpos));

    // fetch edep to npe factor on this layer
    std::vector<int> key;
    for (const auto idx : m_field_idxs)
      key.push_back(static_cast<int>(id_dec->get(cellID, idx)));
    double eDep2NpeFactor = m_edep_to_npe_lut.find(key)->second;

    // find averaged energy
    double npeP, npeN;
    if (m_cfg.usePulseNPE) {
      npeP = pulseP.getIntegral();
      npeN = pulseN.getIntegral();
    } else {
      // get values from both sides in a loop
      for (const bool& NSide : std::vector<bool>{true, false}) {
        auto& ADC = NSide ? ADCN : ADCP;
        auto& npe = NSide ? npeN : npeP;

        switch (m_cfg.proxy_type) {
        case CalorimeterCALOROCCalibrationConfig::ProxyType::sum:
          npe = this->_sumADC(ADC);
          break;
        case CalorimeterCALOROCCalibrationConfig::ProxyType::templateFit:
          error("Proxy type not implemented.");
        case CalorimeterCALOROCCalibrationConfig::ProxyType::simpson:
          error("Proxy type not implemented.");
        default:
          error("Proxy type not implemented.");
        }
      }
    }

    // perform layer-by-layer energy correction
    double chargeP = npeP / eDep2NpeFactor;
    double chargeN = npeN / eDep2NpeFactor;

    // attenuation correction
    double corEP = this->_energyCor(m_reference_z_p, chargeP, zpos) * m_slope + m_intercept;
    double corEN = this->_energyCor(m_reference_z_n, chargeN, zpos) * m_slope + m_intercept;
    double corE  = std::sqrt(corEP * corEN);

    // raw hits for all pulses
    edm4hep::MutableRawCalorimeterHit rawhit;
    rawhit.setCellID(cellID);
    rawhit.setAmplitude(npeP);
    rawhit.setTimeStamp(time);

    double edep = 0;

    // link to parents
    std::unordered_map<podio::ObjectID, edm4eic::MutableMCRecoCalorimeterHitLink> links_staging;
    std::unordered_map<podio::ObjectID, edm4eic::MutableMCRecoCalorimeterHitAssociation>
        rawassocs_staging;

    for (bool NSide : std::vector<bool>{true, false}) {
      const auto& hits = NSide ? pulseN.getCalorimeterHits() : pulseP.getCalorimeterHits();
      for (const auto& hit : hits) {
        // if link is already covered, don't add again
        if (links_staging.find(hit.getObjectID()) != links_staging.end())
          continue;

        edep += hit.getEnergy();
        edm4eic::MutableMCRecoCalorimeterHitAssociation assoc;
        assoc.setRawHit(rawhit);

        edm4eic::MutableMCRecoCalorimeterHitLink link;
        link.setFrom(rawhit);

        assoc.setSimHit(hit);
        assoc.setWeight(hit.getEnergy());

        link.setTo(hit);
        link.setWeight(hit.getEnergy());

        rawassocs_staging[hit.getObjectID()] = assoc;
        links_staging[hit.getObjectID()]     = link;
      }
    }

    for (auto& [key, link] : links_staging) {
      link.setWeight(link.getWeight() / edep);
      rawhitsLink->push_back(link);
    }
    for (auto& [key, assoc] : rawassocs_staging) {
      assoc.setWeight(assoc.getWeight() / edep);
      rawhitsAssoc->push_back(assoc);
    }

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
      gpos.SetZ(zpos * dd4hep::mm);

      // local positions
      if (m_cfg.localDetElement.empty()) {
        auto volman = m_detector->volumeManager();
        local       = volman.lookupDetElement(cellID & local_mask);
      } else {
        local = m_local;
      }
    } catch (...) {
      continue;
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
        lpos.x() / dd4hep::mm, lpos.y() / dd4hep::mm, lpos.z() / dd4hep::mm);

    auto recohit =
        recohits->create(cellID, corE, 0, time, 0, position, dimension, sid, lid, local_position);
    recohit.setRawHit(rawhit);
    rawhits->push_back(rawhit);
  }
}

} // namespace eicrecon
