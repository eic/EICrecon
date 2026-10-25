// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DDRec/CellIDPositionConverter.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4eic/SimPulseCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>

#include "algorithms/calorimetry/CalorimeterCALOROCSingleEndedRecoConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterCALOROCSingleEndedRecoAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::RawCALOROCHitCollection, edm4eic::SimPulseCollection,
                      edm4hep::SimCalorimeterHitCollection>,
    algorithms::Output<edm4eic::CalorimeterHitCollection,
                       edm4hep::RawCalorimeterHitCollection,
                       edm4eic::MCRecoCalorimeterHitLinkCollection,
                       edm4eic::MCRecoCalorimeterHitAssociationCollection>>;

class CalorimeterCALOROCSingleEndedReco : public CalorimeterCALOROCSingleEndedRecoAlgorithm,
                                   public WithPodConfig<CalorimeterCALOROCSingleEndedRecoConfig> {

public:
  CalorimeterCALOROCSingleEndedReco(std::string_view name)
      : CalorimeterCALOROCSingleEndedRecoAlgorithm{
            name,
            {"inputCALOROCHits", "inputPulses", "inputSourceHits"},
            {"outputRecoHits", "outputRawHits", "outputHitLinks", "outputRawHitAssociations"},
            {"Reconstructs calorimeter hits from CALOROC samples"}} {}
  void init() final;
  // Source subset order matches converted SiPM hit indices.
  void process(const Input&, const Output&) const final;

private:
  std::function<double(std::uint64_t)> sampFrac;
  dd4hep::IDDescriptor m_id_spec;
  dd4hep::BitFieldCoder* m_id_decoder{nullptr};
  std::size_t m_layer_index{0};
  mutable bool warned_unsupported_segmentation = false;
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  const dd4hep::rec::CellIDPositionConverter* m_converter{
      algorithms::GeoSvc::instance().cellIDPositionConverter()};
};

} // namespace eicrecon
