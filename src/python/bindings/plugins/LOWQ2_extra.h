// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
#pragma once
#include <edm4eic/TrackerHit.h>

#include "SubDivideFunctors_extra.h"

inline void eicrecon_python_extra_LOWQ2(pybind11::module_& m) {
  eicrecon::python::bind_geometry_split<edm4eic::TrackerHit>(m, "GeometrySplit_TrackerHit");
}
