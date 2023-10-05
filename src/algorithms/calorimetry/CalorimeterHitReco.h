
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Wouter Deconinck, Chao, Whitney Armstrong

// Reconstruct digitized outputs, paired with Jug::Digi::CalorimeterHitDigi
// Author: Chao Peng
// Date: 06/14/2021

#pragma once

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>

#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/vector_utils.h>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "CalorimeterHitRecoConfig.h"

namespace eicrecon {

  class CalorimeterHitReco : public WithPodConfig<CalorimeterHitRecoConfig> {

  public:
    void init(const dd4hep::Detector* detector, const dd4hep::rec::CellIDPositionConverter* converter, std::shared_ptr<spdlog::logger>& logger);
    std::unique_ptr<edm4eic::CalorimeterHitCollection> process(const edm4hep::RawCalorimeterHitCollection &rawhits);

  private:

    // unitless counterparts of the input parameters
    double thresholdADC{0};
    double stepTDC{0};

    dd4hep::BitFieldCoder* id_dec = nullptr;
    uint32_t NcellIDerrors = 0;
    uint32_t MaxCellIDerrors = 100;

    size_t sector_idx{0}, layer_idx{0};

    bool warned_unsupported_segmentation = false;

    dd4hep::DetElement local;
    size_t local_mask = ~static_cast<size_t>(0), gpos_mask = static_cast<size_t>(0);

  private:
    const dd4hep::Detector* m_detector;
    const dd4hep::rec::CellIDPositionConverter* m_converter;
    std::shared_ptr<spdlog::logger> m_log;

  };

} // namespace eicrecon
