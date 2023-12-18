// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

#pragma once

namespace eicrecon {

  struct HEXPLITConfig {

    double                   layer_spacing{24.9};
    double                   side_length{31.3};
    double                   MIP{0.000472};
    double                   Emin_in_MIPs{0.1};
    double                   tmax{325*dd4hep::ns};
    //conversion from local to global coordinates.  translate first, then rotate
    double                   trans_x{0};
    double                   trans_y{0};
    double                   trans_z{0};
    double                   rot_x{0};
    double    	      	     rot_y{0};
    double    	      	     rot_z{0};
  };

} // eicrecon
