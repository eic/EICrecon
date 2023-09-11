// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

#include <DDRec/CellIDPositionConverter.h>
#include <Eigen/Dense>
// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSegmentCollection.h>

#include <spdlog/logger.h>
#include "FarDetectorLinearProjectionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

    class FarDetectorLinearProjection : public WithPodConfig<FarDetectorLinearProjectionConfig>  {

    public:

        /** One time initialization **/
        void init(std::shared_ptr<spdlog::logger>& logger);

        /** Event by event processing **/
        std::unique_ptr<edm4eic::TrackParametersCollection> produce(const edm4eic::TrackSegmentCollection &inputhits);


    private:
        std::shared_ptr<spdlog::logger> m_log;
	Eigen::Vector3d  m_plane_position;
	Eigen::Vector3d  m_plane_a;
	Eigen::Vector3d  m_plane_b;
	Eigen::Matrix3d  m_directions;

    };

} // eicrecon
