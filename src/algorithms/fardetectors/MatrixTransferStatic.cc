// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Alex Jentsch, Wouter Deconinck, Sylvester Joosten, David Lawrence, Simon Gardner
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
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <cmath>

#include <gsl/pointers>
#include <stdexcept>
#include <vector>

#include "algorithms/fardetectors/MatrixTransferStaticConfig.h"

void eicrecon::MatrixTransferStatic::init() {}

void eicrecon::MatrixTransferStatic::process(const MatrixTransferStatic::Input& input,
                                             const MatrixTransferStatic::Output& output) const {

  const auto [mcparts, rechits] = input;
  auto [outputParticles]        = output;

  std::vector<std::vector<double>> aX;
  std::vector<std::vector<double>> aY;

  //----- Define constants here ------
  double aXinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
  double aYinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};

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
  }

  if (numBeamProtons == 0) {
    if (m_cfg.requireBeamProton) {
      error("No beam protons found");
      throw std::runtime_error("No beam protons found");
    }
    return;
  }

  nomMomentum = runningMomentum / numBeamProtons;

  double nomMomentumError = 0.05;

  //This is a temporary solution to get the beam energy information
  //needed to select the correct matrix

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
      error("No matrix found with matching beam momentum");
      throw std::runtime_error("No matrix found with matching beam momentum");
    }
    return;
  }

  double determinant = aX[0][0] * aX[1][1] - aX[0][1] * aX[1][0];

  if (determinant == 0) {
    error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
    return;
  }

  aXinv[0][0] = aX[1][1] / determinant;
  aXinv[0][1] = -aX[0][1] / determinant;
  aXinv[1][0] = -aX[1][0] / determinant;
  aXinv[1][1] = aX[0][0] / determinant;

  determinant = aY[0][0] * aY[1][1] - aY[0][1] * aY[1][0];

  if (determinant == 0) {
    error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
    return;
  }

  aYinv[0][0] = aY[1][1] / determinant;
  aYinv[0][1] = -aY[0][1] / determinant;
  aYinv[1][0] = -aY[1][0] / determinant;
  aYinv[1][1] = aY[0][0] / determinant;

  //---- begin Reconstruction code ----

  edm4hep::Vector3f goodHit[2] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};

  bool goodHit1 = false;
  bool goodHit2 = false;

  int numGoodHits1 = 0;
  int numGoodHits2 = 0;

  trace("size of RP hit array = {}", rechits->size());

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
      goodHit[1].y =
          gpos.y(); //pos0.y() - temporarily changing to global to solve the local coordinate issue
      goodHit[1].z = gpos.z(); //         - which is unique to the Roman pots situation
      goodHit2     = numGoodHits2 == 1;
    }
    if (gpos.z() > m_cfg.hit1minZ && gpos.z() < m_cfg.hit1maxZ) {

      trace("[gpos.x(), gpos.y(), gpos.z()] = {}, {}, {};  E_dep = {} MeV", gpos.x(), gpos.y(),
            gpos.z(), h.getEdep() * 1000);
      numGoodHits1++;
      goodHit[0].x = gpos.x(); //pos0.x()
      goodHit[0].y = gpos.y(); //pos0.y()
      goodHit[0].z = gpos.z();
      goodHit1     = numGoodHits1 == 1;
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
    } else {

      double Xip[2] = {0.0, 0.0};
      double Xrp[2] = {XL[1], ((XL[1] - XL[0]) / (base)) / dd4hep::mrad - local_x_slope_offset};
      double Yip[2] = {0.0, 0.0};
      double Yrp[2] = {YL[1], ((YL[1] - YL[0]) / (base)) / dd4hep::mrad - local_y_slope_offset};

      // use the hit information and calculated slope at the RP + the transfer matrix inverse to calculate the
      // Polar Angle and deltaP at the IP

      for (unsigned i0 = 0; i0 < 2; i0++) {
        for (unsigned i1 = 0; i1 < 2; i1++) {
          Xip[i0] += aXinv[i0][i1] * Xrp[i1];
          Yip[i0] += aYinv[i0][i1] * Yrp[i1];
        }
      }

      Yip[1] = Yrp[0] / aY[0][1];

      // convert polar angles to radians
      double rsx = Xip[1] * dd4hep::mrad;
      double rsy = Yip[1] * dd4hep::mrad;

      // calculate momentum magnitude from measured deltaP – using thin lens optics.
      double p    = nomMomentum * (1 + 0.01 * Xip[0]);
      double norm = std::sqrt(1.0 + rsx * rsx + rsy * rsy);

      edm4hep::Vector3f prec = {static_cast<float>(p * rsx / norm),
                                static_cast<float>(p * rsy / norm), static_cast<float>(p / norm)};
      auto refPoint          = goodHit[0];

      trace("RP Reco Momentum ---> px = {},  py = {}, pz = {}", prec.x, prec.y, prec.z);

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
}
