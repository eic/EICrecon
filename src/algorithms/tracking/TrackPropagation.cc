// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Wenqing Fan, Barak Schmookler, Whitney Armstrong, Sylvester Joosten, Dmitry Romanov, Christopher Dilks, Wouter Deconinck

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Direction.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Surfaces/CylinderBounds.hpp>
#include <Acts/Surfaces/CylinderSurface.hpp>
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <DD4hep/Handle.h>
#include <Evaluator/DD4hepUnits.h>
#include <boost/container/vector.hpp>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <stddef.h>
#include <stdint.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>
#include <exception>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <typeinfo>
#include <variant>

#include "algorithms/tracking/ActsGeometryProvider.h"
#include "algorithms/tracking/TrackPropagation.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "extensions/spdlog/SpdlogToActs.h"

namespace eicrecon {

template<typename ...L>
struct multilambda : L... {
  using L::operator()...;
  constexpr multilambda(L...lambda) : L(std::move(lambda))... {}
};

void TrackPropagation::init(const dd4hep::Detector* detector,
                            std::shared_ptr<const ActsGeometryProvider> geo_svc,
                            std::shared_ptr<spdlog::logger> logger) {
    m_geoSvc = geo_svc;
    m_log = logger;

    std::map<uint32_t,size_t> system_id_layers;

    multilambda _toDouble = {
      [](const std::string& v) { return dd4hep::_toDouble(v); },
      [](const double& v)      { return v; },
    };

    auto _toActsSurface = [&_toDouble, &detector, &system_id_layers](
      const std::variant<CylinderSurfaceConfig,DiscSurfaceConfig> surface_variant) -> std::shared_ptr<Acts::Surface> {
      if (std::holds_alternative<CylinderSurfaceConfig>(surface_variant)) {
        CylinderSurfaceConfig surface = std::get<CylinderSurfaceConfig>(surface_variant);
        const double rmin = std::visit(_toDouble, surface.rmin) / dd4hep::mm * Acts::UnitConstants::mm;
        const double zmin = std::visit(_toDouble, surface.zmin) / dd4hep::mm * Acts::UnitConstants::mm;
        const double zmax = std::visit(_toDouble, surface.zmax) / dd4hep::mm * Acts::UnitConstants::mm;
        const uint32_t system_id = detector->constant<uint32_t>(surface.id);
        auto bounds = std::make_shared<Acts::CylinderBounds>(rmin, (zmax-zmin)/2);
        auto t = Acts::Translation3(Acts::Vector3(0, 0, (zmax+zmin)/2));
        auto tf = Acts::Transform3(t);
        auto acts_surface = Acts::Surface::makeShared<Acts::CylinderSurface>(tf, bounds);
        acts_surface->assignGeometryId(Acts::GeometryIdentifier().setExtra(system_id).setLayer(++system_id_layers[system_id]));
        return acts_surface;
      }
      if (std::holds_alternative<DiscSurfaceConfig>(surface_variant)) {
        DiscSurfaceConfig surface = std::get<DiscSurfaceConfig>(surface_variant);
        const double zmin = std::visit(_toDouble, surface.zmin) / dd4hep::mm * Acts::UnitConstants::mm;
        const double rmin = std::visit(_toDouble, surface.rmin) / dd4hep::mm * Acts::UnitConstants::mm;
        const double rmax = std::visit(_toDouble, surface.rmax) / dd4hep::mm * Acts::UnitConstants::mm;
        const uint32_t system_id = detector->constant<uint32_t>(surface.id);
        auto bounds = std::make_shared<Acts::RadialBounds>(rmin, rmax);
        auto t = Acts::Translation3(Acts::Vector3(0, 0, zmin));
        auto tf = Acts::Transform3(t);
        auto acts_surface = Acts::Surface::makeShared<Acts::DiscSurface>(tf, bounds);
        acts_surface->assignGeometryId(Acts::GeometryIdentifier().setExtra(system_id).setLayer(++system_id_layers[system_id]));
        return acts_surface;
      }
      throw std::domain_error("Unknown surface type");
    };
    std::transform(m_cfg.target_surfaces.cbegin(), m_cfg.target_surfaces.cend(), m_target_surfaces.begin(), _toActsSurface);
    std::transform(m_cfg.filter_surfaces.cbegin(), m_cfg.filter_surfaces.cend(), m_filter_surfaces.begin(), _toActsSurface);

    m_log->trace("Initialized");
}



    std::unique_ptr<edm4eic::TrackSegmentCollection> TrackPropagation::propagateToSurfaceList(
        std::vector<const ActsExamples::Trajectories*> trajectories,
        std::vector<std::shared_ptr<Acts::Surface>> targetSurfaces,
        std::shared_ptr<Acts::Surface> filterSurface,
        std::function<bool(edm4eic::TrackPoint)> trackPointCut,
        bool stopIfTrackPointCutFailed
        ) const
    {
      // logging
      m_log->trace("Propagate trajectories: --------------------");
      m_log->trace("number of trajectories: {}",trajectories.size());

      // start output collection
      auto track_segments = std::make_unique<edm4eic::TrackSegmentCollection>();

      // loop over input trajectories
      for(const auto& traj : trajectories) {

        // check if this trajectory can be propagated to `filterSurface`
        if(filterSurface) {
          try {
            if(!propagate(traj, filterSurface)) {
              m_log->trace("<> Skip this trajectory, since it cannot be propagated to filterSurface");
              continue;
            }
          } catch(std::exception &e) {
            m_log->warn("<> Exception in TrackPropagation::propagateToSurfaceList: {}; skip this TrackPoint and surface", e.what());
            continue;
          }
        }

        // start a mutable TrackSegment
        auto track_segment = track_segments->create();
        decltype(edm4eic::TrackSegmentData::length)      length       = 0;
        decltype(edm4eic::TrackSegmentData::lengthError) length_error = 0;

        // loop over projection-target surfaces
        for(const auto& targetSurf : targetSurfaces) {

          // project the trajectory `traj` to this surface
          std::unique_ptr<edm4eic::TrackPoint> point;
          try {
            point = propagate(traj, targetSurf);
          } catch(std::exception &e) {
            m_log->warn("<> Exception in TrackPropagation::propagateToSurfaceList: {}; skip this TrackPoint and surface", e.what());
            continue;
          }
          if(!point) {
            m_log->trace("<> Failed to propagate trajectory to this plane");
            continue;
          }

          // logging
          m_log->trace("<> trajectory: x=( {:>10.2f} {:>10.2f} {:>10.2f} )",
              point->position.x, point->position.y, point->position.z);
          m_log->trace("               p=( {:>10.2f} {:>10.2f} {:>10.2f} )",
              point->momentum.x, point->momentum.y, point->momentum.z);

          // track point cut
          if(!trackPointCut(*point)) {
            m_log->trace("                 => REJECTED by trackPointCut");
            if(stopIfTrackPointCutFailed)
              break;
            continue;
          }

          // update the `TrackSegment` length
          // FIXME: `length` and `length_error` are currently not used by any callers, and may not be correctly calculated here
          if(track_segment.points_size()>0) {
            auto pos0 = point->position;
            auto pos1 = std::prev(track_segment.points_end())->position;
            auto dist = edm4hep::utils::magnitude(pos0-pos1);
            length += dist;
            m_log->trace("               dist to previous point: {}", dist);
          }

          // add the `TrackPoint` to the `TrackSegment`
          track_segment.addToPoints(*point);

        } // end `targetSurfaces` loop

        // set final length and length error
        track_segment.setLength(length);
        track_segment.setLengthError(length_error);

      } // end loop over input trajectories

      return track_segments;
    }



    std::unique_ptr<edm4eic::TrackPoint> TrackPropagation::propagate(const ActsExamples::Trajectories *traj,
                                                     const std::shared_ptr<const Acts::Surface> &targetSurf) const {
        // Get the entry index for the single trajectory
        // The trajectory entry indices and the multiTrajectory
        const auto &mj = traj->multiTrajectory();
        const auto &trackTips = traj->tips();

        m_log->trace("  Number of elements in trackTips {}", trackTips.size());

        // Skip empty
        if (trackTips.empty()) {
            m_log->trace("  Empty multiTrajectory.");
            return nullptr;
        }
        const auto &trackTip = trackTips.front();

        // Collect the trajectory summary info
        auto trajState = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
        int m_nMeasurements = trajState.nMeasurements;
        int m_nStates = trajState.nStates;

        m_log->trace("  Num measurement in trajectory: {}", m_nMeasurements);
        m_log->trace("  Num states in trajectory     : {}", m_nStates);



        //=================================================
        //Track projection
        //Reference sPHENIX code: https://github.com/sPHENIX-Collaboration/coresoftware/blob/335e6da4ccacc8374cada993485fe81d82e74a4f/offline/packages/trackreco/PHActsTrackProjection.h
        //=================================================
        const auto &initial_bound_parameters = traj->trackParameters(trackTip);


        m_log->trace("    TrackPropagation. Propagating to surface # {}", typeid(targetSurf->type()).name());

        std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry = m_geoSvc->trackingGeometry();
        std::shared_ptr<const Acts::MagneticFieldProvider> magneticField = m_geoSvc->getFieldProvider();
        using Stepper = Acts::EigenStepper<>;
        using Propagator = Acts::Propagator<Stepper>;
        Stepper stepper(magneticField);
        Propagator propagator(stepper);

        ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("PROP", m_log));

        Acts::PropagatorOptions<> options(m_geoContext, m_fieldContext);

        auto result = propagator.propagate(initial_bound_parameters, *targetSurf, options);

        // check propagation result
        if (!result.ok()) {
            m_log->trace("    propagation failed (!result.ok())");
            return nullptr;
        }
        m_log->trace("    propagation result is OK");

        // Pulling results to convenient variables
        auto trackStateParams = *((*result).endParameters);
        const auto &parameter = trackStateParams.parameters();
        const auto &covariance = *trackStateParams.covariance();

        // Path length
        const float pathLength = (*result).pathLength;
        const float pathLengthError = 0;
        m_log->trace("    path len = {}", pathLength);

        // Position:
        auto projectionPos = trackStateParams.position(m_geoContext);
        const decltype(edm4eic::TrackPoint::position) position{
                static_cast<float>(projectionPos(0)),
                static_cast<float>(projectionPos(1)),
                static_cast<float>(projectionPos(2))
        };
        const decltype(edm4eic::TrackPoint::positionError) positionError{0, 0, 0};
        m_log->trace("    pos x = {}", position.x);
        m_log->trace("    pos y = {}", position.y);
        m_log->trace("    pos z = {}", position.z);

        // Momentum
        const decltype(edm4eic::TrackPoint::momentum) momentum = edm4hep::utils::sphericalToVector(
                static_cast<float>(1.0 / std::abs(parameter[Acts::eBoundQOverP])),
                static_cast<float>(parameter[Acts::eBoundTheta]),
                static_cast<float>(parameter[Acts::eBoundPhi])
        );
        const decltype(edm4eic::TrackPoint::momentumError) momentumError{
                static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
                static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
                static_cast<float>(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)),
                static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundPhi)),
                static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundQOverP)),
                static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundQOverP))
        };

        // time
        const float time{static_cast<float>(parameter(Acts::eBoundTime))};
        const float timeError{sqrt(static_cast<float>(covariance(Acts::eBoundTime, Acts::eBoundTime)))};

        // Direction
        const float theta(parameter[Acts::eBoundTheta]);
        const float phi(parameter[Acts::eBoundPhi]);
        const decltype(edm4eic::TrackPoint::directionError) directionError{
                static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
                static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
                static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundPhi))
        };


        // >oO debug print
        m_log->trace("    loc 0   = {:.4f}", parameter[Acts::eBoundLoc0]);
        m_log->trace("    loc 1   = {:.4f}", parameter[Acts::eBoundLoc1]);
        m_log->trace("    phi     = {:.4f}", parameter[Acts::eBoundPhi]);
        m_log->trace("    theta   = {:.4f}", parameter[Acts::eBoundTheta]);
        m_log->trace("    q/p     = {:.4f}", parameter[Acts::eBoundQOverP]);
        m_log->trace("    p       = {:.4f}", 1.0 / parameter[Acts::eBoundQOverP]);
        m_log->trace("    err phi = {:.4f}", sqrt(covariance(Acts::eBoundPhi, Acts::eBoundPhi)));
        m_log->trace("    err th  = {:.4f}", sqrt(covariance(Acts::eBoundTheta, Acts::eBoundTheta)));
        m_log->trace("    err q/p = {:.4f}", sqrt(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)));
        m_log->trace("    chi2    = {:.4f}", trajState.chi2Sum);
        m_log->trace("    loc err = {:.4f}", static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc0)));
        m_log->trace("    loc err = {:.4f}", static_cast<float>(covariance(Acts::eBoundLoc1, Acts::eBoundLoc1)));
        m_log->trace("    loc err = {:.4f}", static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc1)));

        uint64_t surface = targetSurf->geometryId().value();
        uint32_t system = 0; // default value...will be set in TrackPropagation factory

        /*
         ::edm4hep::Vector3f position{}; ///< Position of the trajectory point [mm]
          ::edm4eic::Cov3f positionError{}; ///< Error on the position
          ::edm4hep::Vector3f momentum{}; ///< 3-momentum at the point [GeV]
          ::edm4eic::Cov3f momentumError{}; ///< Error on the 3-momentum
          float time{}; ///< Time at this point [ns]
          float timeError{}; ///< Error on the time at this point
          float theta{}; ///< polar direction of the track at the surface [rad]
          float phi{}; ///< azimuthal direction of the track at the surface [rad]
          ::edm4eic::Cov2f directionError{}; ///< Error on the polar and azimuthal angles
          float pathlength{}; ///< Pathlength from the origin to this point
          float pathlengthError{}; ///< Error on the pathlenght
         */
        return std::make_unique<edm4eic::TrackPoint>(edm4eic::TrackPoint{
                                               surface,
                                               system,
                                               position,
                                               positionError,
                                               momentum,
                                               momentumError,
                                               time,
                                               timeError,
                                               theta,
                                               phi,
                                               directionError,
                                               pathLength,
                                               pathLengthError
                                       });
    }

} // namespace eicrecon
