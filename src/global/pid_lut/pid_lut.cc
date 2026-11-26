// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_global_pid_lut: the factory types (and C++ helpers) used
// by the Python wiring of the global_pid_lut plugin, src/python/eicrecon/plugins/global_pid_lut.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4eic/MCRecoParticleLinkCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <podio/detail/Link.h>
#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/pid_lut/PIDLookup_factory.h"

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_global_pid_lut, m) {
  m.doc() = "Factory types wired by the global_pid_lut plugin";
  py::module_::import("eicrecon._core");
  bind_factory<PIDLookup_factory>(m, "PIDLookup");
  bind_factory<CollectionCollector_factory<edm4eic::ReconstructedParticle, true>>(
      m, "CollectionCollector_ReconstructedParticle_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoParticleAssociation, true>>(
      m, "CollectionCollector_MCRecoParticleAssociation_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoParticleLink, true>>(
      m, "CollectionCollector_MCRecoParticleLink_true");
}
