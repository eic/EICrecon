// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Alex Jentsch

#include "TransformLocalFFFrameToLabFrame.h"

#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <Math/GenVector/RotationY.h>
#include <Math/Vector3Dfwd.h>
#include <edm4hep/Vector3f.h>
#include <tuple>

#include "TransformLocalFFFrameToLabFrameConfig.h"

/*! Read in a collection of reconstructed particles in FF
 *  local frame and rotate them by a configurable amount
 *  in Y. Output particles contain same information as inputs,
 *  but with rotated momenta and relation back to unrotated
 *  particle.
 *
 *  This is to ensure that all particles stored in ReconstructedParticles
 *  share a common reference frame.
 */

void eicrecon::TransformLocalFFFrameToLabFrame::process(
    const TransformLocalFFFrameToLabFrame::Input& input,
    const TransformLocalFFFrameToLabFrame::Output& output) const {

  const auto [ff_particles_local_frame] = input;
  auto [ff_particles_lab_frame]         = output;


  double rotationAngleY = m_cfg.m_crossing_angle; //crossing angle rotation
  ROOT::Math::RotationY rotationAboutY(rotationAngleY);

  for (const auto& localParticle : *ff_particles_local_frame) {

    auto labParticle = localParticle.clone();

    const auto& localFrameVector         = localParticle.getMomentum();
    ROOT::Math::XYZVector labFrameVector = rotationAboutY(
        ROOT::Math::XYZVector(localFrameVector.x, localFrameVector.y, localFrameVector.z));

    labParticle.setMomentum({static_cast<float>(labFrameVector.X()),
                             static_cast<float>(labFrameVector.Y()),
                             static_cast<float>(labFrameVector.Z())});

    labParticle.addToParticles(localParticle);
    ff_particles_lab_frame->push_back(labParticle);
  }
}
