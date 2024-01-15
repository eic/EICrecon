// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

#pragma once

namespace eicrecon {

  struct HEXPLITConfig {
    double                   MIP{0.000472};
    double                   Emin_in_MIPs{0.1};
    double                   tmax{325*dd4hep::ns};
  };

} // eicrecon
