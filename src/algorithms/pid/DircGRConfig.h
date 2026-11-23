// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

struct DircGRConfig {
  // Calibration inputs. lutPath = directory with lut_bar{0..nBar-1}.root.
  // Empty lutPath: no LUT is loaded (skeleton / dry runs only).
  std::string lutPath;
  std::string chrcorrFile;
  // chrcorr graph set "<species>_<momBin>"; 0_0 = pions, 3 GeV
  int chrcorrSpecies = 0;
  int chrcorrMomBin  = 0;

  // PID hypotheses (PDG codes), order defines hypothesis index
  std::vector<int> hypotheses{211, 321, 2212};

  // readout whose cellID encodes module/bar (MCP and quartz hits share it)
  std::string readout = "DIRCHits";

  // ---- geometry constants (TODO: from DD4hep constants) ----
  double zReadout = -3040.074; // [mm] readout (MCP) plane z
  double zBarMin  = -2729.075; // [mm]
  double zBarMax  = 1831.025;  // [mm]
  double vBar     = 199.5;     // [mm/ns] photon group velocity in the bar
  // INTERNAL pixel grid over the box-local readout plane (locx = -y',
  // locy = x' after rotating the box onto box 0): 108 x 73 square pixels of
  // 3.30 mm covering the whole prism face. NOT the DIRCHits cellID grid
  // (3.0 mm) and not the real HRPPD layout (not in the geometry yet). It
  // MUST equal the grid the LUT was generated with (dircGR/run/lutgen.C).
  double pixelSize = 3.30;    // [mm]
  double locXmin   = -178.20; // [mm]
  double locXmax   = 178.20;  // [mm]
  double locYmin   = 739.55;  // [mm]
  double locYmax   = 980.45;  // [mm]
  int nPixX        = 108;
  int nPixY        = 73;
  int nBar         = 10;
  int nBox         = 12; // bar boxes, box k centred at phi = k * 360/nBox deg

  // ---- per-track gates ----
  double refractiveIndex = 1.4738; // fused silica, for expected thetaC
  // photon direction "1" (to prism at creation) is not used for tracks with
  // detector polar angle <= dir1MinThetaDeg; direction "2" (to mirror) not
  // for >= dir2MaxThetaDeg
  double dir1MinThetaDeg = 85.0;
  double dir2MaxThetaDeg = 95.0;
  // near 90 deg (between the two above) the earliest allowed photon time is
  // moved earlier by this amount [ns]
  double tbegShift90 = 0.5;
  // tight time gate |t_hit - t_exp| < half width [ns], linear in t_exp:
  // hw0 at t_exp = tbegOP, hw10 at t_exp = tbegOP + 10 ns
  double tightGateHalfWidth0  = 1.0;
  double tightGateHalfWidth10 = 1.0;

  // ---- per-track thetaC estimator ----
  // per-photon-path thetaC histogram [mrad], 1 mrad bins from 0; estimate =
  // mean of the bins within +-peakHalfWindow bins of the maximum bin
  int thetaCHistBins   = 875;
  double thetaCHistMax = 875.0; // [mrad]
  int peakHalfWindow   = 20;    // [bins]

  // ---- per-track hypothesis likelihood ----
  // lnL_h = -0.5 ((thetaC - thetaC_exp,h) / sigma)^2 [weight of hyp h],
  // sigma^2 = spr^2 / N_photons + sigmaExt^2, all in mrad
  // provisional values from the S1 fit (3 GeV pi, theta 25-160, truth
  // segments, sim photons): SPR is theta dependent (4-11 mrad)
  double spr      = 6.1;  // [mrad] per-photon (per-path) resolution
  double sigmaExt = 0.86; // [mrad] track-level term
  int minPhotons  = 3;    // fewer photons used -> no PID (npe 0, weights 0)
};

} // namespace eicrecon
