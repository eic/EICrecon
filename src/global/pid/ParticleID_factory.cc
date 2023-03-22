// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "ParticleID_factory.h"

//-----------------------------------------------------------------------------
void eicrecon::ParticleID_factory::Init() {

  // get plugin name and tag
  auto app = GetApplication();
  m_detector_name  = eicrecon::str::ReplaceAll(GetPluginName(), ".so", ""); // plugin name should be detector name
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
  set_param("highestWeightOnly", cfg.highestWeightOnly, "");

  // initialize underlying algorithm
  m_algo.applyConfig(cfg);
  m_algo.AlgorithmInit(m_log);
}

//-----------------------------------------------------------------------------
void eicrecon::ParticleID_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
  m_algo.AlgorithmChangeRun();
}

//-----------------------------------------------------------------------------
void eicrecon::ParticleID_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // accumulate input collections
  // FIXME: generalize this when we have differing input tag types
  std::vector<const edm4eic::CherenkovParticleID*> cherenkov_pids;
  for(const auto& input_tag : GetInputTags()) {
    try {
      for(const auto cherenkov_pid : event->Get<edm4eic::CherenkovParticleID>(input_tag))
        cherenkov_pids.push_back(cherenkov_pid);
    } catch(std::exception &e) {
      m_log->critical(e.what());
      throw JException(e.what());
    }
  }

  // call the ParticleID algorithm
  auto global_pids = m_algo.AlgorithmProcess(cherenkov_pids);

  // output
  Set(std::move(global_pids));
}
