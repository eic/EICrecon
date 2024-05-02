// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

#include <algorithms/algorithm.h>
#include <DDRec/CellIDPositionConverter.h>
#include <Eigen/Dense>
// Event Model related classes
#include <edm4eic/TrackSegmentCollection.h>

#include <spdlog/logger.h>
#include "FarDetectorLinearTrackingConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

    using FarDetectorLinearTrackingAlgorithm = algorithms::Algorithm<
        algorithms::Input<
            std::vector<edm4hep::TrackerHitCollection>
        >,
        algorithms::Output<
            edm4eic::TrackSegmentCollection
        >
    >;

    class FarDetectorLinearTracking
    : public FarDetectorLinearTrackingAlgorithm,
      public WithPodConfig<FarDetectorLinearTrackingConfig>  {

    public:
        FarDetectorLinearTracking(std::string_view name)
            : FarDetectorLinearTrackingAlgorithm{name,
                {"inputHitCollections"},
                {"outputTrackSegments"},
                "Fit track segments from hits in the tracker layers"} {}

        /** One time initialization **/
        void init(std::shared_ptr<spdlog::logger>& logger);

        /** Event by event processing **/
        void process(const Input&, const Output&) const final;

    private:
        std::shared_ptr<spdlog::logger> m_log;

        Eigen::VectorXd m_layerWeights;

        Eigen::Vector3d m_optimumDirection;

        void makeHitCombination(int level,
                                Eigen::MatrixXd* hitMatrix,
                                const std::vector<gsl::not_null<const edm4hep::TrackerHitCollection*>>& hits,
                                gsl::not_null<edm4eic::TrackSegmentCollection*> outputTracks) const;

        void checkHitCombination(Eigen::MatrixXd* hitMatrix,
                                gsl::not_null<edm4eic::TrackSegmentCollection*> outputTracks) const;

        bool checkHitPair(const Eigen::Vector3d& hit1,
                          const Eigen::Vector3d& hit2) const;

    };

} // eicrecon
