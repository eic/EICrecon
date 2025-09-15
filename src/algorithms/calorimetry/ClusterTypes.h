// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten
#pragma once

namespace Jug::Reco {
enum ClusterType : int32_t { kCluster2D = 0, kCluster3D = 1, kClusterSlice = 2, kClusterEMCal = 3, kClusterHCal = 4 };
}
