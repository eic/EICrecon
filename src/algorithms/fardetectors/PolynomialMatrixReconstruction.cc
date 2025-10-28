// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Alex Jentsch
//

#include "algorithms/fardetectors/PolynomialMatrixReconstruction.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Objects.h>
#include <DD4hep/VolumeManager.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <TGraph2D.h>
#include <TString.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <cmath>
#include <gsl/pointers>
#include <memory>
#include <stdexcept>
#include <vector>
#include <filesystem>

#include "algorithms/fardetectors/PolynomialMatrixReconstructionConfig.h"

namespace eicrecon {

void eicrecon::PolynomialMatrixReconstruction::init() {}

void eicrecon::PolynomialMatrixReconstruction::process(
    const PolynomialMatrixReconstruction::Input& input,
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

  double nomMomentum          = NAN;
  double local_x_offset       = 0.0;
  double local_y_offset       = 0.0;
  double local_x_slope_offset = 0.0;
  double local_y_slope_offset = 0.0;

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
    if (p.getGeneratorStatus() == 1 && p.getPDG() == 2212) {
      double xMom_noXAngle = sin(m_cfg.crossingAngle) * p.getMomentum().z +
                             cos(m_cfg.crossingAngle) * p.getMomentum().x;

      trace("Final-state proton MCParticle momentum --> px = {}, py = {}, pz = {}, theta_x_IP = "
            "{}, theta_y_IP = {}",
            xMom_noXAngle, p.getMomentum().y, p.getMomentum().z, xMom_noXAngle / p.getMomentum().z,
            p.getMomentum().y / p.getMomentum().z);
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

  double nomMomentumError = 0.05;

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

  bool valid_energy_found = false;
  for (const PolynomialMatrixConfig& poly_matrix_configs : m_cfg.poly_matrix_configs) {
    if (std::abs(poly_matrix_configs.nomMomentum - nomMomentum) / poly_matrix_configs.nomMomentum <
        nomMomentumError) {
      if (valid_energy_found) {
        error("Conflicting matrix values matching momentum {}", nomMomentum);
      }
      valid_energy_found = true;
      nomMomentum        = poly_matrix_configs.nomMomentum;
    }
  }
  if (not valid_energy_found) {
    if (m_cfg.requireValidBeamEnergy) {
      critical("No tune beam energy found - cannot acquire lookup table");
      throw std::runtime_error("No valid beam energy found, cannot reconstruct momentum");
    }
    return;
  }

  //xL table filled here from LUT -- Graph2D used for nice interpolation functionality and simple loading of LUT file

  thread_local std::string filename(
      std::format("calibrations/RP_60_xL_100_beamEnergy_{:.0f}.xL.lut", nomMomentum));
  thread_local std::unique_ptr<TGraph2D> xLGraph{nullptr};
  if (xLGraph == nullptr) {
    if (std::filesystem::exists(filename)) {
      xLGraph = std::make_unique<TGraph2D>(filename.c_str(), "%lf %lf %lf");
    } else {
      critical("Cannot find lookup xL table for {}", nomMomentum);
      throw std::runtime_error("Cannot find xL lookup table from calibrations -- cannot proceed");
    }
  }

  trace("filename for lookup --> {}", filename);

  //important to ensure interoplation works correctly -- do not remove -- not available until ROOT v6.36, will need to add back in later
  //xLGraph->RemoveDuplicates();

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

    trace("gpos.z() = {}, gpos.x() = {}, gpos.y() = {}, E_dep = {}", gpos.z(), gpos.x(), gpos.y(),
          h.getEdep());

    if (gpos.z() > m_cfg.hit2minZ && gpos.z() < m_cfg.hit2maxZ) {

      trace("[gpos.x(), gpos.y(), gpos.z()] = {}, {}, {};  E_dep = {} MeV", gpos.x(), gpos.y(),
            gpos.z(), h.getEdep() * 1000);
      numGoodHits2++;
      goodHit[1].x = gpos.x(); //pos0.x() - pos0 is local coordinates, gpos is global
      goodHit[1].y =
          gpos.y(); //pos0.y() - temporarily changing to global to solve the local coordinate issue
      goodHit[1].z = gpos.z(); //         - which is unique to the Roman pots situation
      if (numGoodHits2 == 1) {
        goodHit2 = true;
      } else
        goodHit2 = false;
    }
    if (gpos.z() > m_cfg.hit1minZ && gpos.z() < m_cfg.hit1maxZ) {

      trace("[gpos.x(), gpos.y(), gpos.z()] = {}, {}, {};  E_dep = {} MeV", gpos.x(), gpos.y(),
            gpos.z(), h.getEdep() * 1000);
      numGoodHits1++;
      goodHit[0].x = gpos.x(); //pos0.x()
      goodHit[0].y = gpos.y(); //pos0.y()
      goodHit[0].z = gpos.z();
      if (numGoodHits1 == 1) {
        goodHit1 = true;
      } else
        goodHit1 = false;
    }
  }

  // This algorithm uses single hits from the first and last layer of the RP
  // Better to do a linear fit with all 4 layers to improve resolution and to
  // better handle both multi-particle final-states and cases where large numbers
  // of secondaries are produced from spallation on detector material

  if (goodHit1 && goodHit2) {

    double base = goodHit[1].z - goodHit[0].z;

    if (base == 0) {
      info("Detector separation = 0! Cannot calculate slope!");
      //delete avoids overwriting of TGraph2D and memory leak warning
      // cannot load the table in init stage because the beam energy is
      // extracted event by event
      // can be fixed with beam energy meta data being added in the npsim
      // output tree
      throw std::runtime_error("Detector separation = 0! Cannot calculate slope!");
    }

    double XL[2] = {goodHit[0].x, goodHit[1].x};
    double YL[2] = {goodHit[0].y, goodHit[1].y};

    double Xrp[2] = {XL[1], ((XL[1] - XL[0]) / (base)) / dd4hep::mrad};
    double Yrp[2] = {YL[1], ((YL[1] - YL[0]) / (base)) / dd4hep::mrad};

    //First, we use the hit information to extract the x_L value needed for the matrix.
    //This x_L evaluation only uses the x_rp and theta_x_rp values (2D lookup).

    //100 converts xL from decimal to percentage -- it's how the fits were done.
    double extracted_xL_value = 100 * xLGraph->Interpolate(Xrp[0], Xrp[1]);

    trace("RP extracted x_L ---> x_rp = {}, theta_x_rp = {}, x_L = {}", Xrp[0], Xrp[1],
          extracted_xL_value);

    if (extracted_xL_value == 0) {
      error("Extracted X_L value is 0 --> cannot calculate matrix");
      throw std::runtime_error("Extracted X_L value is 0 --> cannot calculate matrix");
    }

    local_x_offset       = calculateOffsetFromXL(0, extracted_xL_value, nomMomentum);
    local_x_slope_offset = calculateOffsetFromXL(1, extracted_xL_value, nomMomentum);
    local_y_offset       = calculateOffsetFromXL(2, extracted_xL_value, nomMomentum);
    local_y_slope_offset = calculateOffsetFromXL(3, extracted_xL_value, nomMomentum);

    trace("RP offsets ---> local_x_offset = {},  local_x_slope_offset = {}, local_y_offset = {}, "
          "local_y_slope_offset = {}",
          local_x_offset, local_x_slope_offset, local_y_offset, local_y_slope_offset);

    // extract hit, subtract orbit offset – this is to get the hits in the coordinate system of the orbit
    // trajectory --> then we can get the matrix parameters

    Xrp[0] = Xrp[0] - local_x_offset;
    Xrp[1] = Xrp[1] - local_x_slope_offset;
    Yrp[0] = Yrp[0] - local_y_offset;
    Yrp[1] = Yrp[1] - local_y_slope_offset;

    aXRP[0][0] = calculateMatrixValueFromXL(0, extracted_xL_value, nomMomentum);
    aXRP[0][1] = calculateMatrixValueFromXL(1, extracted_xL_value, nomMomentum);
    aXRP[1][0] = calculateMatrixValueFromXL(2, extracted_xL_value, nomMomentum);
    aXRP[1][1] = calculateMatrixValueFromXL(3, extracted_xL_value, nomMomentum);

    aYRP[0][0] = calculateMatrixValueFromXL(4, extracted_xL_value, nomMomentum);
    aYRP[0][1] = calculateMatrixValueFromXL(5, extracted_xL_value, nomMomentum);
    aYRP[1][0] = calculateMatrixValueFromXL(6, extracted_xL_value, nomMomentum);
    aYRP[1][1] = calculateMatrixValueFromXL(7, extracted_xL_value, nomMomentum);

    trace("matrix values --> x[0][0] = {}, x[0][1] = {}, x[1][0] = {}, x[1][1] = {}", aXRP[0][0],
          aXRP[0][1], aXRP[1][0], aXRP[1][1]);
    trace("matrix values --> y[0][0] = {}, y[0][1] = {}, y[1][0] = {}, y[1][1] = {}", aYRP[0][0],
          aYRP[0][1], aYRP[1][0], aYRP[1][1]);

    double determinant_x = aXRP[0][0] * aXRP[1][1] - aXRP[0][1] * aXRP[1][0];
    double determinant_y = aYRP[0][0] * aYRP[1][1] - aYRP[0][1] * aYRP[1][0];

    if (determinant_x == 0 || determinant_y == 0) {
      error("Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
      throw std::runtime_error(
          "Reco matrix determinant = 0! Matrix cannot be inverted! Double-check matrix!");
    }

    aXInv[0][0] = (aXRP[1][1] / determinant_x);
    aXInv[0][1] = -1 * (aXRP[0][1] / determinant_x);
    aXInv[1][0] = -1 * (aXRP[1][0] / determinant_x);
    aXInv[1][1] = (aXRP[0][0] / determinant_x);

    aYInv[0][0] = (aYRP[1][1] / determinant_y);
    aYInv[0][1] = -1 * (aYRP[0][1] / determinant_y);
    aYInv[1][0] = -1 * (aYRP[1][0] / determinant_y);
    aYInv[1][1] = (aYRP[0][0] / determinant_y);

    double thetax_ip        = aXInv[0][0] * Xrp[0] + aXInv[0][1] * Xrp[1];
    double delta_p_over_p   = aXInv[1][0] * Xrp[0] + aXInv[1][1] * Xrp[1];
    double thetay_ip        = aYInv[1][0] * Yrp[0] + aYInv[1][1] * Yrp[1];
    double thetay_ip_STATIC = Yrp[0] / aYRP[0][1];
    double thetay_AVG =
        0.5 * (thetay_ip +
               thetay_ip_STATIC); //approximation to solve issue with small numbers in y-matrix

    trace(
        "theta_x_IP_proper = {}, thetay_IP_proper = {}, thetay_IP_STATIC = {}, thetay_IP_AVG = {}",
        thetax_ip / 1000, thetay_ip / 1000, thetay_ip_STATIC / 1000, thetay_AVG / 1000);

    double rsx = thetax_ip * dd4hep::mrad;
    double rsy = thetay_AVG * dd4hep::mrad; //using approximation here for the theta_y

    // calculate momentum magnitude from measured deltaP – using thin lens optics.
    double momOrbit = (extracted_xL_value / 100.0) * nomMomentum;

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

  } // end enough hits for matrix reco

} //end ::process

double PolynomialMatrixReconstruction::calculateOffsetFromXL(int whichOffset, double x_L,
                                                             double beamEnergy) const {

  if (whichOffset >= 4) {
    throw std::runtime_error(fmt::format("Bad offset index {}", whichOffset));
  }

  double offset_value_and_par[4][3];

  //because of the optics configuration, these 6 parameters are zero
  //but this may not always be true, so we leave these parameters here
  //to account for x-y coupling with different optics or for the OMD

  offset_value_and_par[2][0] = 0.0;
  offset_value_and_par[2][1] = 0.0;
  offset_value_and_par[2][2] = 0.0;
  offset_value_and_par[3][0] = 0.0;
  offset_value_and_par[3][1] = 0.0;
  offset_value_and_par[3][2] = 0.0;

  if (beamEnergy == 275) { //275 GeV
    offset_value_and_par[0][0] = -1916.507316;
    offset_value_and_par[0][1] = 11.100930;
    offset_value_and_par[0][2] = -0.040338;
    offset_value_and_par[1][0] = -84.913431;
    offset_value_and_par[1][1] = 0.614101;
    offset_value_and_par[1][2] = -0.002200;
  } else if (beamEnergy == 100) { //100 GeV
    offset_value_and_par[0][0] = -1839.212116;
    offset_value_and_par[0][1] = 9.573747;
    offset_value_and_par[0][2] = -0.032770;
    offset_value_and_par[1][0] = -80.659006;
    offset_value_and_par[1][1] = 0.530685;
    offset_value_and_par[1][2] = -0.001790;
  } else if (beamEnergy == 130) { //130 GeV
    offset_value_and_par[0][0] = -1875.115783;
    offset_value_and_par[0][1] = 10.291006;
    offset_value_and_par[0][2] = -0.036341;
    offset_value_and_par[1][0] = -82.577631;
    offset_value_and_par[1][1] = 0.567990;
    offset_value_and_par[1][2] = -0.001972;
  } else if (beamEnergy == 41) { //41 GeV
    offset_value_and_par[0][0] = -1754.718344;
    offset_value_and_par[0][1] = 7.767054;
    offset_value_and_par[0][2] = -0.023105;
    offset_value_and_par[1][0] = -73.956525;
    offset_value_and_par[1][1] = 0.391292;
    offset_value_and_par[1][2] = -0.001063;
  } else
    throw std::runtime_error(fmt::format("Unknown beamEnergy {}", beamEnergy));

  return (offset_value_and_par[whichOffset][0] + offset_value_and_par[whichOffset][1] * x_L +
          offset_value_and_par[whichOffset][2] * x_L * x_L);
}

double PolynomialMatrixReconstruction::calculateMatrixValueFromXL(int whichElement, double x_L,
                                                                  double beamEnergy) const {

  double matrix_value_and_par[8][3];

  if ((whichElement < 0) || (whichElement > 7)) {
    throw std::runtime_error(fmt::format("Bad Array index(ces)", whichElement));
  }

  if (beamEnergy == 275) { //275 GeV
    matrix_value_and_par[0][0] = -94.860289;
    matrix_value_and_par[0][1] = 2.373011;
    matrix_value_and_par[0][2] = -0.011253;
    matrix_value_and_par[1][0] = 4.029769;
    matrix_value_and_par[1][1] = 0.011753;
    matrix_value_and_par[1][2] = -0.000198;
    matrix_value_and_par[2][0] = -8.278798;
    matrix_value_and_par[2][1] = 0.157343;
    matrix_value_and_par[2][2] = -0.000728;
    matrix_value_and_par[3][0] = 0.217521;
    matrix_value_and_par[3][1] = 0.000788;
    matrix_value_and_par[3][2] = -0.000011;
    matrix_value_and_par[4][0] = -0.224777;
    matrix_value_and_par[4][1] = 0.003369;
    matrix_value_and_par[4][2] = -0.000015;
    matrix_value_and_par[5][0] = -162.881282;
    matrix_value_and_par[5][1] = 2.959217;
    matrix_value_and_par[5][2] = -0.013032;
    matrix_value_and_par[6][0] = -0.006100;
    matrix_value_and_par[6][1] = 0.000045;
    matrix_value_and_par[6][2] = 0.000000;
    matrix_value_and_par[7][0] = -7.781919;
    matrix_value_and_par[7][1] = 0.138049;
    matrix_value_and_par[7][2] = -0.000618;
  } else if (beamEnergy == 100) { //100 GeV
    matrix_value_and_par[0][0] = -145.825102;
    matrix_value_and_par[0][1] = 3.099320;
    matrix_value_and_par[0][2] = -0.014369;
    matrix_value_and_par[1][0] = 1.600321;
    matrix_value_and_par[1][1] = 0.055970;
    matrix_value_and_par[1][2] = -0.000407;
    matrix_value_and_par[2][0] = -10.874678;
    matrix_value_and_par[2][1] = 0.193778;
    matrix_value_and_par[2][2] = -0.000883;
    matrix_value_and_par[3][0] = 0.188975;
    matrix_value_and_par[3][1] = 0.000745;
    matrix_value_and_par[3][2] = -0.000008;
    matrix_value_and_par[4][0] = -0.341312;
    matrix_value_and_par[4][1] = 0.005630;
    matrix_value_and_par[4][2] = -0.000026;
    matrix_value_and_par[5][0] = -193.516983;
    matrix_value_and_par[5][1] = 3.532786;
    matrix_value_and_par[5][2] = -0.015695;
    matrix_value_and_par[6][0] = -0.007973;
    matrix_value_and_par[6][1] = 0.000064;
    matrix_value_and_par[6][2] = 0.000000;
    matrix_value_and_par[7][0] = -7.287608;
    matrix_value_and_par[7][1] = 0.118922;
    matrix_value_and_par[7][2] = -0.000468;
  } else if (beamEnergy == 130) { //130 GeV
    matrix_value_and_par[0][0] = -124.028256;
    matrix_value_and_par[0][1] = 2.790119;
    matrix_value_and_par[0][2] = -0.013056;
    matrix_value_and_par[1][0] = 2.985763;
    matrix_value_and_par[1][1] = 0.029403;
    matrix_value_and_par[1][2] = -0.000275;
    matrix_value_and_par[2][0] = -9.921622;
    matrix_value_and_par[2][1] = 0.181781;
    matrix_value_and_par[2][2] = -0.000838;
    matrix_value_and_par[3][0] = 0.322642;
    matrix_value_and_par[3][1] = -0.002093;
    matrix_value_and_par[3][2] = 0.000007;
    matrix_value_and_par[4][0] = -0.291535;
    matrix_value_and_par[4][1] = 0.004711;
    matrix_value_and_par[4][2] = -0.000022;
    matrix_value_and_par[5][0] = -173.506851;
    matrix_value_and_par[5][1] = 3.174727;
    matrix_value_and_par[5][2] = -0.014032;
    matrix_value_and_par[6][0] = -0.006733;
    matrix_value_and_par[6][1] = 0.000052;
    matrix_value_and_par[6][2] = 0.000000;
    matrix_value_and_par[7][0] = -7.030784;
    matrix_value_and_par[7][1] = 0.118149;
    matrix_value_and_par[7][2] = -0.000485;
  } else if (beamEnergy == 41) { //41 GeV
    matrix_value_and_par[0][0] = -174.583113;
    matrix_value_and_par[0][1] = 3.599373;
    matrix_value_and_par[0][2] = -0.016738;
    matrix_value_and_par[1][0] = -2.768064;
    matrix_value_and_par[1][1] = 0.143176;
    matrix_value_and_par[1][2] = -0.000847;
    matrix_value_and_par[2][0] = -12.409090;
    matrix_value_and_par[2][1] = 0.218275;
    matrix_value_and_par[2][2] = -0.000994;
    matrix_value_and_par[3][0] = -0.135417;
    matrix_value_and_par[3][1] = 0.006663;
    matrix_value_and_par[3][2] = -0.000036;
    matrix_value_and_par[4][0] = -0.299041;
    matrix_value_and_par[4][1] = 0.004490;
    matrix_value_and_par[4][2] = -0.000020;
    matrix_value_and_par[5][0] = -175.144897;
    matrix_value_and_par[5][1] = 3.222149;
    matrix_value_and_par[5][2] = -0.014292;
    matrix_value_and_par[6][0] = -0.007376;
    matrix_value_and_par[6][1] = 0.000052;
    matrix_value_and_par[6][2] = 0.000000;
    matrix_value_and_par[7][0] = -8.443702;
    matrix_value_and_par[7][1] = 0.155002;
    matrix_value_and_par[7][2] = -0.000708;
  } else
    throw std::runtime_error(fmt::format("Unknown beamEnergy {}", beamEnergy));

  return (matrix_value_and_par[whichElement][0] + matrix_value_and_par[whichElement][1] * x_L +
          matrix_value_and_par[whichElement][2] * x_L * x_L);
}

} //namespace eicrecon
