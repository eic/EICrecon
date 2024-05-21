// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#pragma once
#include <edm4eic/unit_system.h>

namespace eicrecon {
    struct TrackerHitReconstructionConfig {
    double timeResolution    = 2000 * edm4eic::unit::ns; // It should be >0, maybe large, to not distort the fitter by default. Choosing 2us for now.
    };
}
