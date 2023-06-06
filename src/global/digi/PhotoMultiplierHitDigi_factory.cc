// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks

#include "PhotoMultiplierHitDigi_factory.h"

void eicrecon::PhotoMultiplierHitDigi_factory::Init() {

  auto app = GetApplication();

  // Services
  auto geo_service = app->GetService<JDD4hep_service>();
  auto plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
  auto param_prefix = plugin_name + GetPrefix();
  InitLogger(param_prefix, "info");

  // Configuration parameters
  auto cfg = GetDefaultConfig();
  auto set_param = [&param_prefix, &app] (std::string name, auto &val, std::string description) {
    name = param_prefix + ":" + name;
    app->SetDefaultParameter(name, val, description);
  };
  set_param("seed",            cfg.seed,            "random number generator seed");
  set_param("hitTimeWindow",   cfg.hitTimeWindow,   "");
  set_param("timeResolution",  cfg.timeResolution,  "");
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

void eicrecon::PhotoMultiplierHitDigi_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {
  m_digi_algo.AlgorithmChangeRun();
}

void eicrecon::PhotoMultiplierHitDigi_factory::Process(const std::shared_ptr<const JEvent> &event) {

  auto sim_hits = static_cast<const edm4hep::SimTrackerHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

  try {
    auto result = m_digi_algo.AlgorithmProcess(sim_hits);
    SetCollection<edm4eic::RawTrackerHit>(GetOutputTags()[0], std::move(result.raw_hits));
    SetCollection<edm4eic::MCRecoTrackerHitAssociation>(GetOutputTags()[1], std::move(result.hit_assocs));
  }
  catch(std::exception &e) {
    throw JException(e.what());
  }

}
