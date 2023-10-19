// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#pragma once

namespace eicrecon {

  struct ParticleFlowConfig {

    int flowAlgo  = 0;   // choice of particle flow algorithm
    int mergeAlgo = 0;   // choice of cluster-merging algorithm

  };

}  // end eicrecon namespace
