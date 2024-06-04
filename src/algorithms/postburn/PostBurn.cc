// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Alex Jentsch, Jihee Kim, Brian Page
//

#include "PostBurn.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Objects.h>
#include <DD4hep/VolumeManager.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <gsl/pointers>
#include <vector>
#include <TVector3.h>
#include <TLorentzVector.h>

#include "algorithms/postburn/PostBurnConfig.h"

void eicrecon::PostBurn::init(std::shared_ptr<spdlog::logger> &logger) {
    m_log       = logger;
}

void eicrecon::PostBurn::process(
    const PostBurn::Input& input,
    const PostBurn::Output& output) const {

    const auto [mcparts, recparticles, recParticlesAssoc] = input;
    auto [outputParticles] = output;

    bool      pidAssumePionMass = m_cfg.pidAssumePionMass;
    double    crossingAngle    = m_cfg.crossingAngle;
    double    pidPurity        = m_cfg.pidPurity;
    bool      correctBeamFX    = m_cfg.correctBeamFX;
    bool      pidUseMCTruth    = m_cfg.pidUseMCTruth;

    bool      hasBeamHadron    = false;
    bool      hasBeamLepton    = false;
	
    //read MCParticles information for status == 1 particles and post-burn
	
    ROOT::Math::PxPyPzEVector  e_beam(0.,0.,0.,0.);
    ROOT::Math::PxPyPzEVector  h_beam(0.,0.,0.,0.);
  
    // First, extract beams, flag decides if using beam kinematics as-stored, or only the crossing-angle boost/rotation
    // This functionality is important for functionality for reconstructed tracks
    if(correctBeamFX == true){
        for (const auto& p: *mcparts) {
        
            if(p.getGeneratorStatus() == 4 && (p.getPDG() == 2212 || p.getPDG() == 2112)) { //look for "beam" proton/neutron
                h_beam.SetPxPyPzE(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
                hasBeamHadron = true;
            }
            if(p.getGeneratorStatus() == 4 && p.getPDG() == 11) { //look for "beam" electron
                e_beam.SetPxPyPzE(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
                hasBeamLepton = true;
            }
        }
    }
    else{
        for (const auto& p: *mcparts) {

            if(p.getGeneratorStatus() == 4 && (p.getPDG() == 2212 || p.getPDG() == 2112)) { //look for "beam" proton/neutron
                h_beam.SetPxPyPzE(crossingAngle*p.getEnergy(), 0.0, p.getEnergy(), p.getEnergy());
                hasBeamHadron = true;
            }
            if(p.getGeneratorStatus() == 4 && p.getPDG() == 11) { //look for "beam" electron
                e_beam.SetPxPyPzE(0.0, 0.0, -p.getEnergy(), p.getEnergy());
                hasBeamLepton = true;
            }
        }
    }

    //handling for FF particle gun input!!
    if(!hasBeamHadron || !hasBeamLepton){
        for (const auto& p: *mcparts) {
            if((p.getPDG() == 2212 || p.getPDG() == 2112)) { //look for "gun" proton/neutron
                h_beam.SetPxPyPzE(crossingAngle*p.getEnergy(), 0.0, p.getEnergy(), p.getEnergy());
                if(p.getEnergy() > 270.0 && p.getEnergy() < 280.0){
                    e_beam.SetPxPyPzE(0.0, 0.0, -18.0, 18.0);
                }
            }
        }
    }

    //Calculate boost vectors and rotations here
	
    ROOT::Math::PxPyPzEVector cm_frame_boost = e_beam + h_beam;
    ROOT::Math::PxPyPzEVector tmp(-cm_frame_boost.Px(), -cm_frame_boost.Py(), -cm_frame_boost.Pz(), cm_frame_boost.E());
		
    ROOT::Math::Boost boostVector(tmp.Px()/tmp.E(), tmp.Py()/tmp.E(), tmp.Pz()/tmp.E());
		
    //Boost to CM frame
    e_beam = boostVector(e_beam);
    h_beam = boostVector(h_beam);
		
    double rotationAngleY = -1.0*TMath::ATan2(h_beam.Px(), h_beam.Pz());
    double rotationAngleX = 1.0*TMath::ATan2(h_beam.Py(), h_beam.Pz());
		
    ROOT::Math::RotationY rotationAboutY(rotationAngleY);
    ROOT::Math::RotationX rotationAboutX(rotationAngleX);
	
    e_beam = rotationAboutY(e_beam);
    h_beam = rotationAboutY(h_beam);
    e_beam = rotationAboutX(e_beam);
    h_beam = rotationAboutX(h_beam);
	
    //Boost back to proper head-on frame
	
    ROOT::Math::PxPyPzEVector head_on_frame_boost(0., 0., cm_frame_boost.Pz(), cm_frame_boost.E());
    ROOT::Math::Boost headOnBoostVector(head_on_frame_boost.Px()/head_on_frame_boost.E(), head_on_frame_boost.Py()/head_on_frame_boost.E(), head_on_frame_boost.Pz()/head_on_frame_boost.E());
		
    e_beam = headOnBoostVector(e_beam);
    h_beam = headOnBoostVector(h_beam);

    //Now, loop through events and apply operations to final-state particles
  	for (const auto& p: *mcparts) {
        
  	    if(p.getGeneratorStatus() == 1) { //look for final-state particles
            ROOT::Math::PxPyPzEVector mc(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
				
            mc = boostVector(mc);
            mc = rotationAboutY(mc);
            mc = rotationAboutX(mc);
            mc = headOnBoostVector(mc);
												
            edm4hep::Vector3f mcMom(mc.Px(), mc.Py(), mc.Pz());				

            edm4hep::MutableMCParticle MCTrack;
            MCTrack.setMomentum(mcMom);
            MCTrack.setCharge(p.getCharge());
            MCTrack.setTime(p.getTime());
            MCTrack.setVertex(p.getVertex());
            MCTrack.setEndpoint(p.getEndpoint());
    			
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

}

