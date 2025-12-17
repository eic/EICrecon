// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten

#include <Acts/Definitions/Direction.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/MeasurementHelpers.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
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

using CKF = Acts::CombinatorialKalmanFilter<Propagator, ActsExamples::TrackContainer>;

/** Finder implementation .
   *
   * \ingroup track
   */
struct CKFTrackingFunctionImpl : public eicrecon::CKFTracking::CKFTrackingFunction {
  CKF trackFinder;

  CKFTrackingFunctionImpl(CKF&& f) : trackFinder(std::move(f)) {}

  eicrecon::CKFTracking::TrackFinderResult
  operator()(const ActsExamples::TrackParameters& initialParameters,
             const eicrecon::CKFTracking::TrackFinderOptions& options,
             ActsExamples::TrackContainer& tracks) const override {
    return trackFinder.findTracks(initialParameters, options, tracks);
  };
};

} // namespace eicrecon

namespace eicrecon {

std::shared_ptr<CKFTracking::CKFTrackingFunction> CKFTracking::makeCKFTrackingFunction(
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
    std::shared_ptr<const Acts::MagneticFieldProvider> magneticField, const Acts::Logger& logger) {
  Stepper stepper(std::move(magneticField));
  Navigator::Config cfg{.trackingGeometry = trackingGeometry};
  cfg.resolvePassive   = false;
  cfg.resolveMaterial  = true;
  cfg.resolveSensitive = true;
  Navigator navigator(cfg);

  Propagator propagator(std::move(stepper), std::move(navigator));
  CKF trackFinder(std::move(propagator), logger.cloneWithSuffix("CKF"));

  // build the track finder functions. owns the track finder object.
  return std::make_shared<CKFTrackingFunctionImpl>(std::move(trackFinder));
}

} // namespace eicrecon
