// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_BVTX: the factory types (and C++ helpers) used
// by the Python wiring of the BVTX plugin, src/python/eicrecon/plugins/BVTX.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <edm4eic/RawTrackerHit.h>
#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/digi/RandomNoisePixel_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_BVTX, m) {
  m.doc() = "Factory types wired by the BVTX plugin";
  py::module_::import("eicrecon._core");
  bind_factory<SiliconTrackerDigi_factory>(m, "SiliconTrackerDigi");
  bind_factory<RandomNoisePixel_factory>(m, "RandomNoisePixel");
  bind_factory<CollectionCollector_factory<edm4eic::RawTrackerHit>>(
      m, "CollectionCollector_RawTrackerHit");
  bind_factory<TrackerHitReconstruction_factory>(m, "TrackerHitReconstruction");
}
