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
  m_irtGeoSvc = app->GetService<IrtGeo_service>();
  m_actsSvc   = app->GetService<ACTSGeo_service>();
  InitLogger(param_prefix, "info");

  // default configuration parameters
  m_numPlanes[IrtGeo::kAerogel] = 5;
  m_numPlanes[IrtGeo::kGas]     = 10;

  // configuration parameters
  auto set_param = [&param_prefix, &app, this] (std::string name, auto &val, std::string description) {
    name = param_prefix + ":" + name;
    m_log->info("set_param={}",name); // FIXME: DEBUGGGGGGGGGGGGGG
    app->SetDefaultParameter(name, val, description);
  };
  for(int rad=0; rad<IrtGeo::nRadiators; rad++) {
    auto radName = IrtGeo::RadiatorName(rad);
    set_param(
        "numPlanes:"+radName,
        m_numPlanes[rad],
        "number of "+radName+" radiator track-projection planes"
        );
  }
  m_log->info("NUMPLANES = {}, {}",m_numPlanes[0],m_numPlanes[1]); // FIXME: DEBUGGGGGGGGGGGGGG
  
  // get RICH geometry for track projection
  m_irtGeo = m_irtGeoSvc->GetIrtGeo(m_detector_name);
  for(int rad=0; rad<IrtGeo::nRadiators; rad++)
    m_trackingPlanes.insert({ rad, m_irtGeo->TrackingPlanes(rad, m_numPlanes[rad]) });
}

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // Get trajectories from tracking
  auto trajectories = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories");
  std::vector<edm4eic::TrackPoint*> track_points;

  // outputs
  Set(track_points);
}
