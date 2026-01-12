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
#if Acts_VERSION_MAJOR < 39
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#endif
#include <ActsExamples/EventData/Track.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <memory>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include "CKFTrackingConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

class ActsGeometryProvider;

namespace eicrecon {

using CKFTrackingAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackParametersCollection, edm4eic::Measurement2DCollection>,
    algorithms::Output<Acts::ConstVectorMultiTrajectory*, Acts::ConstVectorTrackContainer*>>;

/** Fitting algorithm implementation .
 *
 * \ingroup tracking
 */

class CKFTracking : public CKFTrackingAlgorithm, public WithPodConfig<eicrecon::CKFTrackingConfig> {
public:
  /// Track finder function that takes input measurements, initial trackstate
  /// and track finder options and returns some track-finder-specific result.
#if Acts_VERSION_MAJOR >= 39
  using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<ActsExamples::TrackContainer>;
#else
  using TrackFinderOptions =
      Acts::CombinatorialKalmanFilterOptions<ActsExamples::IndexSourceLinkAccessor::Iterator,
                                             ActsExamples::TrackContainer>;
#endif
  using TrackFinderResult = Acts::Result<std::vector<ActsExamples::TrackContainer::TrackProxy>>;

  /// Find function that takes the above parameters
  /// @note This is separated into a virtual interface to keep compilation units
  /// small
  class CKFTrackingFunction {
  public:
    virtual ~CKFTrackingFunction() = default;

    virtual TrackFinderResult operator()(const ActsExamples::TrackParameters&,
                                         const TrackFinderOptions&,
                                         ActsExamples::TrackContainer&) const = 0;
  };

  /// Create the track finder function implementation.
  /// The magnetic field is intentionally given by-value since the variantresults
  /// contains shared_ptr anyways.
  static std::shared_ptr<CKFTrackingFunction>
  makeCKFTrackingFunction(std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
                          std::shared_ptr<const Acts::MagneticFieldProvider> magneticField,
                          const Acts::Logger& logger);

  CKFTracking(std::string_view name)
      : CKFTrackingAlgorithm{name,
                             {"inputTrackParameters", "inputMeasurements"},
                             {"outputActsTrackStates", "outputActsTracks"},
                             "Combinatorial Kalman Filter track finding"} {}

  void setGeometryService(std::shared_ptr<const ActsGeometryProvider> geo_svc) {
    m_geoSvc = geo_svc;
  }

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  std::shared_ptr<const Acts::Logger> m_acts_logger{nullptr};
  std::shared_ptr<CKFTrackingFunction> m_trackFinderFunc;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  Acts::MeasurementSelector::Config m_sourcelinkSelectorCfg;

  /// Private access to the logging instance
  const Acts::Logger& acts_logger() const { return *m_acts_logger; }
};

} // namespace eicrecon
