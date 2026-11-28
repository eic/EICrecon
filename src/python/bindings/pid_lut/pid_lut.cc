// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python extension module eicrecon._pid_lut: the PID lookup table factory types
// (src/factories/pid_lut) for eicrecon.factories.pid_lut, and C++ helpers for config values that
// Python cannot write. Add one bind_factory line per factory type that a wiring uses.

#include <pybind11/pybind11.h>

#include "FactoryType.h"
#include "factories/pid_lut/PIDLookup_factory.h"
#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4eic/MCRecoParticleLinkCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <podio/detail/Link.h>
#include <string>

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_pid_lut, m) {
  m.doc() = "EICrecon PID lookup table factory types";
  py::module_::import("eicrecon._core");
  bind_factory<PIDLookup_factory>(m, "PIDLookup");
}
