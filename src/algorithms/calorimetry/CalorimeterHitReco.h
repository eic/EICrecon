
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Wouter Deconinck, Chao, Whitney Armstrong

// Reconstruct digitized outputs, paired with Jug::Digi::CalorimeterHitDigi
// Author: Chao Peng
// Date: 06/14/2021

#pragma once

#include <DD4hep/DetElement.h>
#include <Parsers/Primitives.h>
#include <stddef.h>
#include <stdint.h>
#include <memory>

#include "CalorimeterHitRecoConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace dd4hep::DDSegmentation { class BitFieldCoder; }
namespace dd4hep::rec { class CellIDPositionConverter; }
namespace dd4hep { class Detector; }
namespace edm4eic { class CalorimeterHitCollection; }
namespace edm4hep { class RawCalorimeterHitCollection; }
namespace spdlog { class logger; }

namespace eicrecon {

  class CalorimeterHitReco : public WithPodConfig<CalorimeterHitRecoConfig> {

  public:
    void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger);
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
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_converter;
    std::shared_ptr<spdlog::logger> m_log;

  };

} // namespace eicrecon
