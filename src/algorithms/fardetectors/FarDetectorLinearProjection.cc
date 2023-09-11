// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <edm4eic/vector_utils.h>

#include "FarDetectorLinearProjection.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <iterator>
#include <algorithm>
#include <map>



namespace eicrecon {


    void FarDetectorLinearProjection::init(std::shared_ptr<spdlog::logger>& logger) {

      m_log      = logger;

//       // plane position
//       m_plane_position = Eigen::Vector3d(3,1,m_cfg.plane_position);
//       // plane tangent
//       m_plane_tangent  = Eigen::Vector3d(3,1,m_cfg.plane_tangent);

    }

    std::unique_ptr<edm4eic::TrackParametersCollection> FarDetectorLinearProjection::produce(const edm4eic::TrackSegmentCollection &inputhits) {

        auto outputTracks = std::make_unique<edm4eic::TrackParametersCollection>();

        return outputTracks;

    }


}
