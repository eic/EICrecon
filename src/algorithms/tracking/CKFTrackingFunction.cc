// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten

#include <Acts/EventData/MultiTrajectory.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/TrackFinding/CombinatorialKalmanFilter.hpp>
#include <Acts/TrackFitting/GainMatrixSmoother.hpp>
#include <Acts/TrackFitting/GainMatrixUpdater.hpp>
#include <Acts/Utilities/Intersection.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "ActsExamples/EventData/Track.hpp"
#include "CKFTracking.h"

namespace eicrecon{

  using Updater  = Acts::GainMatrixUpdater;
  using Smoother = Acts::GainMatrixSmoother;

  using Stepper    = Acts::EigenStepper<>;
  using Navigator  = Acts::Navigator;
  using Propagator = Acts::Propagator<Stepper, Navigator>;

  using CKF =
      Acts::CombinatorialKalmanFilter<Propagator, Acts::VectorMultiTrajectory>;

  using TrackContainer =
      Acts::TrackContainer<Acts::VectorTrackContainer,
                           Acts::VectorMultiTrajectory, std::shared_ptr>;

  /** Finder implementation .
   *
   * \ingroup track
   */
  struct CKFTrackingFunctionImpl
      : public eicrecon::CKFTracking::CKFTrackingFunction {
    CKF trackFinder;

    CKFTrackingFunctionImpl(CKF&& f) : trackFinder(std::move(f)) {}

    eicrecon::CKFTracking::TrackFinderResult operator()(
        const ActsExamples::TrackParameters& initialParameters,
        const eicrecon::CKFTracking::TrackFinderOptions& options,
        TrackContainer& tracks) const override {
      return trackFinder.findTracks(initialParameters, options, tracks);
    };
  };

} // namespace

namespace eicrecon {

  std::shared_ptr<CKFTracking::CKFTrackingFunction>
  CKFTracking::makeCKFTrackingFunction(
      std::shared_ptr<const Acts::TrackingGeometry>      trackingGeometry,
      std::shared_ptr<const Acts::MagneticFieldProvider> magneticField,
      const Acts::Logger& logger)
  {
    Stepper   stepper(std::move(magneticField));
    Navigator::Config cfg{trackingGeometry};
    cfg.resolvePassive   = false;
    cfg.resolveMaterial  = true;
    cfg.resolveSensitive = true;
    Navigator navigator(cfg);

    Propagator propagator(std::move(stepper), std::move(navigator));
    CKF        trackFinder(std::move(propagator), logger.cloneWithSuffix("CKF"));

    // build the track finder functions. owns the track finder object.
    return std::make_shared<CKFTrackingFunctionImpl>(std::move(trackFinder));
  }

} // namespace eicrecon::Reco
