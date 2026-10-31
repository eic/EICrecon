// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim

#pragma once

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <cstddef>
#include <gsl/pointers>
#include <string>
#include <string_view>

#include "CalorimeterCALOROCRecoCombinerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

// Combines the calibrated single-sided hits from two CalorimeterCALOROCReco (P and N side):
//  - estimates z from the difference of the times of arrival at both ends
//  - corrects both energies for attenuation along the fiber
//  - estimates the energy as sqrt(E_P * E_N)
// The NpeHits are only used for the MC truth and (optionally) the truth z.
using CalorimeterCALOROCRecoCombinerAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::CalorimeterHitCollection, edm4eic::CalorimeterHitCollection,
                      edm4hep::SimCalorimeterHitCollection, edm4hep::SimCalorimeterHitCollection>,
    algorithms::Output<edm4eic::CalorimeterHitCollection, edm4hep::RawCalorimeterHitCollection,
                       edm4eic::MCRecoCalorimeterHitLinkCollection,
                       edm4eic::MCRecoCalorimeterHitAssociationCollection>>;

class CalorimeterCALOROCRecoCombiner : public CalorimeterCALOROCRecoCombinerAlgorithm,
                                       public WithPodConfig<CalorimeterCALOROCRecoCombinerConfig> {

public:
  CalorimeterCALOROCRecoCombiner(std::string_view name)
      : CalorimeterCALOROCRecoCombinerAlgorithm{
            name,
            {"inputRecHitPCollection", "inputRecHitNCollection", "inputNpeHitPCollection",
             "inputNpeHitNCollection"},
            {"outputRecHitCollection", "outputRawHitCollection", "outputRawLink", "outputRawAssoc"},
            "Combine the P and N sided CALOROC hits into one hit, with z and attenuation."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  double m_reference_z_p{0}, m_reference_z_n{0};

  dd4hep::DetElement m_local;
  std::size_t local_mask = ~static_cast<std::size_t>(0);

  double _energyCor(double referencePos, double energy, double z) const;

  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
};

} // namespace eicrecon
