
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Wouter Deconinck, Chao, Whitney Armstrong

// Reconstruct digitized outputs, paired with Jug::Digi::CalorimeterHitDigi
// Author: Chao Peng
// Date: 06/14/2021

#pragma once

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DDRec/CellIDPositionConverter.h>
#include <Parsers/Primitives.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/SimPulseCollection.h>
#include <stdint.h>
#include <cstddef>
#include <functional>
#include <gsl/pointers>
#include <string>
#include <string_view>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <random>

#include "CalorimeterCALOROCCalibrationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using PulseType        = edm4eic::SimPulse;

using CalorimeterCALOROCCalibrationAlgorithm =
algorithms::Algorithm<algorithms::Input<PulseType::collection_type,
                                        edm4eic::RawCALOROCHitCollection,
                                        PulseType::collection_type,
                                        edm4eic::RawCALOROCHitCollection>,
                          algorithms::Output<edm4eic::CalorimeterHitCollection, 
                                             edm4hep::RawCalorimeterHitCollection, 
                                             edm4eic::MCRecoCalorimeterHitLinkCollection, 
                                             edm4eic::MCRecoCalorimeterHitAssociationCollection>>;

class CalorimeterCALOROCCalibration : public CalorimeterCALOROCCalibrationAlgorithm,
                           public WithPodConfig<CalorimeterCALOROCCalibrationConfig> {

public:
  CalorimeterCALOROCCalibration(std::string_view name)
      : CalorimeterCALOROCCalibrationAlgorithm{name,
                                    {"inputADCPCollection", "inputPulsePCollection",
                                     "inputADCNCollection", "inputPulseNCollection"},
                                    {"outputRecHitCollection", "outputRawHitCollection", "outputRawLink", "outputRawAssoc"},
                                    "Reconstruct hit from half-way-reconstructed pulse."}{}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  // unitless counterparts of the input parameters
  double thresholdADC{0};
  double stepTDC{0};

  std::function<double(const edm4hep::RawCalorimeterHit& h)> sampFrac;

  dd4hep::IDDescriptor id_spec;
  dd4hep::BitFieldCoder* id_dec = nullptr;

  double m_reference_z_p, m_reference_z_n, m_slope, m_intercept;

  mutable uint32_t NcellIDerrors = 0;
  uint32_t MaxCellIDerrors       = 100;

  std::size_t sector_idx{0}, layer_idx{0};

  mutable bool warned_unsupported_segmentation = false;

  dd4hep::DetElement m_local;
  std::size_t local_mask = ~static_cast<std::size_t>(0), gpos_mask = static_cast<std::size_t>(0);

  std::map<std::vector<int>, double> m_edep_to_npe_lut{};
  std::vector<std::size_t> m_field_idxs{};

  double _sumADC(const edm4eic::RawCALOROCHit& ADC) const;
  double _simpson(const edm4eic::RawCALOROCHit& ADC) const;

  double _energyCor(double referencePos, double energy, double z) const;
  double _toa(const edm4eic::RawCALOROCHit& ADC) const;
  double _timeWalkCorrection(double toa, double ADC) const;

  const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();
private:
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  const dd4hep::rec::CellIDPositionConverter* m_converter{
      algorithms::GeoSvc::instance().cellIDPositionConverter()};
};

} // namespace eicrecon
