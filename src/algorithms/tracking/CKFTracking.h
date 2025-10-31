// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck

#pragma once

#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/TrackFinding/CombinatorialKalmanFilter.hpp>
#include <Acts/TrackFinding/MeasurementSelector.hpp>
#include <Acts/Utilities/CalibrationContext.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/Result.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <tuple>
#include <variant>
#include <vector>

#include "CKFTrackingConfig.h"
#include "DD4hepBField.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/ActsExamplesEdm.h"
#include "algorithms/tracking/ActsPodioEdm.h"

class ActsGeometryProvider;

namespace eicrecon {

/** Fitting algorithm implementation .
 *
 * \ingroup tracking
 */

template <typename edm_t = eicrecon::ActsExamplesEdm>
class CKFTracking : public WithPodConfig<eicrecon::CKFTrackingConfig> {
public:
  /// Track finder function that takes input measurements, initial trackstate
  /// and track finder options and returns some track-finder-specific result.
#if Acts_VERSION_MAJOR >= 39
  using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<typename edm_t::TrackContainer>;
#elif Acts_VERSION_MAJOR >= 36
  using TrackFinderOptions =
      Acts::CombinatorialKalmanFilterOptions<ActsExamples::IndexSourceLinkAccessor::Iterator,
                                             typename edm_t::TrackContainer>;
#else
  using TrackFinderOptions =
      Acts::CombinatorialKalmanFilterOptions<ActsExamples::IndexSourceLinkAccessor::Iterator,
                                             Acts::VectorMultiTrajectory>;
#endif
  using TrackFinderResult = Acts::Result<std::vector<typename edm_t::TrackContainer::TrackProxy>>;

  /// Find function that takes the above parameters
  /// @note This is separated into a virtual interface to keep compilation units
  /// small
  class CKFTrackingFunction {
  public:
    virtual ~CKFTrackingFunction() = default;

    virtual TrackFinderResult operator()(const edm_t::TrackParameters&, const TrackFinderOptions&,
                                         edm_t::TrackContainer&) const = 0;
  };

  /// Create the track finder function implementation.
  /// The magnetic field is intentionally given by-value since the variantresults
  /// contains shared_ptr anyways.
  static std::shared_ptr<CKFTrackingFunction>
  makeCKFTrackingFunction(std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
                          std::shared_ptr<const Acts::MagneticFieldProvider> magneticField,
                          const Acts::Logger& logger);

  CKFTracking() = default;

  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> log);

  std::tuple<std::vector<typename edm_t::Trajectories*>,
             std::vector<typename edm_t::ConstTrackContainer*>>
  process(const edm4eic::TrackParametersCollection& init_trk_params,
          const edm4eic::Measurement2DCollection& meas2Ds);

private:
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const Acts::Logger> m_acts_logger{nullptr};
  std::shared_ptr<CKFTrackingFunction> m_trackFinderFunc;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::CalibrationContext m_calibctx;
  Acts::MagneticFieldContext m_fieldctx;

  Acts::MeasurementSelector::Config m_sourcelinkSelectorCfg;

  /// Private access to the logging instance
  const Acts::Logger& logger() const { return *m_acts_logger; }
};

} // namespace eicrecon
