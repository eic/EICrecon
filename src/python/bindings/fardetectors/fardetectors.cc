// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python extension module eicrecon._fardetectors: the far-forward detector factory types
// (src/factories/fardetectors) for eicrecon.factories.fardetectors, and C++ helpers for config values that
// Python cannot write. Add one bind_factory line per factory type that a wiring uses.

#include <pybind11/pybind11.h>
#include <format> // IWYU pragma: keep
#include <string>

#include "FactoryType.h"
#include "factories/fardetectors/FarDetectorLinearTracking_factory.h"
#include "factories/fardetectors/FarDetectorTrackerCluster_factory.h"
#include "factories/fardetectors/FarDetectorTransportationPostML_factory.h"
#include "factories/fardetectors/FarDetectorTransportationPreML_factory.h"
#include "factories/fardetectors/MatrixTransferStatic_factory.h"
#include "factories/fardetectors/PolynomialMatrixReconstruction_factory.h"

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_fardetectors, m) {
  m.doc() = "EICrecon far-forward detector factory types";
  py::module_::import("eicrecon._core");
  bind_factory<FarDetectorLinearTracking_factory>(m, "FarDetectorLinearTracking");
  bind_factory<FarDetectorTrackerCluster_factory>(m, "FarDetectorTrackerCluster");
  bind_factory<FarDetectorTransportationPostML_factory>(m, "FarDetectorTransportationPostML");
  bind_factory<FarDetectorTransportationPreML_factory>(m, "FarDetectorTransportationPreML");
  bind_factory<MatrixTransferStatic_factory>(m, "MatrixTransferStatic");
  bind_factory<PolynomialMatrixReconstruction_factory>(m, "PolynomialMatrixReconstruction");
}
