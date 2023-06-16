// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "IrtCherenkovParticleID_factory.h"

//-----------------------------------------------------------------------------
void eicrecon::IrtCherenkovParticleID_factory::Init() {

  // get plugin name and tag
  auto app = GetApplication();
  m_detector_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", ""); // plugin name should be detector name
  auto param_prefix = m_detector_name + GetPrefix();

  // services
  InitLogger(param_prefix, "info");
  m_richGeoSvc   = app->GetService<RichGeo_service>();
  m_irt_det_coll = m_richGeoSvc->GetIrtGeo(m_detector_name)->GetIrtDetectorCollection();
  m_log->debug("detector: {}   param_prefix: {}", m_detector_name, param_prefix);

  // config
  auto cfg = GetDefaultConfig();
  auto set_param = [&param_prefix, &app] (std::string name, auto &val, std::string description) {
    name = param_prefix + ":" + name;
    app->SetDefaultParameter(name, val, description);
  };
  set_param("numRIndexBins", cfg.numRIndexBins, "");
  set_param("pdgList",       cfg.pdgList,       "");
  for(auto& [name,rad] : cfg.radiators) {
    set_param(name+":smearingMode",    rad.smearingMode,    "");
    set_param(name+":smearing",        rad.smearing,        "");
    set_param(name+":referenceRIndex", rad.referenceRIndex, "");
    set_param(name+":attenuation",     rad.attenuation,     "");
    set_param(name+":zbins",           rad.zbins,           "");
  }
  set_param("cheatPhotonVertex",  cfg.cheatPhotonVertex,  "");
  set_param("cheatTrueRadiator",  cfg.cheatTrueRadiator,  "");

  // initialize underlying algorithm
  m_irt_algo.applyConfig(cfg);
  m_irt_algo.AlgorithmInit(m_irt_det_coll, m_log);
}

//-----------------------------------------------------------------------------
void eicrecon::IrtCherenkovParticleID_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {
  m_irt_algo.AlgorithmChangeRun();
}

//-----------------------------------------------------------------------------
void eicrecon::IrtCherenkovParticleID_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // get input track propagation collections from radiators
  std::map<std::string, const edm4eic::TrackSegmentCollection*> charged_particles; // map : radiator_name -> collection of TrackSegments
  int tag_num = 0;
  while(tag_num < richgeo::nRadiators) {
    auto input_tag   = GetInputTags()[tag_num++];
    auto radiator_id = richgeo::ParseRadiatorName(input_tag);
    if(radiator_id >= 0 && radiator_id < richgeo::nRadiators)
      charged_particles.insert({
          richgeo::RadiatorName(radiator_id),
          static_cast<const edm4eic::TrackSegmentCollection*>(event->GetCollectionBase(input_tag))
          });
    else m_log->error("Unknown input RICH track collection '{}'", input_tag);
  }

  // get merged track propagation
  if(GetInputTags()[tag_num].find("Merge") != std::string::npos)
    charged_particles.insert({
        "Merged",
        static_cast<const edm4eic::TrackSegmentCollection*>(event->GetCollectionBase(GetInputTags()[tag_num++]))
        });
  else {
    m_log->critical("input tag {} should be merged tracks", tag_num);
    return;
  }

  // get input hit collections
  auto raw_hits   = static_cast<const edm4eic::RawTrackerHitCollection*>(event->GetCollectionBase(GetInputTags()[tag_num++]));
  auto hit_assocs = static_cast<const edm4eic::MCRecoTrackerHitAssociationCollection*>(event->GetCollectionBase(GetInputTags()[tag_num++]));

  try {
    // run the IrtCherenkovParticleID algorithm
    auto result = m_irt_algo.AlgorithmProcess(charged_particles, raw_hits, hit_assocs);
    // set output collections, matching radiator name to corresponding output tag
    for(auto& [rad_name, coll] : result) {
      for(auto& output_tag : GetOutputTags()) {
        if(output_tag.find(rad_name) != std::string::npos)
          SetCollection<edm4eic::CherenkovParticleID>(output_tag, std::move(coll));
      }
    }
  }
  catch(std::exception &e) {
    throw JException(e.what());
  }
}
