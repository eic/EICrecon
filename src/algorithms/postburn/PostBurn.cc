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


  	//----- Define constants here ------

  	//GET CROSSING ANGLE INFORMATION HERE FROM HEADER!!!!


	bool      pidAssumePionMass = m_cfg.pidAssumePionMass;
    double    crossingAngle    = m_cfg.crossingAngle;
    double    pidPurity        = m_cfg.pidPurity;
	bool      correctBeamFX    = m_cfg.correctBeamFX;
    bool      pidUseMCTruth    = m_cfg.pidUseMCTruth;

	
  	//read MCParticles information for status == 1 particles and post-burn
  
  	TLorentzVector e_beam(0.,0.,0.,0.);
  	TLorentzVector h_beam(0.,0.,0.,0.);
  
    //First, extract beams -- need to add conditional flags after
  	if(correctBeamFX == true){
		for (const auto& p: *mcparts) {
        
  		  	if(p.getGeneratorStatus() == 4 && (p.getPDG() == 2212 || p.getPDG() == 2112)) { //look for "beam" proton
                h_beam.SetPxPyPzE(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
  		  	}
  		  	if(p.getGeneratorStatus() == 4 && p.getPDG() == 11) { //look for "beam" electron
                e_beam.SetPxPyPzE(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
  		  	}
		 }
	}
	else{
		for (const auto& p: *mcparts) {

            if(p.getGeneratorStatus() == 4 && (p.getPDG() == 2212 || p.getPDG() == 2112)) { //look for "beam" proton
                
				h_beam.SetPxPyPzE(crossingAngle*p.getEnergy(), 0.0, p.getEnergy(), p.getEnergy());
            }
            if(p.getGeneratorStatus() == 4 && p.getPDG() == 11) { //look for "beam" electron
                e_beam.SetPxPyPzE(0.0, 0.0, -p.getEnergy(), p.getEnergy());
            }
         }
	}

	//Calculate boost vectors and rotations here

	TLorentzVector cm_frame_boost = e_beam + h_beam;
	TLorentzVector tmp(-cm_frame_boost[0], -cm_frame_boost[1], -cm_frame_boost[2], cm_frame_boost[3]);
	
	TVector3 boostVector(0.,0.,0.);
	boostVector = tmp.BoostVector();
	
	
	//Boost to CM frame
	e_beam.Boost(boostVector);
	h_beam.Boost(boostVector);

	//perform rotations - need to have flag here to decide if only crossing angle or all beam FX
	double rotationAboutY = -1.0*TMath::ATan2(h_beam.Px(), h_beam.Pz());
	double rotationAboutX = 1.0*TMath::ATan2(h_beam.Py(), h_beam.Pz());
	
	e_beam.RotateY(rotationAboutY);
	h_beam.RotateY(rotationAboutY);
	e_beam.RotateX(rotationAboutX);
	h_beam.RotateX(rotationAboutX);
	
	//Boost back to proper head-on frame
	
	TLorentzVector head_on_frame_boost(0., 0., cm_frame_boost[2], cm_frame_boost[3]);
	TVector3 headOnBoostVector(0.,0.,0.);
	headOnBoostVector = head_on_frame_boost.BoostVector();
	
	e_beam.Boost(headOnBoostVector);
	h_beam.Boost(headOnBoostVector);

	int pdgCode = 0;

    //Now, loop through events and apply operations to final-state particles
  	for (const auto& p: *mcparts) {
        
  		  if(p.getGeneratorStatus() == 1) { //look for "beam" proton
                TLorentzVector mc(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
				mc.Boost(boostVector);
				mc.RotateY(rotationAboutY);
				mc.RotateX(rotationAboutX);
				mc.Boost(headOnBoostVector);

				edm4hep::Vector3f prec = {static_cast<float>(mc.Px()), static_cast<float>(mc.Py()),
					                static_cast<float>(mc.Pz())};

				edm4hep::MutableMCParticle reconTrack;
				//auto reconTrack = mcparts->create();
    			//reconTrack.setType(0);
    			reconTrack.setMomentum(prec);
    			//reconTrack.setEnergy(std::hypot(edm4hep::utils::magnitude(reconTrack.getMomentum()), m_cfg.partMass));
    			//reconTrack.setEnergy(mc.E());
				//reconTrack.setReferencePoint(refPoint);
    			reconTrack.setCharge(p.getCharge());
    			//reconTrack.setGoodnessOfPID(1.);
    			if(pidUseMCTruth){ 
					reconTrack.setPDG(p.getPDG()); 
					reconTrack.setMass(p.getMass());
				}
				if(!pidUseMCTruth && pidAssumePionMass){ 
					reconTrack.setPDG(211);
					reconTrack.setMass(0.13957);
				}
    			//reconTrack.covMatrix(); // @TODO: Errors
    			outputParticles->push_back(reconTrack);

			}
  	}


}

