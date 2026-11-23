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
};

} // namespace eicrecon
