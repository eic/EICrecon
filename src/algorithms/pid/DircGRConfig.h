// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#pragma once

#include <DD4hep/DD4hepUnits.h>
#include <edm4eic/unit_system.h>
#include <string>
#include <vector>

namespace eicrecon {

struct DircGRConfig {
  // Calibration inputs: directory with lut_bar{0..nBar-1}.root and the
  // chromatic correction file. Default: fetched with the geometry
  // (epic compact/calibrations.xml, files in eic/epic-data). Missing files:
  // warning, no PID (no LUT) / no chromatic correction (no chrcorr).
  std::string lutPath     = "calibrations/dircgr";
  std::string chrcorrFile = "calibrations/dircgr/chrcorr.root";
  // chrcorr graph set "<species>_<momBin>"; 0_0 = pions, 3 GeV
  int chrcorrSpecies = 0;
  int chrcorrMomBin  = 0;
  // Calibration dump (off if empty): one TTree row per used photon path
  // (track polar angle, zdir, dt/L, thetaC, wavelength) for the chromatic
  // correction fit (chrcorrfit.C). Rows are written only when no chrcorr is
  // loaded, so thetaC is the uncorrected angle.
  std::string calibFile = "";

  // PID hypotheses (PDG codes), order defines hypothesis index
  std::vector<int> hypotheses{211, 321, 2212};

  // readout whose cellID encodes module/bar (MCP and quartz hits share it)
  std::string readout = "DIRCHits";

  // ---- geometry constants (TODO: from DD4hep constants) ----
  double zReadout = -3040.074 * edm4eic::unit::mm; // readout (MCP) plane z (hit z)
  // LUT time origin: the LUT photons start 2 mm inside the prism-side bar
  // end face (z = -2727.55); LUT times are measured from there
  double zBarMin = -2725.55 * edm4eic::unit::mm;
  double zBarMax = 1832.525 * edm4eic::unit::mm; // mirror-side bar end face
  // photon group velocity in the bar
  double vBar = 199.5 * edm4eic::unit::mm / edm4eic::unit::ns;
  // LUT pixel grid = the DIRCHits MCP readout grid (3.3 mm cells, cell (0,0)
  // centred on the MCP plane): nPixX x nPixY = 107 (cell y) x 73 (cell x),
  // LUT pixel (ix, iy) = ((nPixX-1)/2 - cellY, cellX + (nPixY-1)/2)
  int nPixX = 107;
  int nPixY = 73;
  int nBar  = 10;
  int nBox  = 12; // bar boxes, box k centred at phi = k * 360/nBox deg

  // TDC least significant bit of the raw hits [ns]: hit time =
  // (timeStamp + 0.5) * timeResolution. Must equal DircGRdig's.
  double timeResolution = 0.020 * edm4eic::unit::ns;

  // ---- per-track gates ----
  double refractiveIndex = 1.4738; // fused silica, for expected thetaC
  // LUT paths are kept only if cos(path polar angle) is inside the beta = 1
  // Cherenkov cone around the track direction, widened by this margin
  double cosAngMargin = 0.005;
  // photon direction "1" (to prism at creation) is not used for tracks with
  // detector polar angle <= dir1MinTheta; direction "2" (to mirror) not
  // for >= dir2MaxTheta
  double dir1MinTheta = 85.0 * dd4hep::deg;
  double dir2MaxTheta = 95.0 * dd4hep::deg;
  // near 90 deg (between the two above) the earliest allowed photon time is
  // moved earlier by this amount
  double tbegShift90 = 0.5 * edm4eic::unit::ns;
  // tight time gate |t_hit - t_exp| < half width, linear in t_exp:
  // hw0 at t_exp = tbegOP, hw10 at t_exp = tbegOP + 10 ns
  double tightGateHalfWidth0  = 1.0 * edm4eic::unit::ns;
  double tightGateHalfWidth10 = 1.0 * edm4eic::unit::ns;

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
