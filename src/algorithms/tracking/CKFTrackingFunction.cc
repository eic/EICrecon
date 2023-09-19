// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten

#include <Acts/Definitions/Common.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
#include <Acts/Geometry/detail/Layer.ipp>
#include <Acts/Geometry/detail/TrackingVolume.ipp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/EigenStepper.ipp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/TrackFinding/CombinatorialKalmanFilter.hpp>
#include <Acts/TrackFitting/GainMatrixSmoother.hpp>
#include <Acts/TrackFitting/GainMatrixUpdater.hpp>
#include <Eigen/src/Core/Assign.h>
#include <Eigen/src/Core/AssignEvaluator.h>
#include <Eigen/src/Core/CwiseBinaryOp.h>
#include <Eigen/src/Core/CwiseNullaryOp.h>
#include <Eigen/src/Core/Dot.h>
#include <Eigen/src/Core/GeneralProduct.h>
#include <Eigen/src/Core/GenericPacketMath.h>
#include <Eigen/src/Core/IO.h>
#include <Eigen/src/Core/Redux.h>
#include <Eigen/src/Core/SelfCwiseBinaryOp.h>
#include <Eigen/src/Core/Transpose.h>
#include <Eigen/src/Core/arch/SSE/PacketMath.h>
#include <Eigen/src/Core/util/Memory.h>
#include <Eigen/src/Geometry/OrthoMethods.h>
#include <boost/container/vector.hpp>
#include <boost/move/utility_core.hpp>
#include <algorithm>
#include <map>
#include <memory>
#include <utility>
#include <variant>

#include "ActsExamples/EventData/Track.hpp"
#include "CKFTracking.h"
#include "src/Core/ArrayBase.h"
#include "src/Core/DenseBase.h"

namespace Acts { class MagneticFieldProvider; }
namespace Acts { class TrackingGeometry; }
namespace Acts { class VectorMultiTrajectory; }

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
    operator()(const ActsExamples::TrackParametersContainer& initialParameters,
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

    // build the track finder functions. owns the track finder object.
    return std::make_shared<CKFTrackingFunctionImpl>(std::move(trackFinder));
  }

} // namespace eicrecon::Reco
