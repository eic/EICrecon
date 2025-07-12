// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten

#include <Acts/Definitions/Direction.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/MeasurementHelpers.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
#if Acts_VERSION_MAJOR < 36
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#endif
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/TrackFinding/CombinatorialKalmanFilter.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <algorithm>
#include <any>
#include <cmath>
#include <memory>
#include <string>
#include <utility>

#include "ActsExamples/EventData/Track.hpp"
#include "CKFTracking.h"

namespace eicrecon {

using Stepper    = Acts::EigenStepper<>;
using Navigator  = Acts::Navigator;
using Propagator = Acts::Propagator<Stepper, Navigator>;

/** Finder implementation .
   *
   * \ingroup track
   */
template <typename edm_t>
struct CKFTrackingFunctionImpl : public eicrecon::CKFTracking<edm_t>::CKFTrackingFunction {

#if Acts_VERSION_MAJOR >= 36
  using CKF = Acts::CombinatorialKalmanFilter<Propagator, typename edm_t::TrackContainer>;
#else
  using CKF =
      Acts::CombinatorialKalmanFilter<Propagator, typename edm_t::TrackStateContainerBackend>;
#endif

  CKF trackFinder;

  CKFTrackingFunctionImpl(CKF&& f) : trackFinder(std::move(f)) {}

  eicrecon::CKFTracking<edm_t>::TrackFinderResult
  operator()(const edm_t::TrackParameters& initialParameters,
             const eicrecon::CKFTracking<edm_t>::TrackFinderOptions& options,
             edm_t::TrackContainer& tracks) const override {
    return trackFinder.findTracks(initialParameters, options, tracks);
  };
};

template struct CKFTrackingFunctionImpl<ActsExamplesEdm>;
template <typename edm_t>
std::shared_ptr<typename CKFTracking<edm_t>::CKFTrackingFunction>
CKFTracking<edm_t>::makeCKFTrackingFunction(
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
    std::shared_ptr<const Acts::MagneticFieldProvider> magneticField, const Acts::Logger& logger) {
  Stepper stepper(std::move(magneticField));
  Navigator::Config cfg{.trackingGeometry = trackingGeometry};
  cfg.resolvePassive   = false;
  cfg.resolveMaterial  = true;
  cfg.resolveSensitive = true;
  Navigator navigator(cfg);

  Propagator propagator(std::move(stepper), std::move(navigator));
  typename CKFTrackingFunctionImpl<edm_t>::CKF trackFinder(std::move(propagator),
                                                           logger.cloneWithSuffix("CKF"));

  // build the track finder functions. owns the track finder object.
  return std::make_shared<CKFTrackingFunctionImpl>(std::move(trackFinder));
}

// FIXME why can't the following duplication be avoided with the explicit template specialization?
template <>
std::shared_ptr<typename CKFTracking<ActsExamplesEdm>::CKFTrackingFunction>
CKFTracking<ActsExamplesEdm>::makeCKFTrackingFunction(
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
    std::shared_ptr<const Acts::MagneticFieldProvider> magneticField, const Acts::Logger& logger);

// FIXME but why is this explicit template specialization with definition needed?
template <>
std::shared_ptr<typename CKFTracking<ActsExamplesEdm>::CKFTrackingFunction>
CKFTracking<ActsExamplesEdm>::makeCKFTrackingFunction(
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
    std::shared_ptr<const Acts::MagneticFieldProvider> magneticField, const Acts::Logger& logger) {
  Stepper stepper(std::move(magneticField));
  Navigator::Config cfg{trackingGeometry};
  cfg.resolvePassive   = false;
  cfg.resolveMaterial  = true;
  cfg.resolveSensitive = true;
  Navigator navigator(cfg);

  Propagator propagator(std::move(stepper), std::move(navigator));
  typename CKFTrackingFunctionImpl<ActsExamplesEdm>::CKF trackFinder(std::move(propagator),
                                                                     logger.cloneWithSuffix("CKF"));

  // build the track finder functions. owns the track finder object.
  return std::make_shared<CKFTrackingFunctionImpl<ActsExamplesEdm>>(std::move(trackFinder));
}

} // namespace eicrecon
