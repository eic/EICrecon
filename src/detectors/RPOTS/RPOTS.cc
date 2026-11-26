// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_RPOTS: the factory types (and C++ helpers) used
// by the Python wiring of the RPOTS plugin, src/python/eicrecon/plugins/RPOTS.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/fardetectors/MatrixTransferStatic_factory.h"
#include "factories/fardetectors/PolynomialMatrixReconstruction_factory.h"
#include "factories/reco/TransformLocalFFFrameToLabFrame_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_RPOTS, m) {
  m.doc() = "Factory types wired by the RPOTS plugin";
  py::module_::import("eicrecon._core");
  bind_factory<SiliconTrackerDigi_factory>(m, "SiliconTrackerDigi");
  bind_factory<TrackerHitReconstruction_factory>(m, "TrackerHitReconstruction");
  bind_factory<MatrixTransferStatic_factory>(m, "MatrixTransferStatic");
  bind_factory<PolynomialMatrixReconstruction_factory>(m, "PolynomialMatrixReconstruction");
  bind_factory<TransformLocalFFFrameToLabFrame_factory>(m, "TransformLocalFFFrameToLabFrame");
}
