// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_DIRC: the factory types (and C++ helpers) used
// by the Python wiring of the DIRC plugin, src/python/eicrecon/plugins/DIRC.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <edm4hep/SimTrackerHit.h>
#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/digi/PhotoMultiplierHitDigi_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"

using namespace eicrecon;

// Helpers for config values that Python cannot write

#include "SubDivideFunctorBindings.h"

inline void eicrecon_python_extra_DIRC(pybind11::module_& m) {
  eicrecon::python::bind_geometry_split<edm4hep::SimTrackerHit>(m, "GeometrySplit_SimTrackerHit");
}

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_DIRC, m) {
  m.doc() = "Factory types wired by the DIRC plugin";
  py::module_::import("eicrecon._core");
  bind_factory<SubDivideCollection_factory<edm4hep::SimTrackerHit, true>>(
      m, "SubDivideCollection_SimTrackerHit_true");
  bind_factory<CollectionCollector_factory<edm4hep::SimTrackerHit, true>>(
      m, "CollectionCollector_SimTrackerHit_true");
  bind_factory<PhotoMultiplierHitDigi_factory>(m, "PhotoMultiplierHitDigi");
  eicrecon_python_extra_DIRC(m);
}
