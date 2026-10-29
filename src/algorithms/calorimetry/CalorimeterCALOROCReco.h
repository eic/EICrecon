// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim

#pragma once

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DDRec/CellIDPositionConverter.h>
#include <Parsers/Primitives.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <cstddef>
#include <gsl/pointers>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "CalorimeterCALOROCRecoConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

// Reconstructs the hits seen by ONE end (P or N) of the barrel fibers.
// The output energy and time are calibrated, but z is not estimated here: the position is the
// cell position from the geometry. See CalorimeterCALOROCRecoCombiner for combining both ends.
using CalorimeterCALOROCRecoAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::SimCalorimeterHitCollection, edm4eic::RawCALOROCHitCollection>,
    algorithms::Output<edm4eic::CalorimeterHitCollection, edm4hep::RawCalorimeterHitCollection>>;

class CalorimeterCALOROCReco : public CalorimeterCALOROCRecoAlgorithm,
                               public WithPodConfig<CalorimeterCALOROCRecoConfig> {

public:
  CalorimeterCALOROCReco(std::string_view name)
      : CalorimeterCALOROCRecoAlgorithm{
            name,
            {"inputNpeHitCollection", "inputADCCollection"},
            {"outputRecHitCollection", "outputRawHitCollection"},
            "Reconstruct single-sided hits from CALOROC ADC data (energy and time calibration)."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  dd4hep::IDDescriptor id_spec;
  dd4hep::BitFieldCoder* id_dec = nullptr;

  std::size_t sector_idx{0}, layer_idx{0};

  mutable bool warned_unsupported_segmentation = false;

  dd4hep::DetElement m_local;
  std::size_t local_mask = ~static_cast<std::size_t>(0), gpos_mask = static_cast<std::size_t>(0);

  std::map<std::vector<int>, double> m_edep_to_npe_lut{};
  std::vector<std::size_t> m_field_idxs{};

  double _sumADC(const edm4eic::RawCALOROCHit& ADC) const;
  double _toa(const edm4eic::RawCALOROCHit& ADC) const;
  double _timeWalkCorrection(double toa, double ADC) const;

private:
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  const dd4hep::rec::CellIDPositionConverter* m_converter{
      algorithms::GeoSvc::instance().cellIDPositionConverter()};
};

} // namespace eicrecon
