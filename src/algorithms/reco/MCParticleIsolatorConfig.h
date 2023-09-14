// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Simon Gardner

#pragma once

namespace eicrecon {

  // Defualt values for beam electron
  struct MCParticleIsolatorConfig {
    int  genStatus{4};
    int  pdg{11};
    bool abovePDG{false};
  };

} // eicrecon
