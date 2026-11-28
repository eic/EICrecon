// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python extension module eicrecon._particle_flow: the particle flow factory types
// (src/factories/particle_flow) for eicrecon.factories.particle_flow, and C++ helpers for config values that
// Python cannot write. Add one bind_factory line per factory type that a wiring uses.

#include <pybind11/pybind11.h>

#include "FactoryType.h"
#include "SubDivideFunctorBindings.h"
#include "factories/meta/ObjectIDAccessors.h"
#include "factories/particle_flow/CaloRemnantCombiner_factory.h"
#include "factories/particle_flow/ChargedCandidateMaker_factory.h"
#include "factories/particle_flow/TrackClusterMergeSplitter_factory.h"
#include "factories/particle_flow/TrackClusterSubtractor_factory.h"
#include "factories/particle_flow/TrackProtoClusterMatchPromoter_factory.h"
#include <cstdint>
#include <edm4eic/Cluster.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/MCRecoParticleLinkCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/TrackClusterMatch.h>
#include <edm4eic/TrackSegment.h>
#include <podio/detail/Link.h>
#include <string>

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_particle_flow, m) {
  m.doc() = "EICrecon particle flow factory types";
  py::module_::import("eicrecon._core");
  bind_factory<CaloRemnantCombiner_factory>(m, "CaloRemnantCombiner");
  bind_factory<ChargedCandidateMaker_factory>(m, "ChargedCandidateMaker");
  bind_factory<TrackClusterMergeSplitter_factory>(m, "TrackClusterMergeSplitter");
  bind_factory<TrackClusterSubtractor_factory>(m, "TrackClusterSubtractor");
  bind_factory<TrackProtoClusterMatchPromoter_factory>(m, "TrackProtoClusterMatchPromoter");
}
