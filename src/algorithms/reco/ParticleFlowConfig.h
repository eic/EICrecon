// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#pragma once

namespace eicrecon {

  struct ParticleFlowConfig {

    // Note: the vector elements correspond
    // to different eta regions
    //   [0] = negative
    //   [1] = central
    //   [2] = positive
    std::vector<int>   flowAlgo      = {0, 0, 0};        // choice of particle flow algorithm
    std::vector<float> ecalSumRadius = {1.0, 1.0, 1.0};  // radius of cone to sum energy from ecal clusters
    std::vector<float> hcalSumRadius = {1.0, 1.0, 1.0};  // radius of cone to sum energy from hcal clusters
    std::vector<float> ecalFracSub   = {1.0, 1.0, 1.0};  // fraction of track energy to subtract from ecal sum
    std::vector<float> hcalFracSub   = {1.0, 1.0, 1.0};  // fraction of track energy to subtract from hcal sum

  };

}  // end eicrecon namespace
