// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
// Original header from Gaudi algorithm
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck
// TODO refactor header when license is clear

#ifndef EICRECON_TRACKER_SOURCE_LINKER_H
#define EICRECON_TRACKER_SOURCE_LINKER_H

#include "TrackerSourceLinkerResult.h"

#include <vector>
#include <edm4eic/TrackerHit.h>
#include <spdlog/logger.h>
#include <list>
#include <DDRec/CellIDPositionConverter.h>

#include "JugTrack/IndexSourceLink.hpp"
#include "JugTrack/Measurement.hpp"

#include "GeoSvc.h"

namespace eicrecon {

    class TrackerSourceLinker {
    public:
        void init(std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> cellid_converter,
                  std::shared_ptr<const GeoSvc> acts_context,
                  std::shared_ptr<spdlog::logger> logger);

        eicrecon::TrackerSourceLinkerResult *produce(std::vector<const edm4eic::TrackerHit *> trk_hits);

    private:
        std::shared_ptr<spdlog::logger> m_log;

        /// Cell ID position converter
        std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter;

        std::shared_ptr<const GeoSvc> m_acts_context;

    };

}
#endif //EICRECON_TRACKER_SOURCE_LINKER_H
