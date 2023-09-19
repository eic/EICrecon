// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck

#pragma once

#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/TrackFinding/CombinatorialKalmanFilter.hpp>
#include <Acts/TrackFinding/MeasurementSelector.hpp>
#include <Acts/Utilities/CalibrationContext.hpp>
#include <Acts/Utilities/Result.hpp>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <tuple>
#include <vector>

#include "ActsExamples/EventData/IndexSourceLink.hpp"
#include "ActsExamples/EventData/Measurement.hpp"
#include "ActsExamples/EventData/Track.hpp"
#include "ActsExamples/EventData/Trajectories.hpp"
#include "CKFTrackingConfig.h"
#include "DD4hepBField.h"
#include "algorithms/interfaces/WithPodConfig.h"

class ActsGeometryProvider;

namespace eicrecon {

/** Fitting algorithm implementation .
 *
 * \ingroup tracking
 */

    class CKFTracking: public WithPodConfig<eicrecon::CKFTrackingConfig> {
    public:
        /// Track finder function that takes input measurements, initial trackstate
        /// and track finder options and returns some track-finder-specific result.
        using TrackFinderOptions =
            Acts::CombinatorialKalmanFilterOptions<ActsExamples::IndexSourceLinkAccessor::Iterator,
                                                   Acts::VectorMultiTrajectory>;
        using TrackFinderResult = std::vector<Acts::Result<
            Acts::CombinatorialKalmanFilterResult<Acts::VectorMultiTrajectory>>>;

        /// Find function that takes the above parameters
        /// @note This is separated into a virtual interface to keep compilation units
        /// small
        class CKFTrackingFunction {
        public:
            virtual ~CKFTrackingFunction() = default;

            virtual TrackFinderResult operator()(const ActsExamples::TrackParametersContainer &,
                                                 const TrackFinderOptions &) const = 0;
        };

        /// Create the track finder function implementation.
        /// The magnetic field is intentionally given by-value since the variantresults
        /// contains shared_ptr anyways.
        static std::shared_ptr<CKFTrackingFunction> makeCKFTrackingFunction(
                std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
                std::shared_ptr<const Acts::MagneticFieldProvider> magneticField);

        CKFTracking();

        void init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> log);

        std::tuple<
            std::unique_ptr<edm4eic::TrajectoryCollection>,
            std::unique_ptr<edm4eic::TrackParametersCollection>,
            std::vector<ActsExamples::Trajectories*>
        >
        process(const ActsExamples::IndexSourceLinkContainer &src_links,
                const ActsExamples::MeasurementContainer &measurements,
                const edm4eic::TrackParametersCollection &init_trk_params);

    private:
        std::shared_ptr<spdlog::logger> m_log;
        std::shared_ptr<CKFTrackingFunction> m_trackFinderFunc;
        std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

        std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
        Acts::GeometryContext m_geoctx;
        Acts::CalibrationContext m_calibctx;
        Acts::MagneticFieldContext m_fieldctx;

        Acts::MeasurementSelector::Config m_sourcelinkSelectorCfg;
    };

} // namespace eicrecon::Reco
