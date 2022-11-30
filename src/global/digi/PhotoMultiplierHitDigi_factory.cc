// Created by Christopher Dilks
// Based on SiliconTrackerDigi_factory
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "PhotoMultiplierHitDigi_factory.h"

void eicrecon::PhotoMultiplierHitDigi_factory::Init() {
  using namespace eicrecon::str;

  auto app = GetApplication();
  auto pm  = app->GetJParameterManager();

  std::string plugin_name = ReplaceAll(GetPluginName(), ".so", "");

  // We will use plugin name to get parameters for correct factory
  // So if we use <plugin name>:parameter whichever plugin uses this template. eg:
  //    "BTRK:parameter" or "FarForward:parameter"
  // That has limitations but the convenient in the most of the cases
  std::string param_prefix = plugin_name + ":" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

  // Set input tags
  InitDataTags(param_prefix);

  // Logger. Get plugin level sub-log
  InitLogger(param_prefix, "info");

  // Setup digitization algorithm
  auto cfg = GetDefaultConfig();
  // pm->SetDefaultParameter(param_prefix + ":quantumEfficiency", cfg.quantumEfficiency, ""); // FIXME cannot use vector<pair>
  pm->SetDefaultParameter(param_prefix + ":hitTimeWindow",     cfg.hitTimeWindow,     "");
  pm->SetDefaultParameter(param_prefix + ":timeStep",          cfg.timeStep,          "");
  pm->SetDefaultParameter(param_prefix + ":speMean",           cfg.speMean,           "");
  pm->SetDefaultParameter(param_prefix + ":speError",          cfg.speError,          "");
  pm->SetDefaultParameter(param_prefix + ":pedMean",           cfg.pedMean,           "");
  pm->SetDefaultParameter(param_prefix + ":pedError",          cfg.pedError,          "");

  // Initialize digitization algorithm
  m_digi_algo.applyConfig(cfg);
  m_digi_algo.AlgorithmInit(m_log);
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
    }
  }

  // Digitize
  Set(m_digi_algo.AlgorithmProcess(sim_hits));
}
