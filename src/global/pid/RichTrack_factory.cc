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
    if(output_tag.find("TrackID") == std::string::npos) {
      auto radiator_id = richgeo::ParseRadiatorName(output_tag, m_log);
      radiator_list.push_back({
          radiator_id,
          richgeo::RadiatorName(radiator_id, m_log),
          output_tag
          });
    }
    else m_trackIDs_tag = output_tag; // output TrackID tag name
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

  // get RICH geometry for track projection, for each radiator
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

  /* workaround (FIXME)
   * - this factory creates multiple track projections (`edm4eic::TrackSegment`)
   *   for a single input `eicrecon::TrackingResultTrajectory`, but downstream
   *   code needs a way to know which of these `edm4eic::TrackSegments` came from
   *   the same input `eicrecon::TrackingResultTrajectory`
   * - `eicrecon::TrackingResultTrajectory` is not an `EDM4*` datatype, and
   *   therefore the `edm4eic::TrackSegment` objects this factory creates
   *   cannot link to them
   * - in the data model `edm4eic::TrackSegment` can have a 1-1 relation to
   *   an `edm4eic::Track`
   * - therefore, as a workaround, we generate a unique, empty `edm4eic::Track`
   *   for each input `eicrecon::TrackingResultTrajectory`, which can be used in this
   *   1-1 relation from `edm4eic::TrackSegment` as a unique identifier to encode
   *   which `edm4eic::TrackSegment`s came from the same input track
   * - finally, an additional 'TrackID' output tag is needed, to to guarantee
   *   these related `edm4eic::Track` objects have an owner are accessible by
   *   downstream readers
   */
  auto track_coll = std::make_unique<edm4eic::TrackCollection>();
  for(const auto& traj : trajectories)
    track_coll->create();

  // run algorithm, for each radiator
  for(auto& [output_tag, radiator_tracking_planes] : m_tracking_planes) {
    try {
      // propgate trajectories to RICH planes (discs)
      auto result = m_propagation_algo.propagateToSurfaceList(trajectories, radiator_tracking_planes);
      // 1-1 relation to unique tracks (see 'workaround' above)
      for(unsigned i_track=0; i_track<track_coll->size(); i_track++)
        result->at(i_track).setTrack(track_coll->at(i_track));
      // set factory output projected tracks
      SetCollection<edm4eic::TrackSegment>(output_tag, std::move(result));
    }
    catch(std::exception &e) {
      m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
    }
  }

  // set factory output TrackIDs
  SetCollection<edm4eic::Track>(m_trackIDs_tag, std::move(track_coll));
}
