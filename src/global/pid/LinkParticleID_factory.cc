// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "LinkParticleID_factory.h"

//-----------------------------------------------------------------------------
template<class ParticleDatatype>
void eicrecon::LinkParticleID_factory<ParticleDatatype>::Init() {

  // get plugin name and tag
  auto app = this->GetApplication();
  m_detector_name = eicrecon::str::ReplaceAll(this->GetPluginName(), ".so", ""); // plugin name should be detector name
  auto param_prefix = m_detector_name + ":" + this->GetTag();
  this->InitDataTags(param_prefix);

  // services
  this->InitLogger(param_prefix, "info");
  this->m_log->debug("detector: {}   param_prefix: {}", m_detector_name, param_prefix);

  // config
  auto cfg = this->GetDefaultConfig();
  auto set_param = [&param_prefix, &app] (std::string name, auto &val, std::string description) {
    name = param_prefix + ":" + name;
    app->SetDefaultParameter(name, val, description);
  };
  set_param("phiTolerance", cfg.phiTolerance, "");
  set_param("etaTolerance", cfg.etaTolerance, "");

  // initialize underlying algorithm
  m_algo.applyConfig(cfg);
  m_algo.AlgorithmInit(this->m_log);
}

//-----------------------------------------------------------------------------
template<class ParticleDatatype>
void eicrecon::LinkParticleID_factory<ParticleDatatype>::BeginRun(const std::shared_ptr<const JEvent> &event) {
  m_algo.AlgorithmChangeRun();
}

//-----------------------------------------------------------------------------
template<class ParticleDatatype>
void eicrecon::LinkParticleID_factory<ParticleDatatype>::Process(const std::shared_ptr<const JEvent> &event) {

  // get input PID collection
  auto pids = static_cast<const edm4eic::CherenkovParticleIDCollection*>(event->GetCollectionBase(this->GetInputTags()[0]));

  // get input particle collection, and run algorithm, depending on type of `ParticleDatatype`:
  auto input_particle_tag = this->GetInputTags()[1];

  // - ReconstructedParticle
  if(std::is_same<ParticleDatatype, edm4eic::ReconstructedParticle>::value) {
    auto rec_parts = static_cast<const edm4eic::ReconstructedParticleCollection*>(event->GetCollectionBase(input_particle_tag));
    try { this->SetCollection(std::move(m_algo.AlgorithmProcess(pids, rec_parts))); }
    catch(std::exception &e) { this->m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what()); }
  }

  // - MCRecoParticleAssociation
  else if(std::is_same<ParticleDatatype, edm4eic::MCRecoParticleAssociation>::value) {
    auto rec_assocs = static_cast<const edm4eic::MCRecoParticleAssociationCollection*>(event->GetCollectionBase(input_particle_tag));
    try { this->SetCollection(std::move(m_algo.AlgorithmProcess(pids, rec_assocs))); }
    catch(std::exception &e) { this->m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what()); }
  }

  else
    this->m_log->error("Input tag {} has unfamiliar type", input_particle_tag);

}
