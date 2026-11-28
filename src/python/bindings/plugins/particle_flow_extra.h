// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
#pragma once
#include <edm4eic/TrackSegment.h>

#include <cstdint>

#include "SubDivideFunctors_extra.h"

inline void eicrecon_python_extra_particle_flow(pybind11::module_& m) {
  using Fn = decltype(eicrecon::SubDivideCollectionConfig<edm4eic::TrackSegment>::function);
  m.def(
      "TrackPointSystemSplit",
      [](std::vector<std::uint32_t> system_ids) {
        return eicrecon::python::make_cpp_function<Fn>(
            Fn{eicrecon::TrackPointSystemSplit{system_ids}});
      },
      pybind11::arg("system_ids"),
      "SubDivideCollection function: split track projections by the systems of their points");
}
