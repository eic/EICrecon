// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks

#include "PhotoMultiplierHitDigi_factory.h"

#include <DD4hep/Objects.h>
#include <JANA/JApplication.h>
#include <JANA/JException.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <fmt/core.h>
#include <spdlog/logger.h>
#include <exception>
#include <functional>

#include "datamodel_glue.h"
// services
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "services/geometry/richgeo/RichGeo_service.h"

void eicrecon::PhotoMultiplierHitDigi_factory::Init() {

  // get app and user info
  auto *app    = GetApplication();
  auto plugin = GetPluginName();
  auto prefix = GetPrefix();

  // services
  auto geo_service = app->GetService<JDD4hep_service>();
  InitLogger(app, prefix, "info");
  m_log->debug("PhotoMultiplierHitDigi_factory: plugin='{}' prefix='{}'", plugin, prefix);

  // get readout info (if a RICH)
  bool use_richgeo = plugin=="DRICH" || plugin=="PFRICH";
  if(use_richgeo) {
    auto richGeoSvc = app->GetService<RichGeo_service>();
    m_readoutGeo    = richGeoSvc->GetReadoutGeo(plugin);
  }

  // Configuration parameters
  auto cfg = GetDefaultConfig();
  auto set_param = [&prefix, &app] (std::string name, auto &val, std::string description) {
    name = prefix + ":" + name;
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
  set_param("safetyFactor",    cfg.safetyFactor,    "overall safety factor");
  set_param("enableNoise",     cfg.enableNoise,     "");
  set_param("noiseRate",       cfg.noiseRate,       "");
  set_param("noiseTimeWindow", cfg.noiseTimeWindow, "");
  // set_param("quantumEfficiency", cfg.quantumEfficiency, ""); // FIXME JParameterManager cannot use vector<pair>

  // Initialize digitization algorithm
  m_digi_algo.applyConfig(cfg);
  m_digi_algo.AlgorithmInit(geo_service->detector(), m_log);

  // Initialize richgeo ReadoutGeo and set random CellID visitor lambda (if a RICH)
  if(use_richgeo) {
    m_readoutGeo->SetSeed(cfg.seed);
    m_digi_algo.SetVisitRngCellIDs(
        [readoutGeo = this->m_readoutGeo] (std::function<void(PhotoMultiplierHitDigi::CellIDType)> lambda, float p) { readoutGeo->VisitAllRngPixels(lambda, p); }
        );
    m_digi_algo.SetPixelGapMask(
        [readoutGeo = this->m_readoutGeo] (PhotoMultiplierHitDigi::CellIDType cellID, dd4hep::Position pos) { return readoutGeo->PixelGapMask(cellID, pos); }
        );

  }
}

void eicrecon::PhotoMultiplierHitDigi_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {
  m_digi_algo.AlgorithmChangeRun();
}

void eicrecon::PhotoMultiplierHitDigi_factory::Process(const std::shared_ptr<const JEvent> &event) {

  const auto *sim_hits = static_cast<const edm4hep::SimTrackerHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

  try {
    auto result = m_digi_algo.AlgorithmProcess(sim_hits);
    SetCollection<edm4eic::RawTrackerHit>(GetOutputTags()[0], std::move(result.raw_hits));
    SetCollection<edm4eic::MCRecoTrackerHitAssociation>(GetOutputTags()[1], std::move(result.hit_assocs));
  }
  catch(std::exception &e) {
    throw JException(e.what());
  }

}
