// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "LinkParticleID_factory.h"

//-----------------------------------------------------------------------------
void eicrecon::LinkParticleID_factory::Init() {

  // get plugin name and tag
  auto app = GetApplication();
  m_detector_name  = eicrecon::str::ReplaceAll(GetPluginName(), ".so", ""); // plugin name should be detector name
  std::string param_prefix = m_detector_name + ":" + GetTag();
  InitDataTags(param_prefix);

  // services
  InitLogger(param_prefix, "info");
  m_log->debug("detector: {}   param_prefix: {}", m_detector_name, param_prefix);

  // print list of input collections
  m_log->debug("input collections:");
  for(const auto &input_tag : GetInputTags())
    m_log->debug(" - {}", input_tag);

  // config
  auto cfg = GetDefaultConfig();
  auto set_param = [&param_prefix, &app] (std::string name, auto &val, std::string description) {
    name = param_prefix + ":" + name;
    app->SetDefaultParameter(name, val, description);
  };
  set_param("phiTolerance", cfg.phiTolerance, "");
  set_param("etaTolerance", cfg.etaTolerance, "");

  // initialize underlying algorithm
  m_algo.applyConfig(cfg);
  m_algo.AlgorithmInit(m_log);
}

//-----------------------------------------------------------------------------
void eicrecon::LinkParticleID_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
  m_algo.AlgorithmChangeRun();
}

//-----------------------------------------------------------------------------
void eicrecon::LinkParticleID_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // accumulate input collections
  // - if `input_tag` contains `Reconstructed`, add to `reconstructed_particles`
  // - otherwise, if `input_tag` contains `ParticleID`, add to `reconstructed_pids`
  //   - FIXME: generalize for other ParticleID-type inputs, beyond CherenkovParticleID
  std::vector<const eicrecon::ParticlesWithAssociation*> reconstructed_particles;
  std::vector<const edm4eic::CherenkovParticleID*>       reconstructed_pids;
  for(const auto &input_tag : GetInputTags()) {
    try {
      if(input_tag.find("Reconstructed") != std::string::npos || input_tag.find("Association") != std::string::npos) {
        auto in = event->Get<eicrecon::ParticlesWithAssociation>(input_tag);
        reconstructed_particles.insert(reconstructed_particles.end(), in.begin(), in.end());
      }
      else if(input_tag.find("ParticleID") != std::string::npos) {
        auto in = event->Get<edm4eic::CherenkovParticleID>(input_tag);
        reconstructed_pids.insert(reconstructed_pids.end(), in.begin(), in.end());
      }
      else
        m_log->error("Unknown input collection '{}'", input_tag);
    } catch(std::exception &e) {
      m_log->critical(e.what());
      throw JException(e.what());
    }
  }

  // call the LinkParticleID algorithm
  auto result = m_algo.AlgorithmProcess(reconstructed_particles,reconstructed_pids);

  // output
  Set(std::move(result));
}
