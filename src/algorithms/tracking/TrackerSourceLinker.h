// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
// Original header from Gaudi algorithm
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck
// TODO refactor header when license is clear

#pragma once

#include <gsl/gsl>
#include <spdlog/logger.h>

#include <DDRec/CellIDPositionConverter.h>
#include <DD4hep/Detector.h>

#include <edm4eic/TrackerHit.h>
#include "algorithms/tracking/TrackerSourceLinkerResult.h"

#include "ActsGeometryProvider.h"

namespace eicrecon {

    class TrackerSourceLinker {
    public:
        void init(const dd4hep::Detector* detector,
                  const dd4hep::rec::CellIDPositionConverter* converter,
                  std::shared_ptr<const ActsGeometryProvider> acts_context,
                  std::shared_ptr<spdlog::logger> logger);

        eicrecon::TrackerSourceLinkerResult *produce(std::vector<const edm4eic::TrackerHit *> trk_hits);

    private:
        std::shared_ptr<spdlog::logger> m_log;

        /// Geometry and Cell ID position converter
        const dd4hep::Detector* m_dd4hepGeo;
        const dd4hep::rec::CellIDPositionConverter* m_converter;

        std::shared_ptr<const ActsGeometryProvider> m_acts_context;

        /// Detector-specific information
        int m_detid_b0tracker;
    };

}
