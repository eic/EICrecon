// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
// Original header from Gaudi algorithm
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Shujie Li
// TODO refactor header when license is clear

#pragma once

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <spdlog/logger.h>
#include <memory>

#include "ActsGeometryProvider.h"

namespace eicrecon {

    class TrackerMeasurementFromHits {
    public:
        void init(const dd4hep::Detector* detector,
                  const dd4hep::rec::CellIDPositionConverter* converter,
                  std::shared_ptr<const ActsGeometryProvider> acts_context,
                  std::shared_ptr<spdlog::logger> logger);

        std::unique_ptr<edm4eic::Measurement2DCollection> produce(const edm4eic::TrackerHitCollection& trk_hits);

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
