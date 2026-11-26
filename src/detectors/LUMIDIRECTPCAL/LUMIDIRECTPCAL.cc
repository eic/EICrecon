// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_LUMIDIRECTPCAL: the factory types (and C++ helpers) used
// by the Python wiring of the LUMIDIRECTPCAL plugin, src/python/eicrecon/plugins/LUMIDIRECTPCAL.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_LUMIDIRECTPCAL, m) {
  m.doc() = "Factory types wired by the LUMIDIRECTPCAL plugin";
  py::module_::import("eicrecon._core");
  bind_factory<CalorimeterHitDigi_factory>(m, "CalorimeterHitDigi");
}
