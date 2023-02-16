// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "RichTrack_factory.h"

#include <edm4eic/vector_utils.h>

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::Init() {
  auto app = GetApplication();

  // input tags
  auto detector_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
  auto param_prefix = detector_name + ":" + GetTag();
  InitDataTags(param_prefix);
  m_radiatorID = rich::ParseRadiatorName(GetTag());

  // services
  m_richGeoSvc = app->GetService<RichGeo_service>();
  m_actsSvc    = app->GetService<ACTSGeo_service>();
  InitLogger(param_prefix, "info");
  m_propagation_algo.init(m_actsSvc->actsGeoProvider(), m_log);
  m_log->debug("detector_name='{}'  param_prefix='{}'  m_radiatorID={}", detector_name, param_prefix, m_radiatorID);

  // default configuration parameters
  /* FIXME: eventually we will move this factory to an independent algorithm
   * and be able to use a Config class, overridable in {D,PF}RICH.cc; for now,
   * a quick way to set the defaults without reco_flags.py, to tide us over
   * until we have a proper config file front end
   */
  m_numPlanes = 5;
  if(param_prefix.rfind("DRICH")!=std::string::npos || param_prefix.rfind("PFRICH")!=std::string::npos) {
    if(param_prefix.rfind("Aerogel")!=std::string::npos) m_numPlanes = 5;
    else if(param_prefix.rfind("Gas")!=std::string::npos) m_numPlanes = 10;
  }

  // configuration parameters
  auto set_param = [&param_prefix, &app, this] (std::string name, auto &val, std::string description) {
    app->SetDefaultParameter(param_prefix+":"+name, val, description);
  };
  set_param("numPlanes", m_numPlanes, "number of track-projection planes");
  m_log->debug("numPlanes = {}",m_numPlanes);

  // get RICH geometry for track projection
  m_actsGeo = m_richGeoSvc->GetActsGeo(detector_name);
  m_trackingPlanes = m_actsGeo->TrackingPlanes(m_radiatorID, m_numPlanes);
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

  // result will be `TrackSegments`s for each trajectory
  std::vector<edm4eic::TrackSegment*> result;

  // loop over trajectories
  m_log->debug("Propagate trajectories: --------------------");
  m_log->debug("number of trajectories: {}",trajectories.size());
  for(const auto& traj : trajectories)
    result.push_back(m_propagation_algo.propagateToSurfaceList( traj, m_trackingPlanes ));

  // output
  Set(std::move(result));
}
