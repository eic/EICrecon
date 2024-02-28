// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Alex Jentsch, Wouter Deconinck, Sylvester Joosten, David Lawrence, Simon Gardner
//
// This converted from: https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/FarForwardParticles.cpp

#include "MatrixTransferStatic.h"

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
                                          const dd4hep::rec::CellIDPositionConverter* id_conv,
                                          std::shared_ptr<spdlog::logger> &logger) {

  m_log       = logger;
  m_detector  = det;
  m_converter = id_conv;
  //Calculate inverse of static transfer matrix
    
}

void eicrecon::MatrixTransferStatic::process(
    const MatrixTransferStatic::Input& input,
    const MatrixTransferStatic::Output& output) const {

  const auto [mcparts, rechits] = input;
  auto [outputParticles] = output;
  
  std::vector<std::vector<double>> aX(m_cfg.aX);
  std::vector<std::vector<double>> aY(m_cfg.aY);
  
  //----- Define constants here ------
  double aXinv[2][2] = {{0.0, 0.0},
                        {0.0, 0.0}};
  double aYinv[2][2] = {{0.0, 0.0},
                        {0.0, 0.0}};
  
  double nomMomentum     = m_cfg.nomMomentum; //extract the nominal value first -- will be overwritten by MCParticle
  double local_x_offset  = m_cfg.local_x_offset;
  double local_y_offset  = m_cfg.local_y_offset;
  double local_x_slope_offset  = m_cfg.local_x_slope_offset;
  double local_y_slope_offset  = m_cfg.local_y_slope_offset;
 
  double numBeamProtons = 0;
  double runningMomentum = 0.0;
  
  for (const auto& p: *mcparts) {
  	if (p.getGeneratorStatus() == 4 && p.getPDG() == 2212) { //look for "beam" proton
		runningMomentum += p.getMomentum().z;
		numBeamProtons++;
	}
  }
  
  if(numBeamProtons == 0) {std::cout << "No beam protons to choose matrix!! Skipping!!" << std::endl; return;}
      
  nomMomentum = runningMomentum/numBeamProtons;
  
  std::cout << "average momentum for event sample = " << nomMomentum << std::endl;
 
  double nomMomentumError = 0.02;
  
 
  //This is a temporary solution to get the beam energy information
  //needed to select the correct matrix
  
  if(abs(275.0 - nomMomentum)/275.0 < nomMomentumError){

      aX[0][0] = 2.09948716; //a
      aX[0][1] = 29.17331284; //b
      aX[1][0] = 0.18560183; //c
      aX[1][1] = 0.19764317; //d

      aY[0][0] = 0.3457400000; //a
      aY[0][1] = 3.94047233; //b
      aY[1][0] = 0.0203150000; //c
      aY[1][1] = -0.140321368; //d

      local_x_offset       = -0.00041284;
      local_y_offset       = -0.00281233;
      local_x_slope_offset = 0.00050683;
      local_y_slope_offset = -0.001418633;

  }
  else if(abs(100.0 - nomMomentum)/100.0 < nomMomentumError){

      aX[0][0] = 2.03459216; //a
      aX[0][1] = 22.85780784; //b
      aX[1][0] = 0.179641961; //c
      aX[1][1] = -0.306626961; //d

      aY[0][0] = 0.3887900000; //a
      aY[0][1] = 3.71612646; //b
      aY[1][0] = 0.0226850000; //c
      aY[1][1] = -0.083092151; //d

      local_x_offset       = 0.00979216;
      local_y_offset       = -0.00778646;
      local_x_slope_offset = 0.004526961;
      local_y_slope_offset = -0.003907849;

  }
  else if(abs(41.0 - nomMomentum)/41.0 < nomMomentumError){

      aX[0][0] = 2.0487065; //a
      aX[0][1] = 21.6947935; //b
      aX[1][0] = 0.17480755; //c
      aX[1][1] = -0.50048755; //d

      aY[0][0] = 0.3654300000; //a
      aY[0][1] = 4.5252737; //b
      aY[1][0] = 0.0211000000; //c
      aY[1][1] = 0.001382242; //d

      local_x_offset       = 0.0413065;
      local_y_offset       = -0.0189837;
      local_x_slope_offset = 0.01503755;
      local_y_slope_offset = -0.009532243;

  }
  else {
    std::cout << "MatrixTransferStatic:: No valid matrix found to match beam momentum!! Skipping!!" << std::endl;
    return;
  }

  double determinant = aX[0][0] * aX[1][1] - aX[0][1] * aX[1][0];

  if (determinant == 0) {
    m_log->error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
    return;
  }

  aXinv[0][0] =  aX[1][1] / determinant;
  aXinv[0][1] = -aX[0][1] / determinant;
  aXinv[1][0] = -aX[1][0] / determinant;
  aXinv[1][1] =  aX[0][0] / determinant;


  determinant = aY[0][0] * aY[1][1] - aY[0][1] * aY[1][0];

  if (determinant == 0) {
    m_log->error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
    return;
  }

  aYinv[0][0] =  aY[1][1] / determinant;
  aYinv[0][1] = -aY[0][1] / determinant;
  aYinv[1][0] = -aY[1][0] / determinant;
  aYinv[1][1] =  aY[0][0] / determinant;

  //---- begin Reconstruction code ----

  edm4hep::Vector3f goodHit[2] = {{0.0,0.0,0.0},{0.0,0.0,0.0}};

  double goodHitX[2] = {0.0, 0.0};
  double goodHitY[2] = {0.0, 0.0};
  double goodHitZ[2] = {0.0, 0.0};

  bool goodHit1 = false;
  bool goodHit2 = false;

  std::cout << "num of raw hits: " << rechits->size() << std::endl;

  for (const auto &h: *rechits) {

    auto cellID = h.getCellID();
    // The actual hit position in Global Coordinates
    auto gpos = m_converter->position(cellID);
    // local positions
    auto volman = m_detector->volumeManager();
    auto local = volman.lookupDetElement(cellID);

    auto pos0 = local.nominal().worldToLocal(dd4hep::Position(gpos.x(), gpos.y(), gpos.z())); // hit position in local coordinates

	//std::cout << "gpos = " << gpos.z() << " pos0 = " << pos0.z() << std::endl;

    // convert into mm
    gpos = gpos/dd4hep::mm;
    pos0 = pos0/dd4hep::mm;

	std::cout << "gpos.z() = " << gpos.z() << " pos0.z() = " << pos0.z() << std::endl;

	std::cout << "[gpos.x(), gpos.y()] = " << gpos.x() <<", "<< gpos.y() << "  and [pos0.x(), pos0.y()] = "<< pos0.x()<< ", " << pos0.y() << std::endl;

    if(!goodHit2 && gpos.z() > m_cfg.hit2minZ && gpos.z() < m_cfg.hit2maxZ){

      goodHit[1].x = pos0.x();
      goodHit[1].y = pos0.y();
      goodHit[1].z = gpos.z();
      goodHit2 = true;

    }
    if(!goodHit1 && gpos.z() > m_cfg.hit1minZ && gpos.z() < m_cfg.hit1maxZ){

      goodHit[0].x = pos0.x();
      goodHit[0].y = pos0.y();
      goodHit[0].z = gpos.z();
      goodHit1 = true;

    }

  }

  // NB:
  // This is a "dumb" algorithm - I am just checking the basic thing works with a simple single-proton test.
  // Will need to update and modify for generic # of hits for more complicated final-states.

  if (goodHit1 && goodHit2) {

	std::cout << "beginning roman pots reconstruction..." << std::endl;

    // extract hit, subtract orbit offset – this is to get the hits in the coordinate system of the orbit
    // trajectory
    double XL[2] = {goodHit[0].x - local_x_offset, goodHit[1].x - local_x_offset};
    double YL[2] = {goodHit[0].y - local_y_offset, goodHit[1].y - local_y_offset};

    double base = goodHit[1].z - goodHit[0].z;

	std::cout << "base = " << base << std::endl;
	std::cout << "dd4hep::mm = " << dd4hep::mm << " dd4hep::mrad = " << dd4hep::mrad << std::endl;

    if (base == 0) {
      m_log->info("Detector separation = 0! Cannot calculate slope!");
    }
    else{

      double Xip[2] = {0.0, 0.0};
      double Xrp[2] = {XL[1], 1000*((XL[1] - XL[0]) / (base)) - local_x_slope_offset}; //- _SX0RP_;
      double Yip[2] = {0.0, 0.0};
      double Yrp[2] = {YL[1], 1000*((YL[1] - YL[0]) / (base)) - local_y_slope_offset}; //- _SY0RP_;

      // use the hit information and calculated slope at the RP + the transfer matrix inverse to calculate the
      // Polar Angle and deltaP at the IP

      for (unsigned i0 = 0; i0 < 2; i0++) {
        for (unsigned i1 = 0; i1 < 2; i1++) {
          Xip[i0] += aXinv[i0][i1] * Xrp[i1];
          Yip[i0] += aYinv[i0][i1] * Yrp[i1];
        }
      }

      // convert polar angles to radians
      double rsx = Xip[1] * dd4hep::mrad;
      double rsy = Yip[1] * dd4hep::mrad;

      // calculate momentum magnitude from measured deltaP – using thin lens optics.
      double p = nomMomentum * (1 + 0.01 * Xip[0]);
      double norm = std::sqrt(1.0 + rsx * rsx + rsy * rsy);

      edm4hep::Vector3f prec = {static_cast<float>(p * rsx / norm), static_cast<float>(p * rsy / norm),
                                static_cast<float>(p / norm)};
      auto refPoint = goodHit[0];

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
  } // end enough hits for matrix reco

}
