// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_BHCAL: the factory types (and C++ helpers) used
// by the Python wiring of the BHCAL plugin, src/python/eicrecon/plugins/BHCAL.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterHitsMerger_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_BHCAL, m) {
  m.doc() = "Factory types wired by the BHCAL plugin";
  py::module_::import("eicrecon._core");
  bind_factory<CalorimeterHitDigi_factory>(m, "CalorimeterHitDigi");
  bind_factory<CalorimeterHitReco_factory>(m, "CalorimeterHitReco");
  bind_factory<CalorimeterHitsMerger_factory>(m, "CalorimeterHitsMerger");
  bind_factory<CalorimeterTruthClustering_factory>(m, "CalorimeterTruthClustering");
  bind_factory<CalorimeterIslandCluster_factory>(m, "CalorimeterIslandCluster");
  bind_factory<CalorimeterClusterRecoCoG_factory>(m, "CalorimeterClusterRecoCoG");
  bind_factory<CalorimeterClusterShape_factory>(m, "CalorimeterClusterShape");
}
