// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

#pragma once

namespace eicrecon {

struct HEXPLITConfig {
  double MIP{472. * dd4hep::keV};
  double Emin_in_MIPs{0.1};
  double delta_in_MIPs{0.01};
  double tmax{325 * dd4hep::ns};
  enum StaggerType {H4=0, H3=1, S2=2} stag_type = H4;
};

} // namespace eicrecon
