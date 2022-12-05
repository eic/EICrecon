// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "RichTrack_factory.h"

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::Init() {
  auto app = GetApplication();

  // input tags
  auto plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
  m_detector_name = "DRICH"; // FIXME: respect https://github.com/eic/EICrecon/pull/242
  auto param_prefix = plugin_name + ":" + GetTag();
  InitDataTags(param_prefix);
  m_log->info("plugin_name={}",plugin_name); // FIXME: DEBUGGGGGGGGGGGGGG
  m_log->info("param_prefix={}",plugin_name);

  // services
  m_richGeoSvc = app->GetService<RichGeo_service>();
  m_actsSvc    = app->GetService<ACTSGeo_service>();
  InitLogger(param_prefix, "info");
  m_propagation_algo.init(m_actsSvc->actsGeoProvider(), m_log);

  // default configuration parameters
  m_numPlanes[rich::kAerogel] = 5;
  m_numPlanes[rich::kGas]     = 10;

  // configuration parameters
  auto set_param = [&param_prefix, &app, this] (std::string name, auto &val, std::string description) {
    name = param_prefix + ":" + name;
    m_log->info("set_param={}",name); // FIXME: DEBUGGGGGGGGGGGGGG
    app->SetDefaultParameter(name, val, description);
  };
  for(int rad=0; rad<rich::nRadiators; rad++) {
    auto radName = rich::RadiatorName(rad);
    set_param(
        "numPlanes:"+radName,
        m_numPlanes[rad],
        "number of "+radName+" radiator track-projection planes"
        );
  }
  m_log->info("NUMPLANES = {}, {}",m_numPlanes[0],m_numPlanes[1]); // FIXME: DEBUGGGGGGGGGGGGGG
  
  // get RICH geometry for track projection
  m_actsGeo = m_richGeoSvc->GetActsGeo(m_detector_name);
  for(int rad=0; rad<rich::nRadiators; rad++)
    m_trackingPlanes.insert({ rad, m_actsGeo->TrackingPlanes(rad, m_numPlanes[rad]) });
}

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
}

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // collect all trajectories all input tags
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

  // result will be `TrackPoint`s for each trajectory, RICH tracking plane, and radiator
  std::vector<edm4eic::TrackPoint*> result;

  // loop over trajectories
  m_log->trace("Propagate trajectories:");
  for(const auto& traj : trajectories) { 
    // loop over radiators
    for(const auto& [rad,planes] : m_trackingPlanes) {
      m_log->trace("- radiator = {}", rich::RadiatorName(rad));
      // loop over track-projection planes for this radiator
      for(const auto& plane : planes) {
        edm4eic::TrackPoint *point;
        try {
          // project to this plane
          point = m_propagation_algo.propagate(traj, plane);
        } catch(std::exception &e) {
          m_log->warn("    Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
        }
        if(point) {
          result.push_back(point);
          m_log->trace("    trajectory: x=( {:>10.2f} {:>10.2f} {:>10.2f} )",
              point->position.x, point->position.y, point->position.z);
          m_log->trace("                p=( {:>10.2f} {:>10.2f} {:>10.2f} )",
              point->momentum.x, point->momentum.y, point->momentum.z);
        }
        else m_log->trace("    Failed to propagate trajectory");
      }
    }
  }
  Set(std::move(result));
}
