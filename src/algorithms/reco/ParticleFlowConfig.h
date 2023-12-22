// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#pragma once

#include <string>
#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

  struct ParticleFlowConfig {

    // global parameters
    uint8_t     flowAlgo    = 0;                 // choice of particle flow algorithm
    std::string ecalDetName = "EcalBarrelScFi";  // name of ecal being used
    std::string hcalDetName = "HcalBarrel";      // name of hcal being used

    // general kinematic parameters
    float minTrkMomentum = 0.1 * dd4hep::GeV;  // minimum p of used tracks
    float minECalEnergy  = 0.1 * dd4hep::GeV;  // minimum energy of used ecal clusters
    float minHCalEnergy  = 0.1 * dd4hep::GeV;  // minimum energy of used hcal clusters

    // PFAlpha parameters
    float ecalSumRadius = 1.0;  // radius of cone to sum energy from ecal clusters
    float hcalSumRadius = 1.0;  // radius of cone to sum energy from hcal clusters
    float ecalFracSub   = 1.0;  // fraction of track energy to subtract from ecal sum
    float hcalFracSub   = 1.0;  // fraction of track energy to subtract from hcal sum

  };

}  // end eicrecon namespace
