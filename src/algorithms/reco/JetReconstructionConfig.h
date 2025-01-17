// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson, Zhongling Ji, John Lajoie

#pragma once

#include <string>
#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct JetReconstructionConfig {

  float rJet                 = 1.0;                // jet resolution  parameter
  float pJet                 = -1.0;               // exponent for generalized kt algorithms
  double minCstPt            = 0.2 * dd4hep::GeV;  // minimum pT of objects fed to cluster sequence
  double maxCstPt            = 100. * dd4hep::GeV; // maximum pT of objects fed to clsuter sequence
  double minJetPt            = 1.0 * dd4hep::GeV;  // minimum jet pT
  double ghostMaxRap         = 3.5;                // maximum rapidity of ghosts
  double ghostArea           = 0.01;               // area per ghost
  int numGhostRepeat         = 1; // number of times a ghost is reused per grid site
  std::string jetAlgo        = "antikt_algorithm"; // jet finding algorithm
  std::string recombScheme   = "E_scheme";         // particle recombination scheme
  std::string areaType       = "active_area";      // type of area calculated
  std::string jetContribAlgo = "Centauro";         // contributed algorithm name
};

} // namespace eicrecon
