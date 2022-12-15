// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck

#ifndef JUGGLER_JUGRECO_CKFTracking_HH
#define JUGGLER_JUGRECO_CKFTracking_HH

#include <functional>
#include <random>
#include <stdexcept>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

//#include "JugBase/DataHandle.h"

#include "JugBase/BField/DD4hepBField.h"
#include "JugTrack/GeometryContainers.hpp"
#include "JugTrack/Index.hpp"
#include "JugTrack/IndexSourceLink.hpp"
#include "JugTrack/Measurement.hpp"
#include "JugTrack/Track.hpp"
#include "JugTrack/TrackingResultTrajectory.hpp"

#include "edm4eic/TrackerHitCollection.h"
#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <spdlog/logger.h>

#include "Acts/Definitions/Common.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "CKFTrackingConfig.h"

#include <algorithms/interfaces/WithPodConfig.h>

class ActsGeometryProvider;

namespace eicrecon {

/** Fitting algorithm implmentation .
 *
 * \ingroup tracking
 */

    class CKFTracking: public WithPodConfig<eicrecon::CKFTrackingConfig> {
    public:
        /// Track finder function that takes input measurements, initial trackstate
        /// and track finder options and returns some track-finder-specific result.
        using TrackFinderOptions =
            Acts::CombinatorialKalmanFilterOptions<IndexSourceLinkAccessor::Iterator,
                                                   Acts::VectorMultiTrajectory>;
        using TrackFinderResult = std::vector<Acts::Result<
            Acts::CombinatorialKalmanFilterResult<Acts::VectorMultiTrajectory>>>;

        /// Find function that takes the above parameters
        /// @note This is separated into a virtual interface to keep compilation units
        /// small
        class CKFTrackingFunction {
        public:
            virtual ~CKFTrackingFunction() = default;

            virtual TrackFinderResult operator()(const eicrecon::TrackParametersContainer &,
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

        std::vector<eicrecon::TrackingResultTrajectory*> process(const eicrecon::IndexSourceLinkContainer &src_links,
                                                                 const eicrecon::MeasurementContainer &measurements,
                                                                 const eicrecon::TrackParametersContainer &init_trk_params);

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

} // namespace Jug::Reco

#endif
