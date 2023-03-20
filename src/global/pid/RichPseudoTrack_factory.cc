// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "RichPseudoTrack_factory.h"

//-----------------------------------------------------------------------------
void eicrecon::RichPseudoTrack_factory::Init() {
  auto app = GetApplication();

  // input tags
  auto detector_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
  auto param_prefix = detector_name + ":" + GetTag();
  InitDataTags(param_prefix);
  m_radiatorID = richgeo::ParseRadiatorName(GetTag());

  // services
  m_richGeoSvc = app->GetService<RichGeo_service>();
  m_actsGeo    = m_richGeoSvc->GetActsGeo(detector_name);
  InitLogger(param_prefix, "info");
  m_log->debug("detector_name='{}'  param_prefix='{}'  m_radiatorID={}", detector_name, param_prefix, m_radiatorID);

  // configuration parameters
  auto cfg = GetDefaultConfig();
  app->SetDefaultParameter(param_prefix+":numPoints", cfg.numPoints, "number of pseudtrack points");
  m_tracks_algo.applyConfig(cfg);

  // initialize algorithm
  m_tracks_algo.AlgorithmInit(m_actsGeo->WithinRadiator[m_radiatorID], m_log);
}

//-----------------------------------------------------------------------------
void eicrecon::RichPseudoTrack_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
}

//-----------------------------------------------------------------------------
void eicrecon::RichPseudoTrack_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // collect all hits from all input tags
  std::vector<const edm4hep::SimTrackerHit*> mc_hits;
  for(const auto& input_tag : GetInputTags()) {
    try {
      for(const auto mc_hit : event->Get<edm4hep::SimTrackerHit>(input_tag))
        mc_hits.push_back(mc_hit);
    } catch(std::exception &e) {
      m_log->critical(e.what());
      throw JException(e.what());
    }
  }

  // output
  auto result = m_tracks_algo.AlgorithmProcess(mc_hits);
  Set(std::move(result));
}
