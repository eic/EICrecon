// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Alex Jentsch, Jihee Kim, Brian Page
//

#include "UndoAfterBurner.h"

#include <Math/GenVector/Boost.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/GenVector/RotationX.h>
#include <Math/GenVector/RotationY.h>
#include <Math/Vector4Dfwd.h>
#include <TMath.h>
#include <edm4hep/Vector3f.h>
#include <gsl/pointers>

#include "algorithms/reco/Beam.h"
#include "algorithms/reco/UndoAfterBurnerConfig.h"

void eicrecon::UndoAfterBurner::init() {

}

void eicrecon::UndoAfterBurner::process(
    const UndoAfterBurner::Input& input,
    const UndoAfterBurner::Output& output) const {

    const auto [mcparts] = input;
    auto [outputParticles] = output;

    bool      pidAssumePionMass = m_cfg.m_pid_assume_pion_mass;
    double    crossingAngle    = m_cfg.m_crossing_angle;
    double    pidPurity        = m_cfg.m_pid_purity;
    bool      correctBeamFX    = m_cfg.m_correct_beam_FX;
    bool      pidUseMCTruth    = m_cfg.m_pid_use_MC_truth;

    bool      hasBeamHadron    = true;
    bool      hasBeamLepton    = true;

    //read MCParticles information and "postburn" to remove the afterburner effects.
    //The output is then the original MC input produced by the generator.

    ROOT::Math::PxPyPzEVector  e_beam(0.,0.,0.,0.);
    ROOT::Math::PxPyPzEVector  h_beam(0.,0.,0.,0.);

    auto incoming_lepton = find_first_beam_electron(mcparts);
    if (incoming_lepton.size() == 0) {
        debug("No beam electron found -- particleGun input");
        hasBeamLepton = false;
    }

    auto incoming_hadron = find_first_beam_hadron(mcparts);
    if (incoming_hadron.size() == 0) {
        debug("No beam hadron found -- particleGun input");
        hasBeamHadron = false;
    }

    if((hasBeamHadron && !hasBeamLepton) || (!hasBeamHadron && hasBeamLepton)){
        debug("Only one beam defined! Not a possible configuration!");
        return;
    }

    // Handling for FF particle gun input!!
    if (!hasBeamHadron || !hasBeamLepton) {
        for (const auto& p: *mcparts) {
            if((p.getPDG() == 2212 || p.getPDG() == 2112)) { //look for "gun" proton/neutron
                hasBeamHadron = true;
                h_beam.SetPxPyPzE(crossingAngle*p.getEnergy(), 0.0, p.getEnergy(), p.getEnergy());
                if(p.getEnergy() > 270.0 && p.getEnergy() < 280.0){
                    hasBeamLepton = true;
                    e_beam.SetPxPyPzE(0.0, 0.0, -18.0, 18.0);
                }
            }
        }

    } else {

        if (correctBeamFX) {

            h_beam.SetPxPyPzE(incoming_hadron[0].getMomentum().x, incoming_hadron[0].getMomentum().y, incoming_hadron[0].getMomentum().z, incoming_hadron[0].getEnergy());
            e_beam.SetPxPyPzE(incoming_lepton[0].getMomentum().x, incoming_lepton[0].getMomentum().y, incoming_lepton[0].getMomentum().z, incoming_lepton[0].getEnergy());

        } else {

            h_beam.SetPxPyPzE(crossingAngle*incoming_hadron[0].getEnergy(), 0.0, incoming_hadron[0].getEnergy(), incoming_hadron[0].getEnergy());
            e_beam.SetPxPyPzE(0.0, 0.0, -incoming_lepton[0].getEnergy(), incoming_lepton[0].getEnergy());

        }
    }

    // Bail out if still no beam particles, since this leads to division by zero
    if (!hasBeamHadron && !hasBeamLepton) {
      return;
    }

    // Calculate boost vectors and rotations here
    ROOT::Math::PxPyPzEVector cm_frame_boost = e_beam + h_beam;
    ROOT::Math::Cartesian3D beta(-cm_frame_boost.Px() / cm_frame_boost.E(), -cm_frame_boost.Py() / cm_frame_boost.E(), -cm_frame_boost.Pz() / cm_frame_boost.E());
    ROOT::Math::Boost boostVector(beta);

    // Boost to CM frame
    e_beam = boostVector(e_beam);
    h_beam = boostVector(h_beam);

    double rotationAngleY = -1.0*TMath::ATan2(h_beam.Px(), h_beam.Pz());
    double rotationAngleX = 1.0*TMath::ATan2(h_beam.Py(), h_beam.Pz());

    ROOT::Math::RotationY rotationAboutY(rotationAngleY);
    ROOT::Math::RotationX rotationAboutX(rotationAngleX);

    // Boost back to proper head-on frame
    ROOT::Math::PxPyPzEVector head_on_frame_boost(0., 0., cm_frame_boost.Pz(), cm_frame_boost.E());
    ROOT::Math::Boost headOnBoostVector(head_on_frame_boost.Px()/head_on_frame_boost.E(), head_on_frame_boost.Py()/head_on_frame_boost.E(), head_on_frame_boost.Pz()/head_on_frame_boost.E());

    // Now, loop through events and apply operations to the MCparticles
    for (const auto& p: *mcparts) {

        ROOT::Math::PxPyPzEVector mc(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());

        mc = boostVector(mc);
        mc = rotationAboutY(mc);
        mc = rotationAboutX(mc);
        mc = headOnBoostVector(mc);

        edm4hep::Vector3f mcMom(mc.Px(), mc.Py(), mc.Pz());
        edm4hep::MutableMCParticle MCTrack(p.clone());
        MCTrack.setMomentum(mcMom);

        if(pidUseMCTruth){
            MCTrack.setPDG(p.getPDG());
            MCTrack.setMass(p.getMass());
        }
        if(!pidUseMCTruth && pidAssumePionMass){
            MCTrack.setPDG(211);
            MCTrack.setMass(0.13957);
        }

        outputParticles->push_back(MCTrack);
    }

}
