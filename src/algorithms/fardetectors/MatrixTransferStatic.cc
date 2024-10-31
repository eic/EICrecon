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

void eicrecon::MatrixTransferStatic::init() {

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
          if(mcparts->size() == 1 && p.getPDG() == 2212){
                runningMomentum = p.getMomentum().z;
                numBeamProtons++;
          }
        if (p.getGeneratorStatus() == 4 && p.getPDG() == 2212) { //look for "beam" proton
                runningMomentum += p.getMomentum().z;
                numBeamProtons++;
        }
  }

  if(numBeamProtons == 0) {error("No beam protons to choose matrix!! Skipping!!"); return;}

  nomMomentum = runningMomentum/numBeamProtons;

  double nomMomentumError = 0.05;

  //This is a temporary solution to get the beam energy information
  //needed to select the correct matrix

  if(abs(275.0 - nomMomentum)/275.0 < nomMomentumError){

     aX[0][0] = 3.251116; //a
     aX[0][1] = 30.285734; //b
     aX[1][0] = 0.186036375; //c
     aX[1][1] = 0.196439472; //d

     aY[0][0] = 0.4730500000; //a
     aY[0][1] = 3.062999454; //b
     aY[1][0] = 0.0204108951; //c
     aY[1][1] = -0.139318692; //d

     local_x_offset       = -1209.29;//-0.339334; these are the local coordinate values
     local_y_offset       = 0.00132511;//-0.000299454;
     local_x_slope_offset = -45.4772;//-0.219603248;
     local_y_slope_offset = 0.000745498;//-0.000176128;

  }
  else if(abs(100.0 - nomMomentum)/100.0 < nomMomentumError){

     aX[0][0] = 3.152158; //a
     aX[0][1] = 20.852072; //b
     aX[1][0] = 0.181649517; //c
     aX[1][1] = -0.303998487; //d

     aY[0][0] = 0.5306100000; //a
     aY[0][1] = 3.19623343; //b
     aY[1][0] = 0.0226283320; //c
     aY[1][1] = -0.082666019; //d

     local_x_offset       = -1209.27;//-0.329072;
     local_y_offset       = 0.00355218;//-0.00028343;
     local_x_slope_offset = -45.4737;//-0.218525084;
     local_y_slope_offset = 0.00204394;//-0.00015321;

  }
  else if(abs(41.0 - nomMomentum)/41.0 < nomMomentumError){

         aX[0][0] = 3.135997; //a
         aX[0][1] = 18.482273; //b
         aX[1][0] = 0.176479921; //c
         aX[1][1] = -0.497839483; //d

         aY[0][0] = 0.4914400000; //a
         aY[0][1] = 4.53857451; //b
         aY[1][0] = 0.0179664765; //c
         aY[1][1] = 0.004160679; //d

         local_x_offset       = -1209.22;//-0.283273;
         local_y_offset       = 0.00868737;//-0.00552451;
         local_x_slope_offset = -45.4641;//-0.21174031;
         local_y_slope_offset = 0.00498786;//-0.003212011;

  }
  else if(abs(135.0 - nomMomentum)/135.0 < nomMomentumError){ //135 GeV deuterons

      aX[0][0] = 1.6248;
      aX[0][1] = 12.966293;
      aX[1][0] = 0.1832;
      aX[1][1] = -2.8636535;

      aY[0][0] = 0.0001674; //a
      aY[0][1] = -28.6003; //b
      aY[1][0] = 0.0000837; //c
      aY[1][1] = -2.87985; //d

      local_x_offset       = -11.9872;
      local_y_offset       = -0.0146;
      local_x_slope_offset = -14.75315;
      local_y_slope_offset = -0.0073;

  }
  else {
    error("MatrixTransferStatic:: No valid matrix found to match beam momentum!! Skipping!!");
    return;
  }

  double determinant = aX[0][0] * aX[1][1] - aX[0][1] * aX[1][0];

  if (determinant == 0) {
    error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
    return;
  }

  aXinv[0][0] =  aX[1][1] / determinant;
  aXinv[0][1] = -aX[0][1] / determinant;
  aXinv[1][0] = -aX[1][0] / determinant;
  aXinv[1][1] =  aX[0][0] / determinant;


  determinant = aY[0][0] * aY[1][1] - aY[0][1] * aY[1][0];

  if (determinant == 0) {
    error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
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

  for (const auto &h: *rechits) {

    auto cellID = h.getCellID();
    // The actual hit position in Global Coordinates
    auto gpos = m_converter->position(cellID);
    // local positions
    auto volman = m_detector->volumeManager();
    auto local = volman.lookupDetElement(cellID);

    auto pos0 = local.nominal().worldToLocal(dd4hep::Position(gpos.x(), gpos.y(), gpos.z())); // hit position in local coordinates

    // convert into mm
    gpos = gpos/dd4hep::mm;
    pos0 = pos0/dd4hep::mm;

    //std::cout << "gpos.z() = " << gpos.z() << " pos0.z() = " << pos0.z() << std::endl;
    //std::cout << "[gpos.x(), gpos.y()] = " << gpos.x() <<", "<< gpos.y() << "  and [pos0.x(), pos0.y()] = "<< pos0.x()<< ", " << pos0.y() << std::endl;

    if(!goodHit2 && gpos.z() > m_cfg.hit2minZ && gpos.z() < m_cfg.hit2maxZ){

      goodHit[1].x = gpos.x(); //pos0.x() - pos0 is local coordinates, gpos is global
      goodHit[1].y = gpos.y(); //pos0.y() - temporarily changing to global to solve the local coordinate issue
      goodHit[1].z = gpos.z(); //         - which is unique to the Roman pots situation
      goodHit2 = true;

    }
    if(!goodHit1 && gpos.z() > m_cfg.hit1minZ && gpos.z() < m_cfg.hit1maxZ){

      goodHit[0].x = gpos.x(); //pos0.x()
      goodHit[0].y = gpos.y(); //pos0.y()
      goodHit[0].z = gpos.z();
      goodHit1 = true;

    }

  }

  // NB:
  // This is a "dumb" algorithm - I am just checking the basic thing works with a simple single-proton test.
  // Will need to update and modify for generic # of hits for more complicated final-states.

  if (goodHit1 && goodHit2) {

    // extract hit, subtract orbit offset – this is to get the hits in the coordinate system of the orbit
    // trajectory
    double XL[2] = {goodHit[0].x - local_x_offset, goodHit[1].x - local_x_offset};
    double YL[2] = {goodHit[0].y - local_y_offset, goodHit[1].y - local_y_offset};

    double base = goodHit[1].z - goodHit[0].z;

    if (base == 0) {
      info("Detector separation = 0! Cannot calculate slope!");
    }
    else{

      double Xip[2] = {0.0, 0.0};
      double Xrp[2] = {XL[1], ((XL[1] - XL[0]) / (base))/dd4hep::mrad - local_x_slope_offset};
      double Yip[2] = {0.0, 0.0};
      double Yrp[2] = {YL[1], ((YL[1] - YL[0]) / (base))/dd4hep::mrad - local_y_slope_offset};

      // use the hit information and calculated slope at the RP + the transfer matrix inverse to calculate the
      // Polar Angle and deltaP at the IP

      for (unsigned i0 = 0; i0 < 2; i0++) {
        for (unsigned i1 = 0; i1 < 2; i1++) {
          Xip[i0] += aXinv[i0][i1] * Xrp[i1];
          Yip[i0] += aYinv[i0][i1] * Yrp[i1];
        }
      }

          Yip[1] = Yrp[0]/aY[0][1];

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
