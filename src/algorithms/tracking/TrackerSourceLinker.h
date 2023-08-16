// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
// Original header from Gaudi algorithm
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck
// TODO refactor header when license is clear

#pragma once

#include "TrackerSourceLinkerResult.h"

#include <vector>
#include <edm4eic/TrackerHit.h>
#include <spdlog/logger.h>
#include <list>
#include <DDRec/CellIDPositionConverter.h>
#include <DD4hep/Detector.h>

#include "ActsExamples/EventData/IndexSourceLink.hpp"
#include "ActsExamples/EventData/Measurement.hpp"

#include "ActsGeometryProvider.h"

namespace eicrecon {

    class TrackerSourceLinker {
    public:
        void init(std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> cellid_converter,
                  std::shared_ptr<const ActsGeometryProvider> acts_context,
                  std::shared_ptr<spdlog::logger> logger);

        eicrecon::TrackerSourceLinkerResult *produce(std::vector<const edm4eic::TrackerHit *> trk_hits);

    private:
        std::shared_ptr<spdlog::logger> m_log;

        /// Cell ID position converter
        std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter;

        std::shared_ptr<const ActsGeometryProvider> m_acts_context;

	dd4hep::Detector* m_dd4hepGeo;

	/// Detector-specific information
	int m_detid_b0tracker;
    };

}
