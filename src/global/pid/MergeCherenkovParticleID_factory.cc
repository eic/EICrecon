// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "MergeCherenkovParticleID_factory.h"

//-----------------------------------------------------------------------------
void eicrecon::MergeCherenkovParticleID_factory::Init() {

  // get plugin name and tag
  auto app = GetApplication();
  m_detector_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", ""); // plugin name should be detector name
  std::string param_prefix = m_detector_name + ":" + GetTag();
  InitDataTags(param_prefix);

  // services
  InitLogger(param_prefix, "info");
  m_log->debug("detector: {}   param_prefix: {}", m_detector_name, param_prefix);

  // config
  auto cfg = GetDefaultConfig();
  auto set_param = [&param_prefix, &app] (std::string name, auto &val, std::string description) {
    name = param_prefix + ":" + name;
    app->SetDefaultParameter(name, val, description);
  };
  set_param("mergeMode", cfg.mergeMode, "");

  // initialize underlying algorithm
  m_algo.applyConfig(cfg);
  m_algo.AlgorithmInit(m_log);
}

//-----------------------------------------------------------------------------
void eicrecon::MergeCherenkovParticleID_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {
  m_algo.AlgorithmChangeRun();
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
    SetCollection(std::move(merged_pids));
  }
  catch(std::exception &e) {
    m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
  }
}
