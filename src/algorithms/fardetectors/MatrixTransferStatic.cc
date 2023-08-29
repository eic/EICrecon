// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Alex Jentsch, Wouter Deconinck, Sylvester Joosten, David Lawrence
//
// This converted from: https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/FarForwardParticles.cpp

#include "MatrixTransferStatic.h"

void eicrecon::MatrixTransferStatic::init(std::shared_ptr<spdlog::logger> &logger) {

  m_log = logger;

  // do not get the layer/sector ID if no readout class provided
  if (m_cfg.readout.empty()){m_log->error("READOUT IS EMPTY!"); return;}

  //Calculate inverse of static transfer matrix
  std::vector<std::vector<double>> aX(m_cfg.aX);
  std::vector<std::vector<double>> aY(m_cfg.aY);

  double det = aX[0][0] * aX[1][1] - aX[0][1] * aX[1][0];

  if (det == 0) {
    m_log->error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
    return;
  }

  aXinv[0][0] =  aX[1][1] / det;
  aXinv[0][1] = -aX[0][1] / det;
  aXinv[1][0] = -aX[1][0] / det;
  aXinv[1][1] =  aX[0][0] / det;


  det = aY[0][0] * aY[1][1] - aY[0][1] * aY[1][0];

  if (det == 0) {
    m_log->error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
    return;
  }

  aYinv[0][0] =  aY[1][1] / det;
  aYinv[0][1] = -aY[0][1] / det;
  aYinv[1][0] = -aY[1][0] / det;
  aYinv[1][1] =  aY[0][0] / det;

  return;

}

std::unique_ptr<edm4eic::ReconstructedParticleCollection> eicrecon::MatrixTransferStatic::produce(const edm4hep::SimTrackerHitCollection& rawhits) {

  auto outputParticles = std::make_unique<edm4eic::ReconstructedParticleCollection>();

  //---- begin Reconstruction code ----

  double goodHitX[2] = {0.0, 0.0};
  double goodHitY[2] = {0.0, 0.0};
  double goodHitZ[2] = {0.0, 0.0};

  bool goodHit1 = false;
  bool goodHit2 = false;

  for (const auto &h: rawhits) {

    auto cellID = h.getCellID();
    // The actual hit position in Global Coordinates
    auto gpos = m_cellid_converter->position(cellID);
    // local positions
    auto volman = m_detector->volumeManager();
    auto local = volman.lookupDetElement(cellID);

    auto pos0 = local.nominal().worldToLocal(dd4hep::Position(gpos.x(), gpos.y(), gpos.z())); // hit position in local coordinates

    if(!goodHit2 && gpos.z()/dd4hep::mm > m_cfg.hit2minZ && gpos.z()/dd4hep::mm < m_cfg.hit2maxZ){

      goodHitX[1] = pos0.x()/dd4hep::mm;
      goodHitY[1] = pos0.y()/dd4hep::mm;
      goodHitZ[1] = gpos.z()/dd4hep::mm;
      goodHit2 = true;

    }
    if(!goodHit1 && gpos.z()/dd4hep::mm > m_cfg.hit1minZ && gpos.z()/dd4hep::mm < m_cfg.hit1maxZ){

      goodHitX[0] = pos0.x()/dd4hep::mm;
      goodHitY[0] = pos0.y()/dd4hep::mm;
      goodHitZ[0] = gpos.z()/dd4hep::mm;
      goodHit1 = true;

    }

  }

  // NB:
  // This is a "dumb" algorithm - I am just checking the basic thing works with a simple single-proton test.
  // Will need to update and modify for generic # of hits for more complicated final-states.

  if (goodHit1 && goodHit2) {

    // extract hit, subtract orbit offset – this is to get the hits in the coordinate system of the orbit
    // trajectory
    double XL[2] = {goodHitX[0] - m_cfg.local_x_offset, goodHitX[1] - m_cfg.local_x_offset};
    double YL[2] = {goodHitY[0] - m_cfg.local_y_offset, goodHitY[1] - m_cfg.local_y_offset};

    double base = goodHitZ[1] - goodHitZ[0];

    if (base == 0) {
      m_log->info("Detector separation = 0! Cannot calculate slope!");
    }
    else{

      double Xip[2] = {0.0, 0.0};
      double Xrp[2] = {XL[1], (1000 * (XL[1] - XL[0]) / (base)) - m_cfg.local_x_slope_offset}; //- _SX0RP_;
      double Yip[2] = {0.0, 0.0};
      double Yrp[2] = {YL[1], (1000 * (YL[1] - YL[0]) / (base)) - m_cfg.local_y_slope_offset}; //- _SY0RP_;

      // use the hit information and calculated slope at the RP + the transfer matrix inverse to calculate the
      // Polar Angle and deltaP at the IP

      for (unsigned i0 = 0; i0 < 2; i0++) {
	for (unsigned i1 = 0; i1 < 2; i1++) {
	  Xip[i0] += aXinv[i0][i1] * Xrp[i1];
	  Yip[i0] += aYinv[i0][i1] * Yrp[i1];
	}
      }

      // convert polar angles to radians
      double rsx = Xip[1] / 1000.;
      double rsy = Yip[1] / 1000.;

      // calculate momentum magnitude from measured deltaP – using thin lens optics.
      double p = m_cfg.nomMomentum * (1 + 0.01 * Xip[0]);
      double norm = std::sqrt(1.0 + rsx * rsx + rsy * rsy);

      const float prec[3] = {static_cast<float>(p * rsx / norm), static_cast<float>(p * rsy / norm),
			     static_cast<float>(p / norm)};
      float refPoint[3] = {static_cast<float>(goodHitX[0]), static_cast<float>(goodHitY[0]), static_cast<float>(goodHitZ[0])};

      //----- end reconstruction code ------

      edm4eic::MutableReconstructedParticle reconTrack;
      reconTrack.setType(0);
      reconTrack.setMomentum({prec});
      reconTrack.setEnergy(std::hypot(edm4eic::magnitude(reconTrack.getMomentum()), m_cfg.partMass));
      reconTrack.setReferencePoint({refPoint});
      reconTrack.setCharge(m_cfg.partCharge);
      reconTrack.setMass(m_cfg.partMass);
      reconTrack.setGoodnessOfPID(1.);
      reconTrack.setPDG(m_cfg.partPDG);
      //reconTrack.covMatrix(); // @TODO: Errors
      outputParticles->push_back(edm4eic::ReconstructedParticle(reconTrack));
    }
  } // end enough hits for matrix reco

  return outputParticles;

}
