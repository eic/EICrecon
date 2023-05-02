// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "LinkParticleID_factory.h"

namespace eicrecon {

  // One time initialization
  //-----------------------------------------------------------------------------
  template<class ParticleDataT, class ParticleCollectionT>
  void LinkParticleID_factory<ParticleDataT,ParticleCollectionT>::Init() {

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


  // On run change preparations
  //-----------------------------------------------------------------------------
  template<class ParticleDataT, class ParticleCollectionT>
  void LinkParticleID_factory<ParticleDataT,ParticleCollectionT>::BeginRun(const std::shared_ptr<const JEvent> &event) {
    m_algo.AlgorithmChangeRun();
  }


  // Event by event processing
  //-----------------------------------------------------------------------------
  template<class ParticleDataT, class ParticleCollectionT>
  void LinkParticleID_factory<ParticleDataT,ParticleCollectionT>::Process(const std::shared_ptr<const JEvent> &event) {

    // get input collections
    auto pids  = static_cast<const edm4eic::CherenkovParticleIDCollection*>(event->GetCollectionBase(this->GetInputTags()[0]));
    auto parts = static_cast<const ParticleCollectionT*>(event->GetCollectionBase(this->GetInputTags()[1]));

    // run algorithm
    try { this->SetCollection(std::move(m_algo.AlgorithmProcess(pids, parts))); }
    catch(std::exception &e) { this->m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what()); }

  }

  // explicit instantiations: list of supported datatypes
  //-----------------------------------------------------------------------------
  template class LinkParticleID_factory<edm4eic::ReconstructedParticle,edm4eic::ReconstructedParticleCollection>;
  template class LinkParticleID_factory<edm4eic::MCRecoParticleAssociation,edm4eic::MCRecoParticleAssociationCollection>;

}
