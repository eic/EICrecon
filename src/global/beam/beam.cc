// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_beam: the factory types (and C++ helpers) used
// by the Python wiring of the beam plugin, src/python/eicrecon/plugins/beam.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <edm4hep/MCParticleCollection.h>
#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/meta/Cloner_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"

using namespace eicrecon;

// Helpers for config values that Python cannot write
#include <edm4hep/MCParticle.h>

#include "SubDivideFunctorBindings.h"

inline void eicrecon_python_extra_beam(pybind11::module_& m) {
  using Fn = decltype(eicrecon::SubDivideCollectionConfig<edm4hep::MCParticle>::function);
  m.def(
      "ValueSplit_GeneratorStatus_PDG",
      [](std::vector<std::vector<int>> values) {
        return eicrecon::python::make_cpp_function<Fn>(
            Fn{eicrecon::ValueSplit<&edm4hep::MCParticle::getGeneratorStatus,
                                    &edm4hep::MCParticle::getPDG>{values}});
      },
      pybind11::arg("values"),
      "SubDivideCollection function: split MCParticles by (generator status, PDG) pairs");
}

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_beam, m) {
  m.doc() = "Factory types wired by the beam plugin";
  py::module_::import("eicrecon._core");
  bind_factory<SubDivideCollection_factory<edm4hep::MCParticle>>(m,
                                                                 "SubDivideCollection_MCParticle");
  bind_factory<CollectionCollector_factory<edm4hep::MCParticle, true>>(
      m, "CollectionCollector_MCParticle_true");
  bind_factory<Cloner_factory<edm4hep::MCParticle>>(m, "Cloner_MCParticle");
  eicrecon_python_extra_beam(m);
}
