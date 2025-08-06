// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 John Lajoie

// class definition
#include "TransformBreitFrame.h"

#include <Math/GenVector/Boost.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/GenVector/Rotation3D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/Cov4f.h>
#include <edm4eic/Vertex.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/kinematics.h>
#include <fmt/core.h>
#include <gsl/pointers>
#include <vector>

#include "Beam.h"

namespace eicrecon {

void TransformBreitFrame::process(const TransformBreitFrame::Input& input,
                                  const TransformBreitFrame::Output& output) const {
  // Grab input collections
  const auto [mcpart, kine, lab_collection] = input;
  auto [breit_collection]                   = output;

  // Beam momenta extracted from MCParticle
  // This is the only place truth information is used!

  // Get incoming electron beam
  const auto ei_coll = find_first_beam_electron(mcpart);
  if (ei_coll.empty()) {
    debug("No beam electron found");
    return;
  }
  const PxPyPzEVector e_initial(round_beam_four_momentum(
      ei_coll[0].getMomentum(), m_particleSvc.particle(ei_coll[0].getPDG()).mass,
      {-5.0, -10.0, -18.0}, 0.0));

  // Get incoming hadron beam
  const auto pi_coll = find_first_beam_hadron(mcpart);
  if (pi_coll.empty()) {
    debug("No beam hadron found");
    return;
  }
  const PxPyPzEVector p_initial(round_beam_four_momentum(
      pi_coll[0].getMomentum(), m_particleSvc.particle(pi_coll[0].getPDG()).mass,
      {41.0, 100.0, 275.0}, m_crossingAngle));

  debug("electron energy, proton energy = {},{}", e_initial.E(), p_initial.E());

  // Get the event kinematics, set up transform
  if (kine->empty()) {
    debug("No kinematics found");
    return;
  }

  const auto& evt_kin = kine->at(0);

  const auto meas_x  = evt_kin.getX();
  const auto meas_Q2 = evt_kin.getQ2();

  // Use relation to get reconstructed scattered electron
  const PxPyPzEVector e_final =
      edm4hep::utils::detail::p4(evt_kin.getScat(), &edm4hep::utils::UseEnergy);
  debug("scattered electron in lab frame px,py,pz,E = {},{},{},{}", e_final.Px(), e_final.Py(),
        e_final.Pz(), e_final.E());

  // Set up the transformation
  const PxPyPzEVector virtual_photon = (e_initial - e_final);
  debug("virtual photon in lab frame px,py,pz,E = {},{},{},{}", virtual_photon.Px(),
        virtual_photon.Py(), virtual_photon.Pz(), virtual_photon.E());

  debug("x, Q^2 = {},{}", meas_x, meas_Q2);

  // Set up the transformation (boost) to the Breit frame
  const auto P3 = p_initial.Vect();
  const auto q3 = virtual_photon.Vect();
  const ROOT::Math::Boost breit(-(2.0 * meas_x * P3 + q3) *
                                (1.0 / (2.0 * meas_x * p_initial.E() + virtual_photon.E())));

  PxPyPzEVector p_initial_breit      = (breit * p_initial);
  PxPyPzEVector e_initial_breit      = (breit * e_initial);
  PxPyPzEVector e_final_breit        = (breit * e_final);
  PxPyPzEVector virtual_photon_breit = (breit * virtual_photon);

  // Now rotate so the virtual photon momentum is all along the negative z-axis
  const auto zhat = -virtual_photon_breit.Vect().Unit();
  const auto yhat = (e_initial_breit.Vect().Cross(e_final_breit.Vect())).Unit();
  const auto xhat = yhat.Cross(zhat);

  const ROOT::Math::Rotation3D breitRotInv(xhat, yhat, zhat);
  const ROOT::Math::Rotation3D breitRot = breitRotInv.Inverse();

  // Diagnostics
  p_initial_breit      = breitRot * p_initial_breit;
  e_initial_breit      = breitRot * e_initial_breit;
  e_final_breit        = breitRot * e_final_breit;
  virtual_photon_breit = breitRot * virtual_photon_breit;

  debug("incoming hadron in Breit frame px,py,pz,E = {},{},{},{}", p_initial_breit.Px(),
        p_initial_breit.Py(), p_initial_breit.Pz(), p_initial_breit.E());
  debug("virtual photon in Breit frame px,py,pz,E = {},{},{},{}", virtual_photon_breit.Px(),
        virtual_photon_breit.Py(), virtual_photon_breit.Pz(), virtual_photon_breit.E());

  // look over the input particles and transform
  for (const auto& lab : *lab_collection) {

    // Transform to Breit frame
    PxPyPzEVector lab_particle(lab.getMomentum().x, lab.getMomentum().y, lab.getMomentum().z,
                               lab.getEnergy());
    PxPyPzEVector breit_particle = breitRot * (breit * lab_particle);

    // create particle to store in output collection
    auto breit_out = breit_collection->create();
    breit_out.setMomentum(
        edm4hep::Vector3f(breit_particle.Px(), breit_particle.Py(), breit_particle.Pz()));
    breit_out.setEnergy(breit_particle.E());

    // Copy the rest of the particle information
    breit_out.setType(lab.getType());
    breit_out.setReferencePoint(lab.getReferencePoint());
    breit_out.setCharge(lab.getCharge());
    breit_out.setMass(lab.getMass());
    breit_out.setGoodnessOfPID(lab.getGoodnessOfPID());
    breit_out.setCovMatrix(lab.getCovMatrix());
    breit_out.setPDG(lab.getPDG());
    breit_out.setStartVertex(lab.getStartVertex());
    breit_out.setParticleIDUsed(lab.getParticleIDUsed());

    // set up a relation between the lab and Breit frame representations
    breit_out.addToParticles(lab);
  }

} // end 'process'

} // end namespace eicrecon
