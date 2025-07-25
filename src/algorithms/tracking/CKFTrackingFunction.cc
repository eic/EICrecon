// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten

#include <Acts/Definitions/Direction.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
#if Acts_VERSION_MAJOR < 36
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#endif
#include <Acts/Geometry/Layer.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/TrackFinding/CombinatorialKalmanFilter.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <boost/container/vector.hpp>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <memory>
#include <utility>

#include "ActsExamples/EventData/Track.hpp"
#include "CKFTracking.h"

namespace eicrecon {

using Stepper    = Acts::EigenStepper<>;
using Navigator  = Acts::Navigator;
using Propagator = Acts::Propagator<Stepper, Navigator>;

#if Acts_VERSION_MAJOR >= 36
using CKF = Acts::CombinatorialKalmanFilter<Propagator, ActsExamples::TrackContainer>;
#else
using CKF = Acts::CombinatorialKalmanFilter<Propagator, Acts::VectorMultiTrajectory>;

using TrackContainer =
    Acts::TrackContainer<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, std::shared_ptr>;
#endif

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
#if Acts_VERSION_MAJOR >= 36
             ActsExamples::TrackContainer& tracks) const override {
#else
             TrackContainer& tracks) const override {
#endif
    return trackFinder.findTracks(initialParameters, options, tracks);
  };
};

} // namespace eicrecon

namespace eicrecon {

std::shared_ptr<CKFTracking::CKFTrackingFunction> CKFTracking::makeCKFTrackingFunction(
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
    std::shared_ptr<const Acts::MagneticFieldProvider> magneticField, const Acts::Logger& logger) {
  Stepper stepper(std::move(magneticField));
  Navigator::Config cfg{trackingGeometry};
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
