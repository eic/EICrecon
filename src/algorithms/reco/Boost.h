// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck, Barak Schmookler, 2026 Stephen Maple

#pragma once

#include <Math/Vector4D.h>
#include <Math/LorentzRotation.h>
#include <Math/LorentzVector.h>
#include <Math/RotationX.h>
#include <Math/RotationY.h>
#include <Math/Boost.h>

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

using ROOT::Math::LorentzRotation;

inline LorentzRotation determine_boost(PxPyPzEVector ei, PxPyPzEVector pi) {

  using ROOT::Math::Boost;
  using ROOT::Math::RotationX;
  using ROOT::Math::RotationY;

  // Save original input for constructing target "head-on" boost
  PxPyPzEVector eo = ei;
  PxPyPzEVector po = pi;

  // Step 1: Boost to CM frame
  const auto cmBoost = (ei + pi).BoostToCM();
  const Boost boost_to_cm(cmBoost);

  ei = boost_to_cm(ei);
  pi = boost_to_cm(pi);

  // Step 2: Rotate so pi is aligned along +z
  RotationY rotAboutY(-1.0 * atan2(pi.Px(), pi.Pz()));
  RotationX rotAboutX(+1.0 * atan2(pi.Py(), pi.Pz()));

  ei = rotAboutX(rotAboutY(ei));
  pi = rotAboutX(rotAboutY(pi));

  // Step 3: Construct "ideal" head-on configuration (same energies, back-to-back along z)
  double e_energy = eo.E();
  double p_energy = po.E();
  double e_pz = -sqrt(e_energy*e_energy - eo.M()*eo.M());
  double p_pz = +sqrt(p_energy*p_energy - po.M()*po.M());

  PxPyPzEVector eh(0, 0, e_pz, e_energy);
  PxPyPzEVector ph(0, 0, p_pz, p_energy);

  // Step 4: Boost to the frame where those particles look like the current ei/pi
  const auto hoBoost = (eh + ph).BoostToCM();  // CM of head-on frame
  const Boost boost_to_headon(-hoBoost);       // Boost from CM to head-on

  // Final transformation = headon boost * rotations * CM boost
  LorentzRotation tf;
  tf *= boost_to_headon;
  tf *= rotAboutX;
  tf *= rotAboutY;
  tf *= boost_to_cm;

  return tf;
}


inline PxPyPzEVector apply_boost(const LorentzRotation& tf, PxPyPzEVector part) {

  // Step 5: Apply boosts and rotations to any particle 4-vector
  // (here too, choices will have to be made as to what the 4-vector is for reconstructed particles)

  // Boost and rotate particle 4-momenta into the headon frame
  return tf(part);
}

} // namespace eicrecon
