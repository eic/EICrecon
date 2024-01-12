// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

#pragma once

namespace eicrecon {

  struct LogWeightRecoConfig {
    // strawman energy reconstruction
    double                   sampling_fraction{0.0203};
    //parameters to determine the cut-off parameter w0
    // as a function of energy based on an optimized
    double                   E0{50};
    double                   w0_a{5.0};
    double                   w0_b{0.65};
    double                   w0_c{0.31};

  };

} // eicrecon
