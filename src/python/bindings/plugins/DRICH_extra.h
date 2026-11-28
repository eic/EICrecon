// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Geometry-derived configuration for the DRICH wiring (eicrecon.plugins.DRICH): the track
// propagation surfaces and track point cuts of the aerogel and gas radiators, computed by the
// RichGeo service, as DRICH.cc does.

#pragma once

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
