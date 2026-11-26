// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// eicrecon._calorimetry: factory types for the calorimetry chain (prototype).
//
// This file is the only per-factory boilerplate in the Python frontend: one line per factory
// class. Config structs are reflected generically (see FactoryType.h).

#include <pybind11/pybind11.h>

#include "FactoryType.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_calorimetry, m) {
  m.doc() = "EICrecon calorimetry factory types (prototype)";

  // Ensure FactoryType is registered with pybind11 before instances are created
  py::module_::import("eicrecon._core");

  bind_factory<eicrecon::CalorimeterHitDigi_factory>(m, "CalorimeterHitDigi");
  bind_factory<eicrecon::CalorimeterHitReco_factory>(m, "CalorimeterHitReco");
  bind_factory<eicrecon::CalorimeterTruthClustering_factory>(m, "CalorimeterTruthClustering");
  bind_factory<eicrecon::CalorimeterIslandCluster_factory>(m, "CalorimeterIslandCluster");
  bind_factory<eicrecon::CalorimeterClusterRecoCoG_factory>(m, "CalorimeterClusterRecoCoG");
  bind_factory<eicrecon::CalorimeterClusterShape_factory>(m, "CalorimeterClusterShape");
}
