// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Dmitry Kalinkin

#include "TwoTrackVertexSeeder.h"

#include <Acts/Definitions/Units.hpp>
#include <Acts/Vertexing/TrackAtVertex.hpp>
#include <array>
#include <cmath>

namespace eicrecon {

namespace {

  /// Helix through a track's perigee position, parametrised by the transverse
  /// path length s
  struct Helix {
    Acts::Vector3 origin;
    double phi0;
    double cotTheta;
    /// Signed curvature dphi/ds, zero for a straight line
    double h;

    Acts::Vector3 position(double s) const {
      if (std::abs(h * s) < 1e-9) {
        return origin + s * Acts::Vector3(std::cos(phi0), std::sin(phi0), cotTheta);
      }
      const double phi = phi0 + h * s;
      return origin + Acts::Vector3((std::sin(phi) - std::sin(phi0)) / h,
                                    (std::cos(phi0) - std::cos(phi)) / h, s * cotTheta);
    }

    Acts::Vector3 tangent(double s) const {
      const double phi = phi0 + h * s;
      return {std::cos(phi), std::sin(phi), cotTheta};
    }
  };

  Helix makeHelix(const Acts::BoundTrackParameters& track, const Acts::GeometryContext& gctx,
                  double bz) {
    const double pt = track.transverseMomentum();
    // A positive charge in a field along +z bends clockwise
    const double h = (pt > 0) ? -track.charge() * bz / pt : 0.;
    return {track.position(gctx), track.phi(), 1. / std::tan(track.theta()), h};
  }

} // namespace

Acts::Vector3 TwoTrackVertexSeeder::closestApproach(const Acts::BoundTrackParameters& track1,
                                                    const Acts::BoundTrackParameters& track2,
                                                    const Acts::GeometryContext& gctx,
                                                    double bz) const {
  const Helix h1 = makeHelix(track1, gctx, bz);
  const Helix h2 = makeHelix(track2, gctx, bz);

  // Minimise |h1(s1) - h2(s2)|^2, linearising both helices around the current
  // points at each step, starting from the perigees
  double s1 = 0.;
  double s2 = 0.;
  for (unsigned int i = 0; i < m_cfg.maxIterations; ++i) {
    const Acts::Vector3 w  = h1.position(s1) - h2.position(s2);
    const Acts::Vector3 t1 = h1.tangent(s1);
    const Acts::Vector3 t2 = h2.tangent(s2);
    const double a         = t1.dot(t1);
    const double b         = t1.dot(t2);
    const double c         = t2.dot(t2);
    const double d         = t1.dot(w);
    const double e         = t2.dot(w);
    const double det       = a * c - b * b;
    if (det < 1e-12 * a * c) {
      // (Nearly) parallel tangents: keep the current points
      break;
    }
    const double ds1 = (b * e - c * d) / det;
    const double ds2 = (a * e - b * d) / det;
    if (!std::isfinite(ds1) || !std::isfinite(ds2)) {
      break;
    }
    s1 += ds1;
    s2 += ds2;
    if (std::abs(ds1) < m_cfg.tolerance && std::abs(ds2) < m_cfg.tolerance) {
      break;
    }
  }

  Acts::Vector3 seed = 0.5 * (h1.position(s1) + h2.position(s2));
  if (!seed.allFinite()) {
    seed = 0.5 * (h1.origin + h2.origin);
  }
  return seed;
}

Acts::Result<std::vector<Acts::Vertex>>
TwoTrackVertexSeeder::find(const std::vector<Acts::InputTrack>& trackVector,
                           const Acts::VertexingOptions& vertexingOptions,
                           Acts::IVertexFinder::State& /*anyState*/) const {
  if (trackVector.size() != 2) {
    return std::vector<Acts::Vertex>{};
  }

  const auto& track1 = Acts::InputTrack::extractParameters(trackVector[0]);
  const auto& track2 = Acts::InputTrack::extractParameters(trackVector[1]);
  const auto& gctx   = vertexingOptions.geoContext.get();

  // Field at the midpoint of the perigees, assumed uniform along z
  auto cache              = m_cfg.bField->makeCache(vertexingOptions.magFieldContext);
  const Acts::Vector3 mid = 0.5 * (track1.position(gctx) + track2.position(gctx));
  auto field              = m_cfg.bField->getField(mid, cache);
  if (!field.ok()) {
    return field.error();
  }

  Acts::Vertex seed(closestApproach(track1, track2, gctx, (*field)[Acts::eZ]));
  seed.setFullCovariance(vertexingOptions.constraint.fullCovariance());
  return std::vector<Acts::Vertex>{seed};
}

Acts::IVertexFinder::State
TwoTrackVertexSeeder::makeState(const Acts::MagneticFieldContext& /*mctx*/) const {
  return Acts::IVertexFinder::State{State{}};
}

void TwoTrackVertexSeeder::setTracksToRemove(
    Acts::IVertexFinder::State& /*anyState*/,
    const std::vector<Acts::InputTrack>& /*removedTracks*/) const {
  // Nothing to do: the seed only depends on the tracks passed to find()
}

} // namespace eicrecon
