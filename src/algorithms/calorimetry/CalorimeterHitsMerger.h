// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Jihee Kim, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, David Lawrence

/*
 *  An algorithm to group readout hits from a calorimeter
 *  Energy is summed
 *
 *  Author: Chao Peng (ANL), 03/31/2021
 */

#pragma once

#include <algorithm>
#include <bitset>
#include <memory>
#include <tuple>
#include <unordered_map>

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>

#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/vector_utils.h>
#include <spdlog/spdlog.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "CalorimeterHitsMergerConfig.h"

namespace eicrecon {

  class CalorimeterHitsMerger : public WithPodConfig<CalorimeterHitsMergerConfig>  {

  public:
    void init(const dd4hep::Detector* detector, const dd4hep::rec::CellIDPositionConverter* converter, std::shared_ptr<spdlog::logger>& logger);
    std::unique_ptr<edm4eic::CalorimeterHitCollection> process(const edm4eic::CalorimeterHitCollection &input);

  private:
    uint64_t id_mask{0}, ref_mask{0};

  private:
    const dd4hep::Detector* m_detector;
    const dd4hep::rec::CellIDPositionConverter* m_converter;
    std::shared_ptr<spdlog::logger> m_log;

  };

} // namespace eicrecon
