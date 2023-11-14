// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "RichTrack_factory.h"

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JApplication.h>
#include <JANA/JException.h>
#include <fmt/core.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <exception>
#include <tuple>

#include "ActsExamples/EventData/Trajectories.hpp"
#include "TrackPropagation.h"
#include "datamodel_glue.h"
#include "services/geometry/richgeo/RichGeo.h"

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::Init() {

  // get app and user info
  auto *app    = GetApplication();
  auto plugin = GetPluginName(); // plugin name should be detector name
  auto prefix = GetPrefix();

  // services
  m_richGeoSvc = app->GetService<RichGeo_service>();
  m_actsSvc    = app->GetService<ACTSGeo_service>();
  InitLogger(app, prefix, "info");
  m_propagation_algo.init(m_actsSvc->actsGeoProvider(), m_log);
  m_log->debug("RichTrack_factory: plugin='{}' prefix='{}'", plugin, prefix);

  // get list of radiators
  std::vector<std::tuple<int, std::string, std::string>> radiator_list; // < radiator_id, radiator_name, output_tag >
  for(auto& output_tag : GetOutputTags()) {
    auto radiator_id = richgeo::ParseRadiatorName(output_tag, m_log);
    radiator_list.emplace_back(
        radiator_id,
        richgeo::RadiatorName(radiator_id, m_log),
        output_tag
        );
  }

  // configuration parameters
  auto cfg = GetDefaultConfig();
  auto set_param = [&prefix, &app] (std::string name, auto &val, std::string description) {
    name = prefix + ":" + name;
    app->SetDefaultParameter(name, val, description);
  };
  for(auto& [radiator_id, radiator_name, output_tag] : radiator_list)
    set_param(radiator_name+":numPlanes", cfg.numPlanes[radiator_name], "");
  cfg.Print(m_log, spdlog::level::debug);

  // get RICH geometry for track propagation, for each radiator
  m_actsGeo = m_richGeoSvc->GetActsGeo(plugin);
  for(auto& [radiator_id, radiator_name, output_tag] : radiator_list) {
    m_tracking_planes.insert({
        output_tag,
        m_actsGeo->TrackingPlanes(radiator_id, cfg.numPlanes.at(radiator_name))
        });
    m_track_point_cuts.insert({ output_tag, m_actsGeo->TrackPointCut(radiator_id) });
  }

}

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {
}

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // FIXME this factory should use podio collections

  auto input_tags = GetInputTags();

  // collect tracks from first input tag
  auto tracks = event->Get<ActsExamples::ConstTrackContainer>(GetInputTags().back());
  input_tags.pop_back();

  // collect all trajectories from all input tags
  std::vector<const ActsExamples::Trajectories*> trajectories;
  for(const auto& input_tag : input_tags) {
    try {
      for(const auto traj : event->Get<ActsExamples::Trajectories>(input_tag))
        trajectories.push_back(traj);
    } catch(std::exception &e) {
      m_log->critical(e.what());
      throw JException(e.what());
    }
  }

  // choose the filter surface: all charged particles that pass through the dRICH vessel will pass
  // through the backplane of the gas radiator
  std::shared_ptr<Acts::Surface> filter_surface;
  for(auto& [output_tag, radiator_tracking_planes] : m_tracking_planes) {
    if(richgeo::ParseRadiatorName(output_tag, m_log) == richgeo::kGas) {
      filter_surface = radiator_tracking_planes.back();
      break;
    }
  }
  if(!filter_surface)
    throw JException("cannot find filter surface for RICH track propagation");

  // run track propagator algorithm, for each radiator
  for(auto& [output_tag, radiator_tracking_planes] : m_tracking_planes) {
    try {
      auto track_point_cut = m_track_point_cuts.at(output_tag);
      auto result = m_propagation_algo.propagateToSurfaceList(
          trajectories,
          radiator_tracking_planes,
          filter_surface,
          track_point_cut,
          true
          );
      SetCollection<edm4eic::TrackSegment>(output_tag, std::move(result));
    }
    catch(std::exception &e) {
      throw JException(e.what());
    }
  }

}
