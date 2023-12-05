// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

#include "MergeCherenkovParticleID_factory.h"

#include <JANA/JApplication.h>
#include <JANA/JException.h>
#include <fmt/core.h>
#include <spdlog/logger.h>
#include <exception>

#include "datamodel_glue.h"

//-----------------------------------------------------------------------------
void eicrecon::MergeCherenkovParticleID_factory::Init() {

  // get app and user info
  auto app    = GetApplication();
  auto plugin = GetPluginName();
  auto prefix = plugin + ":" + GetTag();

  // services
  InitLogger(app, prefix, "info");
  m_log->debug("MergeCherenkovParticleID_factory: plugin='{}' prefix='{}'", plugin, prefix);

  // config
  auto cfg = GetDefaultConfig();
  auto set_param = [&prefix, &app] (std::string name, auto &val, std::string description) {
    name = prefix + ":" + name;
    app->SetDefaultParameter(name, val, description);
  };
  set_param("mergeMode", cfg.mergeMode, "");

  // initialize underlying algorithm
  m_algo.applyConfig(cfg);
  m_algo.AlgorithmInit(m_log);
}

//-----------------------------------------------------------------------------
void eicrecon::MergeCherenkovParticleID_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // get input collections
  std::vector<const edm4eic::CherenkovParticleIDCollection*> cherenkov_pids;
  for(auto& input_tag : GetInputTags())
    cherenkov_pids.push_back(
        static_cast<const edm4eic::CherenkovParticleIDCollection*>(event->GetCollectionBase(input_tag))
        );

  // call the MergeParticleID algorithm
  try {
    auto merged_pids = m_algo.AlgorithmProcess(cherenkov_pids);
    SetCollection<edm4eic::CherenkovParticleID>(GetOutputTags()[0], std::move(merged_pids));
  }
  catch(std::exception &e) {
    throw JException(e.what());
  }
}
