// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <memory>

#include "TrackerHitReconstructionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace dd4hep::rec { class CellIDPositionConverter; }
namespace dd4hep { class Detector; }
namespace edm4eic { class RawTrackerHitCollection; }
namespace edm4eic { class TrackerHitCollection; }
namespace spdlog { class logger; }

namespace eicrecon {

    /**
     * Produces edm4eic::TrackerHit with geometric info from edm4eic::RawTrackerHit
     */
    class TrackerHitReconstruction : public WithPodConfig<TrackerHitReconstructionConfig> {

    public:
        /// Once in a lifetime initialization
        void init(const dd4hep::Detector *detector, std::shared_ptr<spdlog::logger>& logger);

        /// Processes RawTrackerHit and produces a TrackerHit
        std::unique_ptr<edm4eic::TrackerHitCollection> process(const edm4eic::RawTrackerHitCollection& raw_hits);

        /// Set a configuration
        eicrecon::TrackerHitReconstructionConfig& applyConfig(eicrecon::TrackerHitReconstructionConfig& cfg) {m_cfg = cfg; return m_cfg;}

    private:
        /** algorithm logger */
        std::shared_ptr<spdlog::logger> m_log;

        /// Cell ID position converter
        std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter;
    };
}
