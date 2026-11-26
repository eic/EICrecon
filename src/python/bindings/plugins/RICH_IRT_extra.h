// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Geometry-derived configuration for the RICH-IRT wiring (eicrecon.plugins.RICH_IRT).

#pragma once

#include <JANA/JApplication.h>
#include <pybind11/pybind11.h>

#include <cstdint>
#include <string>

#include "ConfigConversion.h"
#include "detectors/RICH-IRT/RichIrtConfig.h"

inline void eicrecon_python_extra(pybind11::module_& m) {
  m.def(
      "rich_configs",
      [](std::uintptr_t app_pointer, const std::string& rich) -> pybind11::object {
        auto configs =
            eicrecon::GetRichIrtConfigs(reinterpret_cast<JApplication*>(app_pointer), rich);
        if (!configs) {
          return pybind11::none();
        }
        pybind11::dict out;
        out["track"] = eicrecon::python::struct_to_dict(configs->track_cfg);
        out["irt"]   = eicrecon::python::struct_to_dict(configs->config);
        return std::move(out);
      },
      pybind11::arg("app_pointer"), pybind11::arg("rich"),
      "{'track': TrackPropagationConfig, 'irt': IrtConfig} dicts for one RICH, or None if it is "
      "not in the geometry");
}
