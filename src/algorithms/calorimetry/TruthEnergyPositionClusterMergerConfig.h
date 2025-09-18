// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson 

#pragma once

#include "ClusterTypes.h"

namespace eicrecon {

struct TruthEnergyPositionClusterMergerConfig {

  // cluster type: can be used to flag clusters
  // as being a type specified by the Jug::Reco::ClusterType
  // enum in ClusterTypes.h. This can be useful for, eg.,
  // flagging EMCal vs. HCal clusters in downstream algorithms
  int32_t clusterType{Jug::Reco::ClusterType::kCluster3D};
};

} // namespace eicrecon
