
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
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <stdint.h>
#include <cstddef>
#include <functional>
#include <gsl/pointers>
#include <string>
#include <string_view>

#include "CalorimeterHitRecoConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterHitRecoAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::RawCalorimeterHitCollection>,
                          algorithms::Output<edm4eic::CalorimeterHitCollection>>;

class CalorimeterHitReco : public CalorimeterHitRecoAlgorithm,
                           public WithPodConfig<CalorimeterHitRecoConfig> {

public:
  CalorimeterHitReco(std::string_view name)
      : CalorimeterHitRecoAlgorithm{name,
                                    {"inputRawHitCollection"},
                                    {"outputRecHitCollection"},
                                    "Reconstruct hit from digitized input."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  // unitless counterparts of the input parameters
  double thresholdADC{0};
  double stepTDC{0};

  std::function<double(const edm4hep::RawCalorimeterHit& h)> sampFrac;

  dd4hep::IDDescriptor id_spec;
  dd4hep::BitFieldCoder* id_dec = nullptr;

  mutable uint32_t NcellIDerrors = 0;
  uint32_t MaxCellIDerrors       = 100;

  std::size_t sector_idx{0}, layer_idx{0};

  mutable bool warned_unsupported_segmentation = false;

  dd4hep::DetElement m_local;
  std::size_t local_mask = ~static_cast<std::size_t>(0), gpos_mask = static_cast<std::size_t>(0);

private:
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  const dd4hep::rec::CellIDPositionConverter* m_converter{
      algorithms::GeoSvc::instance().cellIDPositionConverter()};
};

} // namespace eicrecon
