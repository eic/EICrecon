// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
#pragma once
#include <edm4hep/SimTrackerHit.h>

#include "SubDivideFunctors_extra.h"

inline void eicrecon_python_extra_DIRC(pybind11::module_& m) {
  eicrecon::python::bind_geometry_split<edm4hep::SimTrackerHit>(m, "GeometrySplit_SimTrackerHit");
}
