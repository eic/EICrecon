// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023-2025, Simon Gardner

#include <edm4eic/Cov6f.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/vector_utils.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <Eigen/LU>
#include <cstdint>
#include <gsl/pointers>
#include <vector>

#include "algorithms/fardetectors/FarDetectorLinearProjection.h"
#include "algorithms/fardetectors/FarDetectorLinearProjectionConfig.h"

namespace eicrecon {

void FarDetectorLinearProjection::init() {

  // plane position
  m_plane_position << m_cfg.plane_position[0], m_cfg.plane_position[1], m_cfg.plane_position[2];
  m_directions.block<3, 1>(0, 0) << m_cfg.plane_a[0], m_cfg.plane_a[1], m_cfg.plane_a[2];
  m_directions.block<3, 1>(0, 1) << m_cfg.plane_b[0], m_cfg.plane_b[1], m_cfg.plane_b[2];
}

void FarDetectorLinearProjection::process(const FarDetectorLinearProjection::Input& input,
                                          const FarDetectorLinearProjection::Output& output) const {

  const auto [inputTracks] = input;
  auto [outputTracks]      = output;

  Eigen::Matrix3d directions = m_directions;

  for (const auto& track : *inputTracks) {

    Eigen::Vector3d point_position(track.getPosition().x, track.getPosition().y,
                                   track.getPosition().z);
    Eigen::Vector3d positionDiff = point_position - m_plane_position;

    // Convert spherical coordinates to Cartesian
    double x = track.getMomentum().x;
    double y = track.getMomentum().y;
    double z = track.getMomentum().z;
    directions.block<3, 1>(0, 2) << x, y, z;

    auto projectedPoint = directions.inverse() * positionDiff;

    // Create track parameters edm4eic structure
    // TODO - populate more of the fields correctly
    std::int32_t type = 0;
    // Surface ID not used in this context
    std::uint64_t surface = 0;
    // Plane Point
    edm4hep::Vector2f loc(projectedPoint[0], projectedPoint[1]); //Temp unit transform
    float theta     = edm4eic::anglePolar(track.getMomentum());
    float phi       = edm4eic::angleAzimuthal(track.getMomentum());
    float qOverP    = 0.;
    float time      = 0;
    int32_t pdgCode = 11;
    // Point Error
    edm4eic::Cov6f error;

    debug("Position:      a={},   b={}", loc.a, loc.b);
    debug("Direction: theta={}, phi={}", theta, phi);

    outputTracks->create(type, surface, loc, theta, phi, qOverP, time, pdgCode, error);
  }
}

} // namespace eicrecon
