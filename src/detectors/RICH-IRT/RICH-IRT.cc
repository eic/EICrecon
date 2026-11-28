// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_RICH_IRT: the factory types (and C++ helpers) used
// by the Python wiring of the RICH-IRT plugin, src/python/eicrecon/plugins/RICH_IRT.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <pybind11/pybind11.h>

#include "FactoryType.h"
#if IRT2_VERSION_MAJOR > 2 || (IRT2_VERSION_MAJOR == 2 && IRT2_VERSION_MINOR >= 2)
#include <string>

#include "factories/tracking/TrackPropagation_factory.h"
#include "global/pid/IrtInterface_factory.h"

using namespace eicrecon;

// Helpers for config values that Python cannot write
#include <JANA/JApplication.h>

#include "ConfigConversion.h"
#include "detectors/RICH-IRT/RichIrtConfig.h"

inline void eicrecon_python_extra_RICH_IRT(pybind11::module_& m) {
  m.def(
      "rich_configs",
      [](const eicrecon::python::AppHandle& handle, const std::string& rich) -> pybind11::object {
        auto configs = eicrecon::GetRichIrtConfigs(handle.app, rich);
        if (!configs) {
          return pybind11::none();
        }
        pybind11::dict out;
        out["track"] = eicrecon::python::struct_to_dict(configs->track_cfg);
        out["irt"]   = eicrecon::python::struct_to_dict(configs->config);
        return std::move(out);
      },
      pybind11::arg("app"), pybind11::arg("rich"),
      "{'track': TrackPropagationConfig, 'irt': IrtConfig} dicts for one RICH, or None if it is "
      "not in the geometry");
}

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_RICH_IRT, m) {
  m.doc() = "Factory types wired by the RICH-IRT plugin";
  py::module_::import("eicrecon._core");
  bind_factory<TrackPropagation_factory>(m, "TrackPropagation");
  bind_factory<IrtInterface_factory>(m, "IrtInterface");
  eicrecon_python_extra_RICH_IRT(m);
}
#endif
