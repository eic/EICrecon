// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python extension module eicrecon._calorimetry: the calorimetry factory types
// (src/factories/calorimetry) for eicrecon.factories.calorimetry, and C++ helpers for config values that
// Python cannot write. Add one bind_factory line per factory type that a wiring uses.

#include <pybind11/pybind11.h>

#include "FactoryType.h"
#include "SubDivideFunctorBindings.h"
#include "factories/calorimetry/CalorimeterCALOROCRecoLegacy_factory.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterHitsMerger_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterParticleIDPostML_factory.h"
#include "factories/calorimetry/CalorimeterParticleIDPreML_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/EdepToNpeConversion_factory.h"
#include "factories/calorimetry/EnergyPositionClusterMerger_factory.h"
#include "factories/calorimetry/HEXPLIT_factory.h"
#include "factories/calorimetry/ImagingClusterReco_factory.h"
#include "factories/calorimetry/ImagingTopoCluster_factory.h"
#include "factories/calorimetry/SimCalorimeterHitProcessor_factory.h"
#include "factories/calorimetry/TruthEnergyPositionClusterMerger_factory.h"
#include "factories/meta/ObjectIDAccessors.h"
#include <cstdint>
#include <edm4eic/Cluster.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/MCRecoParticleLinkCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/TrackClusterMatch.h>
#include <edm4eic/TrackSegment.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <podio/detail/Link.h>
#include <string>

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_calorimetry, m) {
  m.doc() = "EICrecon calorimetry factory types";
  py::module_::import("eicrecon._core");
  bind_factory<CalorimeterCALOROCRecoLegacy_factory>(m, "CalorimeterCALOROCRecoLegacy");
  bind_factory<CalorimeterClusterRecoCoG_factory>(m, "CalorimeterClusterRecoCoG");
  bind_factory<CalorimeterClusterShape_factory>(m, "CalorimeterClusterShape");
  bind_factory<CalorimeterHitDigi_factory>(m, "CalorimeterHitDigi");
  bind_factory<CalorimeterHitReco_factory>(m, "CalorimeterHitReco");
  bind_factory<CalorimeterHitsMerger_factory>(m, "CalorimeterHitsMerger");
  bind_factory<CalorimeterIslandCluster_factory>(m, "CalorimeterIslandCluster");
  bind_factory<CalorimeterParticleIDPostML_factory>(m, "CalorimeterParticleIDPostML");
  bind_factory<CalorimeterParticleIDPreML_factory>(m, "CalorimeterParticleIDPreML");
  bind_factory<CalorimeterTruthClustering_factory>(m, "CalorimeterTruthClustering");
  bind_factory<EdepToNpeConversion_factory>(m, "EdepToNpeConversion");
  bind_factory<EnergyPositionClusterMerger_factory>(m, "EnergyPositionClusterMerger");
  bind_factory<HEXPLIT_factory>(m, "HEXPLIT");
  bind_factory<ImagingClusterReco_factory>(m, "ImagingClusterReco");
  bind_factory<ImagingTopoCluster_factory>(m, "ImagingTopoCluster");
  bind_factory<SimCalorimeterHitProcessor_factory>(m, "SimCalorimeterHitProcessor");
  bind_factory<TruthEnergyPositionClusterMerger_factory>(m, "TruthEnergyPositionClusterMerger");
}
