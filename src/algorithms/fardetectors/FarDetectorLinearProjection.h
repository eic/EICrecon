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
#include "algorithms/algorithm.h"

namespace eicrecon {

    using FarDetectorLinearProjectionAlgorithm = algorithms::Algorithm<
        algorithms::Input<
            edm4eic::TrackSegmentCollection
        >,
        algorithms::Output<
            edm4eic::TrackParametersCollection
        >
    >;

    class FarDetectorLinearProjection
    : public FarDetectorLinearProjectionAlgorithm,
      public WithPodConfig<FarDetectorLinearProjectionConfig>  {

    public:
        FarDetectorLinearProjection(std::string_view name)
            : FarDetectorLinearProjectionAlgorithm{name,
                {"inputTrackSegments"},
                {"outputTrackParameters"},
                "Project track segments to a plane"} {}

        /** One time initialization **/
        void init(std::shared_ptr<spdlog::logger>& logger);

        /** Event by event processing **/
        void process(const Input&, const Output&) const final;


    private:
        std::shared_ptr<spdlog::logger> m_log;
        Eigen::Vector3d  m_plane_position;
        Eigen::Vector3d  m_plane_a;
        Eigen::Vector3d  m_plane_b;
        Eigen::Matrix3d  m_directions;

    };

} // eicrecon
