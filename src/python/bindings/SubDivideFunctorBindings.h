// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python constructors of the SubDivideCollection functors (used by the binding modules of
// the beam, DIRC, LOWQ2 and particle_flow plugins) (algorithms/meta/SubDivideFunctors.h).
// They return CppFunction values for SubDivideCollectionConfig<T>::function.

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <vector>

#include "ConfigConversion.h"
#include "algorithms/meta/SubDivideCollectionConfig.h"
#include "algorithms/meta/SubDivideFunctors.h"

namespace eicrecon::python {

/// m.<name>(ids, readout, divisions) -> GeometrySplit for collections of T
template <class T> void bind_geometry_split(pybind11::module_& m, const char* name) {
  using Fn = decltype(SubDivideCollectionConfig<T>::function);
  m.def(
      name,
      [](std::vector<std::vector<long int>> ids, std::string readout,
         std::vector<std::string> divisions) {
        return make_cpp_function<Fn>(Fn{GeometrySplit{ids, readout, divisions}});
      },
      pybind11::arg("ids"), pybind11::arg("readout"), pybind11::arg("divisions"),
      "SubDivideCollection function: split by the values of readout fields");
}

} // namespace eicrecon::python
