// Created by Christopher Dilks
// Based on SiliconTrackerDigi_factory
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "PhotoMultiplierHitDigi_factory.h"

void eicrecon::PhotoMultiplierHitDigi_factory::Init() {
  using namespace eicrecon::str;

  auto app = GetApplication();

  m_plugin_name = ReplaceAll(GetPluginName(), ".so", "");

  // We will use plugin name to get parameters for correct factory
  // So if we use <plugin name>:parameter whichever plugin uses this template. eg:
  //    "BTRK:parameter" or "FarForward:parameter"
  // That has limitations but the convenient in the most of the cases
  std::string param_prefix = m_plugin_name + ":" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

  // Set input tags
  InitDataTags(param_prefix);

  // Services
  auto geo_service = app->GetService<JDD4hep_service>();
  InitLogger(param_prefix, "info");

  // Configuration parameters
  auto cfg = GetDefaultConfig();
  auto set_param = [&param_prefix, &app] (std::string name, auto &val, std::string description) {
    name = param_prefix + ":" + name;
    app->SetDefaultParameter(name, val, description);
  };
  set_param("seed",            cfg.seed,            "random number generator seed");
  set_param("hitTimeWindow",   cfg.hitTimeWindow,   "");
  set_param("timeStep",        cfg.timeStep,        "");
  set_param("speMean",         cfg.speMean,         "");
  set_param("speError",        cfg.speError,        "");
  set_param("pedMean",         cfg.pedMean,         "");
  set_param("pedError",        cfg.pedError,        "");
  set_param("enablePixelGaps", cfg.enablePixelGaps, "enable/disable removal of hits in gaps between pixels");
  set_param("pixelSize",       cfg.pixelSize,       "pixel (active) size");
  set_param("safetyFactor",    cfg.safetyFactor,    "overall safety factor");
  // set_param("quantumEfficiency", cfg.quantumEfficiency, ""); // FIXME JParameterManager cannot use vector<pair>

  // Initialize digitization algorithm
  m_digi_algo.applyConfig(cfg);
  m_digi_algo.AlgorithmInit(geo_service->detector(), m_log);
}

void eicrecon::PhotoMultiplierHitDigi_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
  m_digi_algo.AlgorithmChangeRun();
}

void eicrecon::PhotoMultiplierHitDigi_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // Collect all hits from different tags
  std::vector<const edm4hep::SimTrackerHit*> sim_hits;
  for(const auto &input_tag: GetInputTags()) {
    try {
      for (const auto hit : event->Get<edm4hep::SimTrackerHit>(input_tag))
        sim_hits.push_back(hit);
    } catch(std::exception &e) {
      m_log->critical(e.what());
      throw JException(e.what());
    }
  }

  // Digitize
  auto result = m_digi_algo.AlgorithmProcess(sim_hits);
  Set(result.raw_hits);
  event->Insert(result.photons, m_plugin_name+"Photons");
}
