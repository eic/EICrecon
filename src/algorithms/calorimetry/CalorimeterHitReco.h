
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Wouter Deconinck, Chao, Whitney Armstrong

// Reconstruct digitized outputs, paired with Jug::Digi::CalorimeterHitDigi
// Author: Chao Peng
// Date: 06/14/2021

#pragma once

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <Parsers/Primitives.h>
#include <algorithms/algorithm.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <spdlog/logger.h>
#include <stddef.h>
#include <stdint.h>
#include <memory>

#include "CalorimeterHitRecoConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  using CalorimeterHitRecoAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4hep::RawCalorimeterHitCollection
    >,
    algorithms::Output<
      edm4eic::CalorimeterHitCollection
    >
  >;

  class CalorimeterHitReco
    : public CalorimeterHitRecoAlgorithm,
      public WithPodConfig<CalorimeterHitRecoConfig> {

  public:
    CalorimeterHitReco(std::string_view name)
      : CalorimeterHitRecoAlgorithm{name,
                            {"inputRawHitCollection"},
                            {"outputRecHitCollection"},
                            "Reconstruct hit from digitized input."} {}

    void init(const dd4hep::Detector* detector, const dd4hep::rec::CellIDPositionConverter* converter, std::shared_ptr<spdlog::logger>& logger);
    void process(const Input&, const Output&) const final;

  private:

    // unitless counterparts of the input parameters
    double thresholdADC{0};
    double stepTDC{0};

    dd4hep::BitFieldCoder* id_dec = nullptr;

    mutable uint32_t NcellIDerrors = 0;
    uint32_t MaxCellIDerrors = 100;

    size_t sector_idx{0}, layer_idx{0};

    mutable bool warned_unsupported_segmentation = false;

    dd4hep::DetElement m_local;
    size_t local_mask = ~static_cast<size_t>(0), gpos_mask = static_cast<size_t>(0);

  private:
    const dd4hep::Detector* m_detector;
    const dd4hep::rec::CellIDPositionConverter* m_converter;
    std::shared_ptr<spdlog::logger> m_log;

  };

} // namespace eicrecon
