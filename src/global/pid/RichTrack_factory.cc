// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "RichTrack_factory.h"

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::Init() {
  auto app = GetApplication();

  // input tags
  auto detector_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
  auto param_prefix  = detector_name + GetPrefix();

  // services
  m_richGeoSvc = app->GetService<RichGeo_service>();
  m_actsSvc    = app->GetService<ACTSGeo_service>();
  InitLogger(param_prefix, "info");
  m_propagation_algo.init(m_actsSvc->actsGeoProvider(), m_log);
  m_log->debug("detector_name='{}'  param_prefix='{}'", detector_name, param_prefix);

  // get list of radiators
  std::vector<std::tuple<int, std::string, std::string>> radiator_list; // < radiator_id, radiator_name, output_tag >
  for(auto& output_tag : GetOutputTags()) {
    auto radiator_id = richgeo::ParseRadiatorName(output_tag, m_log);
    radiator_list.push_back({
        radiator_id,
        richgeo::RadiatorName(radiator_id, m_log),
        output_tag
        });
  }

  // configuration parameters
  auto cfg = GetDefaultConfig();
  auto set_param = [&param_prefix, &app] (std::string name, auto &val, std::string description) {
    name = param_prefix + ":" + name;
    app->SetDefaultParameter(name, val, description);
  };
  for(auto& [radiator_id, radiator_name, output_tag] : radiator_list)
    set_param(radiator_name+":numPlanes", cfg.numPlanes[radiator_name], "");
  cfg.Print(m_log, spdlog::level::debug);

  // get RICH geometry for track propagation, for each radiator
  m_actsGeo = m_richGeoSvc->GetActsGeo(detector_name);
  for(auto& [radiator_id, radiator_name, output_tag] : radiator_list)
    m_tracking_planes.insert({
        output_tag,
        m_actsGeo->TrackingPlanes(radiator_id, cfg.numPlanes.at(radiator_name))
        });

}

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {
}

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // collect all trajectories from all input tags
  std::vector<const eicrecon::TrackingResultTrajectory*> trajectories;
  for(const auto& input_tag : GetInputTags()) {
    try {
      for(const auto traj : event->Get<eicrecon::TrackingResultTrajectory>(input_tag))
        trajectories.push_back(traj);
    } catch(std::exception &e) {
      m_log->critical(e.what());
      throw JException(e.what());
    }
  }

  // run track propagator algorithm, for each radiator
  for(auto& [output_tag, radiator_tracking_planes] : m_tracking_planes) {
    try {
      auto result = m_propagation_algo.propagateToSurfaceList(trajectories, radiator_tracking_planes);
      SetCollection<edm4eic::TrackSegment>(output_tag, std::move(result));
    }
    catch(std::exception &e) {
      m_log->warn("Exception in underlying propagator algorithm: {}. Event data will be skipped", e.what());
    }
  }

}
