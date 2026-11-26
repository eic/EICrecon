// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_DRICH: the factory types (and C++ helpers) used
// by the Python wiring of the DRICH plugin, src/python/eicrecon/plugins/DRICH.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/digi/PhotoMultiplierHitDigi_factory.h"
#include "factories/pid/IrtCherenkovParticleID_factory.h"
#include "factories/pid/MergeCherenkovParticleID_factory.h"
#include "factories/pid/MergeTrack_factory.h"
#include "factories/pid/RichTrack_factory.h"

using namespace eicrecon;

// Helpers for config values that Python cannot write
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <JANA/JApplication.h>
#include <pybind11/pybind11.h>

#include "ConfigConversion.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "services/geometry/richgeo/ActsGeo.h"
#include "services/geometry/richgeo/RichGeo.h"
#include "services/geometry/richgeo/RichGeo_service.h"

inline void eicrecon_python_extra_DRICH(pybind11::module_& m) {
  m.def(
      "track_propagation_configs",
      [](const eicrecon::python::AppHandle& handle) {
        auto* app = handle.app;
        eicrecon::TrackPropagationConfig aerogel_track_cfg;
        eicrecon::TrackPropagationConfig gas_track_cfg;
        auto richGeoSvc       = app->GetService<RichGeo_service>();
        const auto* dd4hepGeo = richGeoSvc->GetDD4hepGeo();
        if (dd4hepGeo->world().children().contains("DRICH")) {
          const auto* actsGeo          = richGeoSvc->GetActsGeo("DRICH");
          auto aerogel_tracking_planes = actsGeo->TrackingPlanes(richgeo::kAerogel, 5);
          auto aerogel_track_point_cut = actsGeo->TrackPointCut(richgeo::kAerogel);
          auto gas_tracking_planes     = actsGeo->TrackingPlanes(richgeo::kGas, 10);
          auto gas_track_point_cut     = actsGeo->TrackPointCut(richgeo::kGas);
          auto filter_surface          = gas_tracking_planes.back();
          aerogel_track_cfg.filter_surfaces.push_back(filter_surface);
          aerogel_track_cfg.target_surfaces = aerogel_tracking_planes;
          aerogel_track_cfg.track_point_cut = aerogel_track_point_cut;
          gas_track_cfg.filter_surfaces.push_back(filter_surface);
          gas_track_cfg.target_surfaces = gas_tracking_planes;
          gas_track_cfg.track_point_cut = gas_track_point_cut;
        }
        pybind11::dict out;
        out["aerogel"] = eicrecon::python::struct_to_dict(aerogel_track_cfg);
        out["gas"]     = eicrecon::python::struct_to_dict(gas_track_cfg);
        return out;
      },
      pybind11::arg("app"),
      "TrackPropagationConfig dicts {'aerogel': ..., 'gas': ...} from the RichGeo service");
}

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_DRICH, m) {
  m.doc() = "Factory types wired by the DRICH plugin";
  py::module_::import("eicrecon._core");
  bind_factory<PhotoMultiplierHitDigi_factory>(m, "PhotoMultiplierHitDigi");
  bind_factory<RichTrack_factory>(m, "RichTrack");
  bind_factory<MergeTrack_factory>(m, "MergeTrack");
  bind_factory<IrtCherenkovParticleID_factory>(m, "IrtCherenkovParticleID");
  bind_factory<MergeCherenkovParticleID_factory>(m, "MergeCherenkovParticleID");
  eicrecon_python_extra_DRICH(m);
}
