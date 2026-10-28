// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Alex Jentsch
//

#include "TransformLocalFFFrameToLabFrame.h"

#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <Math/GenVector/RotationY.h>
#include <Math/Vector3Dfwd.h>
#include <edm4hep/Vector3f.h>
#include <tuple>

void eicrecon::TransformLocalFFFrameToLabFrame::init() {}

void eicrecon::TransformLocalFFFrameToLabFrame::process(
    const TransformLocalFFFrameToLabFrame::Input& input,
    const TransformLocalFFFrameToLabFrame::Output& output) const {

  const auto [ff_particles_local_frame] = input;
  auto [ff_particles_lab_frame]         = output;

  //read reco tracks from RomanPots, OMD, etc. collections which are reconstructed in local frame centered on hadron beam
  //The output is the same reco information but rotated to ePIC lab frame to store into RectonstructedParticles

  double rotationAngleY = -0.025; //crossing angle rotation
  ROOT::Math::RotationY rotationAboutY(rotationAngleY);

  for (const auto& p : *ff_particles_local_frame) {

    auto tmp = p.clone();

    const auto& localFrameVector         = p.getMomentum();
    ROOT::Math::XYZVector labFrameVector = rotationAboutY(
        ROOT::Math::XYZVector(localFrameVector.x, localFrameVector.y, localFrameVector.z));

    tmp.setMomentum({static_cast<float>(labFrameVector.X()), static_cast<float>(labFrameVector.Y()),
                     static_cast<float>(labFrameVector.Z())});

    ff_particles_lab_frame->push_back(tmp);
  }
}
