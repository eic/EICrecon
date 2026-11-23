// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 hpDIRC GR effort

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

struct DircGRConfig {
  // Photon input: "sim" = MCP SimTrackerHits (truth position/time, as the
  // standalone dircGR), "raw" = digitized RawTrackerHits (cellID + TDC).
  std::string photonInput = "sim";

  // Calibration inputs. lutPath = directory with lut_bar{0..nBar-1}.root.
  // Empty lutPath: no LUT is loaded (skeleton / dry runs only).
  std::string lutPath;
  std::string chrcorrFile;
  // chrcorr graph set "<species>_<momBin>"; old default 0_0 = pions, 3 GeV
  int chrcorrSpecies = 0;
  int chrcorrMomBin  = 0;

  // PID hypotheses (PDG codes), order defines hypothesis index
  std::vector<int> hypotheses{211, 321, 2212};

  // readout whose cellID encodes module/bar (MCP and quartz hits share it)
  std::string readout = "DIRCHits";

  // ---- geometry constants, from the old DIRCgr.h / standalone dircGR ----
  double zReadout = -3040.074; // [mm] readout (MCP) plane z
  double zBarMin  = -2729.075; // [mm]
  double zBarMax  = 1831.025;  // [mm]
  double vBar     = 199.5;     // [mm/ns] photon group velocity in the bar
  // Pixel grid of the LUT: 108 x 73 cells of 3.30 mm over the box-local
  // readout plane. CIRCLE BACK: DIRCHits MCP segmentation is a 3.0 mm grid;
  // the LUT index grid (locX/locY limits / 3.3) must match LUT generation.
  double pixelSize = 3.30;    // [mm]
  double locXmin   = -178.20; // [mm]
  double locXmax   = 178.20;  // [mm]
  double locYmin   = 739.55;  // [mm]
  double locYmax   = 980.45;  // [mm]
  int nPixX        = 108;
  int nPixY        = 73;
  int nBar         = 10;
  int nBox         = 12; // bar boxes, box k centred at phi = k * 360/nBox deg

  // ---- per-track gates (old DIRCgr::Process / OPTimeCut*) ----
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
  // hw0 at t_exp = tbegOP, hw10 at t_exp = tbegOP + 10 ns (old: 1.0, 1.0)
  double tightGateHalfWidth0  = 1.0;
  double tightGateHalfWidth10 = 1.0;
  double crudeGateHalfWidth   = 2.0; // [ns], diagnostics only

  // ---- per-track thetaC estimator (old PID#2, "thetaCestPeak") ----
  // per-photon-path thetaC histogram [mrad], 1 mrad bins from 0; estimate =
  // mean of the bins within +-peakHalfWindow bins of the maximum bin
  int thetaCHistBins  = 875;
  double thetaCHistMax = 875.0; // [mrad]
  int peakHalfWindow  = 20;     // [bins]

  // Old code bug: in the 8-fold ambiguity search the stored best
  // difference per hypothesis (used to pick the best hypothesis of a
  // photon path) was |theta_chr - thetaC_exp - chrcorr| while the search
  // compares |theta_chr - thetaC_exp|. false = fixed (default);
  // true = reproduce the old code exactly (regression checks only).
  bool legacyBestHypDiff = false;
};

} // namespace eicrecon
