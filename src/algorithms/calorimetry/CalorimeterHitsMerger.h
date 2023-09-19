// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Jihee Kim, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, David Lawrence

/*
 *  An algorithm to group readout hits from a calorimeter
 *  Energy is summed
 *
 *  Author: Chao Peng (ANL), 03/31/2021
 */

#pragma once

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <spdlog/logger.h>
#include <stdint.h>
#include <memory>

#include "CalorimeterHitsMergerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  class CalorimeterHitsMerger : public WithPodConfig<CalorimeterHitsMergerConfig>  {

  public:
    void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger);
    std::unique_ptr<edm4eic::CalorimeterHitCollection> process(const edm4eic::CalorimeterHitCollection &input);

  private:
    uint64_t id_mask{0}, ref_mask{0};

  private:
    const dd4hep::Detector* m_detector;
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_converter;
    std::shared_ptr<spdlog::logger> m_log;

  };

} // namespace eicrecon
