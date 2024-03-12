// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 John Lajoie

// class definition
#include "TransformBreitFrame.h"

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/GenVector/LorentzRotation.h>
#include <Math/GenVector/Rotation3D.h>
#include <Math/GenVector/Boost.h>
#include <edm4hep/utils/vector_utils.h>

// for error handling
#include <JANA/JException.h>
#include <fmt/core.h>
#include <stdexcept>
#include <vector>

#include "Beam.h"

namespace eicrecon {

  void TransformBreitFrame::init(std::shared_ptr<spdlog::logger> logger) {

    m_log = logger;
    m_log->trace("Initialized");

  }  // end 'init(std::shared_ptr<spdlog::logger>)'


  std::unique_ptr<edm4eic::ReconstructedParticleCollection> TransformBreitFrame::process(const edm4hep::MCParticleCollection *mcpart,
                                                                                         const edm4eic::InclusiveKinematicsCollection *kine,
                                                                                         const edm4eic::ReconstructedParticleCollection *lab_collection) {
    // Store the transformed particles
    std::unique_ptr<edm4eic::ReconstructedParticleCollection> breit_collection { std::make_unique<edm4eic::ReconstructedParticleCollection>() };

    // Beam momenta extracted from MCParticle
    // This is the only place truth information is used!

    // Get incoming electron beam
    const auto ei_coll = find_first_beam_electron(mcpart);
    if (ei_coll.size() == 0) {
      m_log->debug("No beam electron found");
      return breit_collection;
    }
    const PxPyPzEVector e_initial(
      round_beam_four_momentum(
        ei_coll[0].getMomentum(),
        m_electron,
        {-5.0, -10.0, -18.0},
        0.0)
      );

    // Get incoming hadron beam
    const auto pi_coll = find_first_beam_hadron(mcpart);
    if (pi_coll.size() == 0) {
      m_log->debug("No beam hadron found");
      return breit_collection;
    }
    const PxPyPzEVector p_initial(
      round_beam_four_momentum(
        pi_coll[0].getMomentum(),
        pi_coll[0].getPDG() == 2212 ? m_proton : m_neutron,
        {41.0, 100.0, 275.0},
        m_crossingAngle)
      );

    m_log->debug("electron energy, proton energy = {},{}",e_initial.E(),p_initial.E());

    // Get the event kinematics, set up transform
    if (kine->size() == 0) {
      m_log->debug("No kinematics found");
      return breit_collection;
    }

    const auto& evt_kin = kine->at(0);

    const auto meas_x = evt_kin.getX();
    const auto meas_Q2 = evt_kin.getQ2();

    // Use relation to get reconstructed scattered electron
    const auto ef_r = evt_kin.getScat();
    const PxPyPzEVector e_final(ef_r.getMomentum().x, ef_r.getMomentum().y, ef_r.getMomentum().z, ef_r.getEnergy());

    // Set up the transformation
    const PxPyPzEVector virtual_photon = (e_initial - e_final);

    m_log->debug("x, Q^2 = {},{}",meas_x,meas_Q2);

    // Set up the transformation (boost) to the Breit frame
    const auto P3 = p_initial.Vect();
    const auto q3 = virtual_photon.Vect();
    const ROOT::Math::Boost boost(-(2.0*meas_x*P3 + q3)*(1.0/(2.0*meas_x*p_initial.E() + virtual_photon.E())));
    const auto breit = ROOT::Math::LorentzRotation(boost);

    PxPyPzEVector p_initial_breit = (breit * p_initial);
    PxPyPzEVector e_initial_breit = (breit * e_initial);
    PxPyPzEVector e_final_breit = (breit * e_final);
    PxPyPzEVector virtual_photon_breit = (breit * virtual_photon);

    // Now rotate so the virtual photon momentum is all along the negative z-axis
    const auto zhat =  -virtual_photon_breit.Vect().Unit();
    const auto yhat = (e_initial_breit.Vect().Cross(e_final_breit.Vect())).Unit();
    const auto xhat = yhat.Cross(zhat);

    const ROOT::Math::Rotation3D breitRotInv(xhat,yhat,zhat);
    const ROOT::Math::Rotation3D breitRot = breitRotInv.Inverse();

    // Diagnostics
    p_initial_breit = breitRot*p_initial_breit;
    e_initial_breit = breitRot*e_initial_breit;
    e_final_breit = breitRot*e_final_breit;
    virtual_photon_breit = breitRot*virtual_photon_breit;

    m_log->debug("incoming hadron in Breit frame px,py,pz,E = {},{},{},{}",
                 p_initial_breit.Px(),p_initial_breit.Py(),p_initial_breit.Pz(),p_initial_breit.E());
    m_log->debug("virtual photon in Breit frame px,py,pz,E = {},{},{},{}",
                 virtual_photon_breit.Px(),virtual_photon_breit.Py(),virtual_photon_breit.Pz(),virtual_photon_breit.E());

    // look over the input particles and transform
    for (unsigned iInput = 0; const auto& lab : *lab_collection) {

      // Transform to Breit frame
      PxPyPzEVector track(lab.getMomentum().x,lab.getMomentum().y,lab.getMomentum().z,lab.getEnergy());
      PxPyPzEVector breit_track = breitRot*(breit*track);

      // create particle to store in output collection
      edm4eic::MutableReconstructedParticle breit = breit_collection->create();
      breit.setMomentum(edm4hep::Vector3f(breit_track.Px(), breit_track.Py(), breit_track.Pz()));
      breit.setEnergy(breit_track.E());

      // Copy the rest of the particle information
      breit.setType(lab.getType());
      breit.setReferencePoint(lab.getReferencePoint());
      breit.setCharge(lab.getCharge());
      breit.setMass(lab.getMass());
      breit.setGoodnessOfPID(lab.getGoodnessOfPID());
      breit.setCovMatrix(lab.getCovMatrix());
      breit.setPDG(lab.getPDG());
      breit.setStartVertex(lab.getStartVertex());
      breit.setParticleIDUsed(lab.getParticleIDUsed());

      // set up a relation between the lab and Breit frame representations
      breit.addToParticles( lab );

    }

    // return the transfromed particles
    return breit_collection;

  }  // end 'process'


}  // end namespace eicrecon
