// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_LUMISPECTRK: the factory types (and C++ helpers) used
// by the Python wiring of the LUMISPECTRK plugin, src/python/eicrecon/plugins/LUMISPECTRK.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_LUMISPECTRK, m) {
  m.doc() = "Factory types wired by the LUMISPECTRK plugin";
  py::module_::import("eicrecon._core");
  bind_factory<SiliconTrackerDigi_factory>(m, "SiliconTrackerDigi");
}
