// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Dmitry Kalinkin

#pragma once

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#if Acts_VERSION_MAJOR >= 46
#include <Acts/EventData/BoundTrackParameters.hpp>
#else
#include <Acts/EventData/TrackParameters.hpp>
#endif
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/Vertexing/IVertexFinder.hpp>
#include <memory>

namespace eicrecon {

/// Vertex seeder for a pair of tracks: returns the midpoint of the points of
/// closest approach of the two track helices. Unlike the Acts density seeders,
/// which only scan in z along the beam line, the seed is placed in 3D, so it
/// is suited for displaced (secondary) vertices. Returns no seed unless
/// exactly two tracks are given.
class TwoTrackVertexSeeder final : public Acts::IVertexFinder {
public:
  struct Config {
    std::shared_ptr<const Acts::MagneticFieldProvider> bField;
    /// Maximum number of Gauss-Newton iterations
    unsigned int maxIterations = 20;
    /// Stop iterating once both path length updates are below this value
    double tolerance = 1e-4 * Acts::UnitConstants::mm;
  };

  struct State {};

  explicit TwoTrackVertexSeeder(const Config& cfg) : m_cfg(cfg) {}

  Acts::Result<std::vector<Acts::Vertex>> find(const std::vector<Acts::InputTrack>& trackVector,
                                               const Acts::VertexingOptions& vertexingOptions,
                                               Acts::IVertexFinder::State& anyState) const final;

  Acts::IVertexFinder::State makeState(const Acts::MagneticFieldContext& mctx) const final;

  void setTracksToRemove(Acts::IVertexFinder::State& anyState,
                         const std::vector<Acts::InputTrack>& removedTracks) const final;

  /// Midpoint of the points of closest approach of the helices of two tracks
  /// in a uniform field along z
  Acts::Vector3 closestApproach(const Acts::BoundTrackParameters& track1,
                                const Acts::BoundTrackParameters& track2,
                                const Acts::GeometryContext& gctx, double bz) const;

private:
  Config m_cfg;
};

} // namespace eicrecon
