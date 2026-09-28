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
#include <cstddef>
#include <string_view>

#include "algorithms/calorimetry/CALOROCToRecoCalorimeterHitConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CALOROCToRecoCalorimeterHitAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::RawCALOROCHitCollection, edm4eic::SimPulseCollection>,
    algorithms::Output<edm4eic::CalorimeterHitCollection,
                       edm4hep::RawCalorimeterHitCollection,
                       edm4eic::MCRecoCalorimeterHitLinkCollection,
                       edm4eic::MCRecoCalorimeterHitAssociationCollection>>;

class CALOROCToRecoCalorimeterHit : public CALOROCToRecoCalorimeterHitAlgorithm,
                                   public WithPodConfig<CALOROCToRecoCalorimeterHitConfig> {

public:
  CALOROCToRecoCalorimeterHit(std::string_view name)
      : CALOROCToRecoCalorimeterHitAlgorithm{
            name,
            {"inputCALOROCHits", "inputPulses"},
            {"outputRecoHits", "outputRawHits", "outputHitLinks", "outputRawHitAssociations"},
            {"Reconstructs calorimeter hits from CALOROC samples"}} {}
  void init() final;
  void process(const Input&, const Output&) const final;

private:
  dd4hep::IDDescriptor m_id_spec;
  dd4hep::BitFieldCoder* m_id_decoder{nullptr};
  std::size_t m_layer_index{0};
  mutable bool warned_unsupported_segmentation = false;
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  const dd4hep::rec::CellIDPositionConverter* m_converter{
      algorithms::GeoSvc::instance().cellIDPositionConverter()};
};

} // namespace eicrecon
