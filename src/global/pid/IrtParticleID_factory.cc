// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "IrtParticleID_factory.h"

//-----------------------------------------------------------------------------
void eicrecon::IrtParticleID_factory::Init() {

  // get plugin name and tag
  auto app = GetApplication();
  m_detector_name  = eicrecon::str::ReplaceAll(GetPluginName(), ".so", ""); // plugin name should be detector name
  std::string param_prefix = m_detector_name + ":" + GetTag();
  InitDataTags(param_prefix);

  // services
  InitLogger(param_prefix, "info");
  m_richGeoSvc   = app->GetService<RichGeo_service>();
  m_irt_det_coll = m_richGeoSvc->GetIrtGeo(m_detector_name)->GetIrtDetectorCollection();
  m_dd4hep_det   = m_richGeoSvc->GetDD4hepGeo();
  m_log->debug("detector: {}   param_prefix: {}", m_detector_name, param_prefix);

  // config
  auto cfg = GetDefaultConfig();
  auto set_param = [&param_prefix, &app] (std::string name, auto &val, std::string description) {
    name = param_prefix + ":" + name;
    app->SetDefaultParameter(name, val, description);
  };
  set_param("numRIndexBins", cfg.numRIndexBins, "");
  for(auto& [name,rad] : cfg.radiators) {
    set_param(name+":id",              rad.id,              "");
    set_param(name+":smearingMode",    rad.smearingMode,    "");
    set_param(name+":smearing",        rad.smearing,        "");
    set_param(name+":referenceRIndex", rad.referenceRIndex, "");
    set_param(name+":attenuation",     rad.attenuation,     "");
    set_param(name+":zbins",           rad.zbins,           "");
  }

  // initialize underlying algorithm
  m_irt_algo.applyConfig(cfg);
  m_irt_algo.AlgorithmInit(m_dd4hep_det, m_irt_det_coll, m_log);
}

//-----------------------------------------------------------------------------
void eicrecon::IrtParticleID_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
  m_irt_algo.AlgorithmChangeRun();
}

//-----------------------------------------------------------------------------
void eicrecon::IrtParticleID_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // accumulate input collections
  // - if `input_tag` contains `Hits`, add to `raw_hits`
  // - if `input_tag` contains `Tracks`, add to `charged_particles`
  std::vector<const edm4eic::RawPMTHit*> raw_hits;
  std::map<std::string,std::vector<const edm4eic::TrackSegment*>> charged_particles; // map : radiator_name -> list of TrackSegments
  for(const auto &input_tag: GetInputTags()) {
    try {
      if(input_tag.find("Hits") != std::string::npos) {
        auto in = event->Get<edm4eic::RawPMTHit>(input_tag);
        raw_hits.insert(raw_hits.end(), in.begin(), in.end());
      }
      else {
        auto radiator_id = rich::ParseRadiatorName(input_tag);
        if(radiator_id>=0) {
          auto in = event->Get<edm4eic::TrackSegment>(input_tag);
          charged_particles.insert({ rich::RadiatorName(radiator_id), in });
        }
        else
          m_log->error("Unknown input collection '{}'", input_tag);
      }
    } catch(std::exception &e) {
      m_log->critical(e.what());
      throw JException(e.what());
    }
  }

  // call the IrtParticleID algorithm
  Set(m_irt_algo.AlgorithmProcess( raw_hits, charged_particles ));
}
