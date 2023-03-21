// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "IrtCherenkovParticleID_factory.h"

//-----------------------------------------------------------------------------
void eicrecon::IrtCherenkovParticleID_factory::Init() {

  // get plugin name and tag
  auto app = GetApplication();
  m_detector_name  = eicrecon::str::ReplaceAll(GetPluginName(), ".so", ""); // plugin name should be detector name
  std::string param_prefix = m_detector_name + ":" + GetTag();
  InitDataTags(param_prefix);

  // services
  InitLogger(param_prefix, "info");
  m_richGeoSvc   = app->GetService<RichGeo_service>();
  m_irt_det_coll = m_richGeoSvc->GetIrtGeo(m_detector_name)->GetIrtDetectorCollection();
  m_log->debug("detector: {}   param_prefix: {}", m_detector_name, param_prefix);

  // print list of input collections, and inform the user if the charged
  // particle tracks were determined from MC photons
  m_log->debug("input collections:");
  for(const auto &input_tag : GetInputTags()) {
    m_log->debug(" - {}", input_tag);
    if(input_tag.find("PseudoTrack") != std::string::npos)
      m_log->warn("CHEAT MODE '{}' ENABLED: use photon emission points as tracks", input_tag);
  }

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
void eicrecon::IrtCherenkovParticleID_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
  m_irt_algo.AlgorithmChangeRun();
}

//-----------------------------------------------------------------------------
void eicrecon::IrtCherenkovParticleID_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // accumulate input collections
  // - if `input_tag` contains `Hits`, add to `raw_hit_assocs`
  // - if `input_tag` contains `Tracks`, add to `charged_particles`
  std::vector<const edm4eic::MCRecoTrackerHitAssociation*> raw_hit_assocs;
  std::map<std::string,std::vector<const edm4eic::TrackSegment*>> charged_particles; // map : radiator_name -> list of TrackSegments
  for(const auto &input_tag : GetInputTags()) {
    try {
      if(input_tag.find("Hits") != std::string::npos) {
        auto in = event->Get<edm4eic::MCRecoTrackerHitAssociation>(input_tag);
        raw_hit_assocs.insert(raw_hit_assocs.end(), in.begin(), in.end());
      }
      else {
        auto radiator_id = richgeo::ParseRadiatorName(input_tag);
        if(radiator_id>=0) {
          auto in = event->Get<edm4eic::TrackSegment>(input_tag);
          charged_particles.insert({ richgeo::RadiatorName(radiator_id), in });
        }
        else
          m_log->error("Unknown input collection '{}'", input_tag);
      }
    } catch(std::exception &e) {
      m_log->critical(e.what());
      throw JException(e.what());
    }
  }

  // call the IrtCherenkovParticleID algorithm
  auto cherenkov_pids = m_irt_algo.AlgorithmProcess( raw_hit_assocs, charged_particles );

  // output
  Set(std::move(cherenkov_pids));
}
