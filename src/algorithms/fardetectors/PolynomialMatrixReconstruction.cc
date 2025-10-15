// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Alex Jentsch
//

#include "PolynomialMatrixReconstruction.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Objects.h>
#include <DD4hep/VolumeManager.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <cmath>

#include <gsl/pointers>
#include <stdexcept>
#include <vector>

#include "algorithms/fardetectors/PolynomialMatrixReconstructionConfig.h"

void eicrecon::PolynomialMatrixReconstruction::init() {}

void eicrecon::PolynomialMatrixReconstruction::process(const PolynomialMatrixReconstruction::Input& input,
                                             const PolynomialMatrixReconstruction::Output& output) const {

  const auto [mcparts, rechits] = input;
  auto [outputParticles]        = output;

  std::vector<std::vector<double>> aX;
  std::vector<std::vector<double>> aY;

  //----- Define constants here ------
  double aXRP[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
  double aYRP[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
  
  double aXInv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
  double aYInv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};

  double nomMomentum = NAN;
  double local_x_offset{0.0};
  double local_y_offset{0.0};
  double local_x_slope_offset{0.0};
  double local_y_slope_offset{0.0};

  double numBeamProtons  = 0;
  double runningMomentum = 0.0;

  for (const auto& p : *mcparts) {
    if (mcparts->size() == 1 && p.getPDG() == 2212) { //proton particle gun for testing
      runningMomentum = p.getMomentum().z;
      numBeamProtons++;
    }
    if (p.getGeneratorStatus() == 4 && p.getPDG() == 2212) { //look for "beam" proton
      runningMomentum += p.getMomentum().z;
      numBeamProtons++;
    }
    if (p.getGeneratorStatus() == 4 &&
        p.getPDG() == 2112) { //look for "beam" neutron (for deuterons)
      runningMomentum += p.getMomentum().z;
      numBeamProtons++;
    }
	if(p.getGeneratorStatus() == 1 && p.getPDG() == 2212){
		trace("Final-state proton MCParticle momentum --> px = {}, py = {}, pz = {}", p.getMomentum().x, p.getMomentum().y, p.getMomentum().z);
    }
  }

  if (numBeamProtons == 0) {
    if (m_cfg.requireBeamProton) {
      critical("No beam protons found");
      throw std::runtime_error("No beam protons found");
    }
    return;
  }

  nomMomentum = runningMomentum / numBeamProtons;

  trace("nomMomentum extracted from beam protons --> nomMomentum = {}", nomMomentum);

  //nomMomentum = 275.0;

  double nomMomentumError = 0.05;

  //This is a temporary solution to get the beam energy information
  //needed to select the correct matrix

  //method for polynomial method of reconstruction
  //
  // 1) use the hit information to find the correct X_L value needed to evaluate the matrix
  // 2) evaluate the correct matrix for x and y reconstruction
  // 3) invert the matrices and extract the needed deltaP/p and theta_x,y values
  // 4) use 3) to calculate the momentum vector
  // 5) save output
  //
  // matrices are as follows
  //
  // | a0  a1 |   | theta_x_ip |   | x_rp       |
  // |        | x |            | = |            |
  // | b0  b1 |   | deltap/p   |   | theta_x_rp |
  //
  // | c0  c1 |   | y_ip       |   | y_rp       |
  // |        | x |            | = |            |
  // | d0  d1 |   | theta_y_ip |   | theta_y_rp |
  //
  // the "y_ip" part is a bit extraneous is more for some level of testing - we generally "assume" the vectors come from 
  // (x, y, z) = (0, 0, 0), because we don't really have a choice. 
  //
  // We will test how vertex smearing contributes to overall worsening of the resolution with the generally "correct" matrix being used
  // allowing us to isolate the various smearing effects.
  //

  /*
  bool matrix_found = false;
  for (const MatrixConfig& matrix_config : m_cfg.matrix_configs) {
    if (std::abs(matrix_config.nomMomentum - nomMomentum) / matrix_config.nomMomentum <
        nomMomentumError) {
      if (matrix_found) {
        error("Conflicting matrix values matching momentum {}", nomMomentum);
      }
      matrix_found = true;

      aX = matrix_config.aX;
      aY = matrix_config.aY;

      local_x_offset       = matrix_config.local_x_offset;
      local_y_offset       = matrix_config.local_y_offset;
      local_x_slope_offset = matrix_config.local_x_slope_offset;
      local_y_slope_offset = matrix_config.local_y_slope_offset;
    }
  }
  if (not matrix_found) {
    if (m_cfg.requireMatchingMatrix) {
      critical("No matrix found with matching beam momentum");
      throw std::runtime_error("No matrix found with matching beam momentum");
    }
    return;
  }
  */

  //---- begin Reconstruction code ----

  edm4hep::Vector3f goodHit[2] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};

  bool goodHit1 = false;
  bool goodHit2 = false;

  int numGoodHits1 = 0;
  int numGoodHits2 = 0;

  trace("size of RP hit array = {}", rechits->size());

  //
  //we need the hit information FIRST in order to start the reconstruction procedure
  //

  for (const auto& h : *rechits) {

    auto cellID = h.getCellID();
    // The actual hit position in Global Coordinates
    auto gpos = m_converter->position(cellID);
    // local positions
    auto volman = m_detector->volumeManager();
    auto local  = volman.lookupDetElement(cellID);

    auto pos0 = local.nominal().worldToLocal(
        dd4hep::Position(gpos.x(), gpos.y(), gpos.z())); // hit position in local coordinates

    // convert into mm
    gpos = gpos / dd4hep::mm;
    pos0 = pos0 / dd4hep::mm;

    trace("gpos.z() = {}, pos0.z() = {}, E_dep = {}", gpos.z(), pos0.z(), h.getEdep());

    if (gpos.z() > m_cfg.hit2minZ && gpos.z() < m_cfg.hit2maxZ) {

      trace("[gpos.x(), gpos.y(), gpos.z()] = {}, {}, {};  E_dep = {} MeV", gpos.x(), gpos.y(),
            gpos.z(), h.getEdep() * 1000);
      numGoodHits2++;
      goodHit[1].x = gpos.x(); //pos0.x() - pos0 is local coordinates, gpos is global
      goodHit[1].y = gpos.y(); //pos0.y() - temporarily changing to global to solve the local coordinate issue
      goodHit[1].z = gpos.z(); //         - which is unique to the Roman pots situation
	  if (numGoodHits2 == 1) { goodHit2 = true;} 
	  else goodHit2 = false;
    }
    if (gpos.z() > m_cfg.hit1minZ && gpos.z() < m_cfg.hit1maxZ) {

      trace("[gpos.x(), gpos.y(), gpos.z()] = {}, {}, {};  E_dep = {} MeV", gpos.x(), gpos.y(),
            gpos.z(), h.getEdep() * 1000);
      numGoodHits1++;
      goodHit[0].x = gpos.x(); //pos0.x()
      goodHit[0].y = gpos.y(); //pos0.y()
      goodHit[0].z = gpos.z();
	  if (numGoodHits1 == 1) { goodHit1 = true;} 
	  else goodHit1 = false;
    }
  }

  // NB:
  // This is a "dumb" algorithm - I am just checking the basic thing works with a simple single-proton test.
  // Will need to update and modify for generic # of hits for more complicated final-states.

  if (goodHit1 && goodHit2) {

    double base = goodHit[1].z - goodHit[0].z;

    if (base == 0) {
      info("Detector separation = 0! Cannot calculate slope!");
    }

    double XL[2] = {goodHit[0].x, goodHit[1].x};
    double YL[2] = {goodHit[0].y, goodHit[1].y};

    double Xip[2] = {0.0, 0.0};
    double Xrp[2] = {XL[1], ((XL[1] - XL[0]) / (base)) / dd4hep::mrad};
    double Yip[2] = {0.0, 0.0};
    double Yrp[2] = {YL[1], ((YL[1] - YL[0]) / (base)) / dd4hep::mrad};

    //First, we use the hit information to extract the x_L value needed for the matrix.
    //This x_L evaluation only uses the x_rp and theta_x_rp values (2D lookup).

    double extracted_xL_value = extractXLFromHits(Xrp[0], Xrp[1], "RP"); //RP string necessary - will eventually store OMD table there too 

    trace("RP extracted x_L ---> x_rp = {}, theta_x_rp = {}, x_L = {}", Xrp[0],  Xrp[1],  extracted_xL_value);

    if(extracted_xL_value == 0){
        info("Extracted X_L value is 0 --> cannot calculate matrix");
		return;
    }

    local_x_offset       = calculateOffsetFromXL(0, extracted_xL_value, "RP");
    local_x_slope_offset = calculateOffsetFromXL(1, extracted_xL_value, "RP");
    local_y_offset       = calculateOffsetFromXL(2, extracted_xL_value, "RP");
    local_y_slope_offset = calculateOffsetFromXL(3, extracted_xL_value, "RP");

    trace("RP offsets ---> local_x_offset = {},  local_x_slope_offset = {}, local_y_offset = {}, local_y_slope_offset = {}", local_x_offset, local_x_slope_offset, local_y_offset, local_y_slope_offset);
    

    Xrp[0] = Xrp[0] - local_x_offset;
    Xrp[1] = Xrp[1] - local_x_slope_offset;
    Yrp[0] = Yrp[0] - local_y_offset;
    Yrp[1] = Yrp[1] - local_y_slope_offset;

    aXRP[0][0] = calculateMatrixValueFromXL(0, extracted_xL_value, "RP");
    aXRP[0][1] = calculateMatrixValueFromXL(1, extracted_xL_value, "RP");
    aXRP[1][0] = calculateMatrixValueFromXL(2, extracted_xL_value, "RP");
    aXRP[1][1] = calculateMatrixValueFromXL(3, extracted_xL_value, "RP");

    aYRP[0][0] = calculateMatrixValueFromXL(4, extracted_xL_value, "RP");
    aYRP[0][1] = calculateMatrixValueFromXL(5, extracted_xL_value, "RP");
    aYRP[1][0] = calculateMatrixValueFromXL(6, extracted_xL_value, "RP");
    aYRP[1][1] = calculateMatrixValueFromXL(7, extracted_xL_value, "RP");

    trace("matrix values --> x[0][0] = {}, x[0][1] = {}, x[1][0] = {}, x[1][1] = {}", aXRP[0][0], aXRP[0][1], aXRP[1][0], aXRP[1][1]);
	trace("matrix values --> y[0][0] = {}, y[0][1] = {}, y[1][0] = {}, y[1][1] = {}", aYRP[0][0], aYRP[0][1], aYRP[1][0], aYRP[1][1]);

    double determinant_x = aXRP[0][0]*aXRP[1][1] - aXRP[0][1]*aXRP[1][0];
    double determinant_y = aYRP[0][0]*aYRP[1][1] - aYRP[0][1]*aYRP[1][0];
										
    if (determinant_x == 0 || determinant_y == 0) {
      error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
      return;
    }

	
    aXInv[0][0] =    (aXRP[1][1]/determinant_x);
    aXInv[0][1] = -1*(aXRP[0][1]/determinant_x);
    aXInv[1][0] = -1*(aXRP[1][0]/determinant_x);
    aXInv[1][1] =    (aXRP[0][0]/determinant_x);
						
    aYInv[0][0] =    (aYRP[1][1]/determinant_y);
    aYInv[0][1] = -1*(aYRP[0][1]/determinant_y);
    aYInv[1][0] = -1*(aYRP[1][0]/determinant_y);
    aYInv[1][1] =    (aYRP[0][0]/determinant_y);


    // extract hit, subtract orbit offset – this is to get the hits in the coordinate system of the orbit
    // trajectory

      // use the hit information and calculated slope at the RP + the transfer matrix inverse to calculate the
      // Polar Angle and deltaP at the IP
	/*
      for (unsigned i0 = 0; i0 < 2; i0++) {
        for (unsigned i1 = 0; i1 < 2; i1++) {
          Xip[i0] += aXInv[i0][i1] * Xrp[i1];
          Yip[i0] += aYInv[i0][i1] * Yrp[i1];
        }
      }
	  */
      //Yip[1] = Yrp[0] / aY[0][1];

	  double thetax_ip      = 0.0;
	  double thetay_ip      = 0.0;
	  double delta_p_over_p = 0.0;

	  thetax_ip      = aXInv[0][0]*Xrp[0] + aXInv[0][1]*Xrp[1];
	  delta_p_over_p = aXInv[1][0]*Xrp[0] + aXInv[1][1]*Xrp[1];
	  
	  thetay_ip      = aYInv[1][0]*Yrp[0] + aYInv[1][1]*Yrp[1];

      // convert polar angles to radians
      //double rsx = Xip[0] * dd4hep::mrad;
      //double rsy = Yip[1] * dd4hep::mrad;

      double rsx = thetax_ip/1000., rsy = thetay_ip/1000.;

      // calculate momentum magnitude from measured deltaP – using thin lens optics.
	  double momOrbit = (extracted_xL_value/100.0)*nomMomentum;
	  
      double p    = momOrbit * (1 + 0.01 * delta_p_over_p);
      double norm = std::sqrt(1.0 + rsx * rsx + rsy * rsy);

      edm4hep::Vector3f prec = {static_cast<float>(p * rsx / norm),
                                static_cast<float>(p * rsy / norm), static_cast<float>(p / norm)};
      auto refPoint          = goodHit[0];

      trace("RP polynomial Reco Momentum ---> px = {},  py = {}, pz = {}", prec.x, prec.y, prec.z);

      //----- end reconstruction code ------

      edm4eic::MutableReconstructedParticle reconTrack;
      reconTrack.setType(0);
      reconTrack.setMomentum(prec);
      reconTrack.setEnergy(
          std::hypot(edm4hep::utils::magnitude(reconTrack.getMomentum()), m_cfg.partMass));
      reconTrack.setReferencePoint(refPoint);
      reconTrack.setCharge(m_cfg.partCharge);
      reconTrack.setMass(m_cfg.partMass);
      reconTrack.setGoodnessOfPID(1.);
      reconTrack.setPDG(m_cfg.partPDG);
      //reconTrack.covMatrix(); // @TODO: Errors
      outputParticles->push_back(reconTrack);
    }
  } // end enough hits for matrix reco

