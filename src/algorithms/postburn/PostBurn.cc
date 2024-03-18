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

#include "algorithms/fardetectors/MatrixTransferStaticConfig.h"

void eicrecon::MatrixTransferStatic::init(const dd4hep::Detector* det,
                                          std::shared_ptr<spdlog::logger> &logger) {

  m_log       = logger;
  m_detector  = det;
  //Calculate inverse of static transfer matrix

}

void eicrecon::PostBurn::process(
    const PostBurn::Input& input,
    const PostBurn::Output& output) const {

  	const auto [mcparts, rechits] = input;
  	auto [outputParticles] = output;


  	//----- Define constants here ------

  	//GET CROSSING ANGLE INFORMATION HERE FROM HEADER!!!!

	
  	//read MCParticles information for status == 1 particles and post-burn
  
  	TLorentzVector e_beam(0.,0.,0.,0.);
  	TLorentzVector h_beam(0.,0.,0.,0.);
  
    //First, extract beams -- need to add conditional flags after
  	for (const auto& p: *mcparts) {
        
  		  if(p.getGeneratorStatus() == 4 && p.getPDG() == 2212) { //look for "beam" proton
                h_beam.SetPxPyPzE(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
  		  }
  		  if(p.getGeneratorStatus() == 4 && p.getPDG() == 11) { //look for "beam" electron
                e_beam.SetPxPyPzE(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
  		  }
		  
  	}

	//Calculate boost vectors and rotations here

	TLorentzVector cm_frame_boost = e_beam + h_beam;
	TLorentzVector tmp(-cm_frame_boost[0], -cm_frame_boost[1], -cm_frame_boost[2], cm_frame_boost[3],)
	
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
	TVector headOnBoostVector(0.,0.,0.);
	headOnBoostVector = head_on_frame_boost.BoostVector();
	
	e_beam.Boost(headOnBoostVector);
	h_beam.Boost(headOnBoostVector);

    //Now, loop through events and apply operations to final-state particles
  	for (const auto& p: *mcparts) {
        
  		  if(p.getGeneratorStatus() == 1) { //look for "beam" proton
                TLorentzVector mc(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
				mc.Boost(boostVector);
				mc.RotateY(rotationAboutY);
				mc.RotateX(rotationAboutX);
				mc.Boost(headOnBoostVector);
  		  }
  		  
		  
  	}

    edm4hep::Vector3f prec = {static_cast<float>(p * rsx / norm), static_cast<float>(p * rsy / norm),
                                static_cast<float>(p / norm)};

    //----- end reconstruction code ------

    edm4eic::MutableReconstructedParticle reconTrack;
    reconTrack.setType(0);
    reconTrack.setMomentum(prec);
    reconTrack.setEnergy(std::hypot(edm4hep::utils::magnitude(reconTrack.getMomentum()), m_cfg.partMass));
    reconTrack.setReferencePoint(refPoint);
    reconTrack.setCharge(m_cfg.partCharge);
    reconTrack.setMass(m_cfg.partMass);
    reconTrack.setGoodnessOfPID(1.);
    reconTrack.setPDG(m_cfg.partPDG);
    //reconTrack.covMatrix(); // @TODO: Errors
    outputParticles->push_back(reconTrack);

}
