// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#pragma once

namespace eicrecon {

struct ClustersToParticlesConfig {
  /// PDG code of the particle hypothesis (default: 22 = photon)
  int pdgCode = 22;
};

} // namespace eicrecon
