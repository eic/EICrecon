// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson, Zhongling Ji

#pragma once

#include <fastjet/config.h>
#include <fastjet/JetDefinition.hh>
#include <fastjet/AreaDefinition.hh>
#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

  struct JetReconstructionConfig {

    // input parameters
    double minCstPt = 0.2  * dd4hep::GeV;  // minimum pT of objects fed to cluster sequence
    double maxCstPt = 100. * dd4hep::GeV;  // maximum pT of objects fed to clsuter sequence

    // jet parameters
    float  rJet     = 1.0;                // jet resolution  parameter
    double minJetPt = 1.0 * dd4hep::GeV;  // minimum jet pT

    // area parameters
    int    numGhostRepeat = 1;      // number of times a ghost is reused per grid site
    double ghostMaxRap    = 3.5;    // maximum rapidity of ghosts
    double ghostArea      = 0.001;  // area per ghost

    // fastjet options
    fastjet::JetAlgorithm        jetAlgo      = fastjet::antikt_algorithm;               // jet finding algorithm
    fastjet::RecombinationScheme recombScheme = fastjet::RecombinationScheme::E_scheme;  // particle recombination scheme
    fastjet::AreaType            areType      = fastjet::AreaType::active_area;          // type of area calculated

  };

}  // end eicrecon namespace
