// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten

#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/TrackFitting/GainMatrixSmoother.hpp>
#include <Acts/TrackFitting/GainMatrixUpdater.hpp>

#if 0
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/TrackFitting/GainMatrixSmoother.hpp>
#include <Acts/TrackFitting/GainMatrixUpdater.hpp>
#endif

#include "CKFTracking.h"

#include "JugBase/BField/DD4hepBField.h"


#include <random>
#include <stdexcept>

namespace eicrecon{
  using Updater  = Acts::GainMatrixUpdater;
  using Smoother = Acts::GainMatrixSmoother;

  using Stepper    = Acts::EigenStepper<>;
  using Navigator  = Acts::Navigator;
  using Propagator = Acts::Propagator<Stepper, Navigator>;
  using CKF =
      Acts::CombinatorialKalmanFilter<Propagator, Acts::VectorMultiTrajectory>;

  /** Finder implementation .
   *
   * \ingroup track
   */
  struct CKFTrackingFunctionImpl
: public eicrecon::CKFTracking::CKFTrackingFunction {
    CKF trackFinder;

    CKFTrackingFunctionImpl(CKF&& f) : trackFinder(std::move(f)) {}

    eicrecon::CKFTracking::TrackFinderResult
    operator()(const eicrecon::TrackParametersContainer& initialParameters,
               const eicrecon::CKFTracking::TrackFinderOptions& options)
               const override
    {
        return trackFinder.findTracks(initialParameters, options);
    };
  };

} // namespace

namespace eicrecon {

  std::shared_ptr<CKFTracking::CKFTrackingFunction>
  CKFTracking::makeCKFTrackingFunction(
      std::shared_ptr<const Acts::TrackingGeometry>      trackingGeometry,
      std::shared_ptr<const Acts::MagneticFieldProvider> magneticField)
  {
    Stepper   stepper(std::move(magneticField));
    Navigator::Config cfg{trackingGeometry};
    cfg.resolvePassive   = false;
    cfg.resolveMaterial  = true;
    cfg.resolveSensitive = true;
    Navigator navigator(cfg);

    Propagator propagator(std::move(stepper), std::move(navigator));
    CKF        trackFinder(std::move(propagator));

    // build the track finder functions. onws the track finder object.
    return std::make_shared<CKFTrackingFunctionImpl>(std::move(trackFinder));
  }

} // namespace Jug::Reco
