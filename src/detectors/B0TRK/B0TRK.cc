// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_B0TRK: the factory types (and C++ helpers) used
// by the Python wiring of the B0TRK plugin, src/python/eicrecon/plugins/B0TRK.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_B0TRK, m) {
  m.doc() = "Factory types wired by the B0TRK plugin";
  py::module_::import("eicrecon._core");
  bind_factory<SiliconTrackerDigi_factory>(m, "SiliconTrackerDigi");
  bind_factory<TrackerHitReconstruction_factory>(m, "TrackerHitReconstruction");
}
