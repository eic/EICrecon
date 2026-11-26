// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
#pragma once
#include <edm4hep/MCParticle.h>

#include "SubDivideFunctors_extra.h"

inline void eicrecon_python_extra(pybind11::module_& m) {
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
