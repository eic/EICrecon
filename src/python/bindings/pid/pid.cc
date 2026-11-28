// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python extension module eicrecon._pid: the particle identification factory types
// (src/factories/pid) for eicrecon.factories.pid, and C++ helpers for config values that
// Python cannot write. Add one bind_factory line per factory type that a wiring uses.

#include <pybind11/pybind11.h>

#include "FactoryType.h"
#include "ConfigConversion.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "factories/pid/IrtCherenkovParticleID_factory.h"
#include "factories/pid/MatchToRICHPID_factory.h"
#include "factories/pid/MergeCherenkovParticleID_factory.h"
#include "factories/pid/MergeTrack_factory.h"
#include "factories/pid/RichTrack_factory.h"
#include "services/geometry/richgeo/ActsGeo.h"
#include "services/geometry/richgeo/RichGeo.h"
#include "services/geometry/richgeo/RichGeo_service.h"
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <JANA/JApplication.h>
#include <string>
#if IRT2_VERSION_MAJOR > 2 || (IRT2_VERSION_MAJOR == 2 && IRT2_VERSION_MINOR >= 2)
#include "RichIrtConfig.h"
#include "factories/pid/IrtInterface_factory.h"
#endif

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

namespace {

// DRICH wiring
void add_drich_helpers(pybind11::module_& m) {
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

// RICH_IRT wiring
#if IRT2_VERSION_MAJOR > 2 || (IRT2_VERSION_MAJOR == 2 && IRT2_VERSION_MINOR >= 2)
void add_rich_irt_helpers(pybind11::module_& m) {
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
#endif

} // namespace

PYBIND11_MODULE(_pid, m) {
  m.doc() = "EICrecon particle identification factory types";
  py::module_::import("eicrecon._core");
  bind_factory<IrtCherenkovParticleID_factory>(m, "IrtCherenkovParticleID");
  bind_factory<MatchToRICHPID_factory>(m, "MatchToRICHPID");
  bind_factory<MergeCherenkovParticleID_factory>(m, "MergeCherenkovParticleID");
  bind_factory<MergeTrack_factory>(m, "MergeTrack");
  bind_factory<RichTrack_factory>(m, "RichTrack");
  add_drich_helpers(m);
#if IRT2_VERSION_MAJOR > 2 || (IRT2_VERSION_MAJOR == 2 && IRT2_VERSION_MINOR >= 2)
  bind_factory<IrtInterface_factory>(m, "IrtInterface");
  add_rich_irt_helpers(m);
#endif
}
