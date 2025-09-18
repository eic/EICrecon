// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "ClusterTypes.h"

namespace eicrecon {

struct EnergyPositionClusterMergerConfig {

  double energyRelTolerance{0.5};
  double phiTolerance{0.1};
  double etaTolerance{0.2};

  // cluster type: can be used to flag clusters
  // as being a type specified by the Jug::Reco::ClusterType
  // enum in ClusterTypes.h. This can be useful for, eg.,
  // flagging EMCal vs. HCal clusters in downstream algorithms
  int32_t clusterType{Jug::Reco::ClusterType::kCluster3D};
};

} // namespace eicrecon
