// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Zhongling Ji, Derek Anderson

#pragma once

#include <string>
#include "extensions/jana/JOmniFactory.h"
#include "algorithms/reco/JetReconstruction.h"
#include "algorithms/reco/JetReconstructionConfig.h"

namespace eicrecon {

template <typename InputT>
class JetReconstruction_factory
    : public JOmniFactory<JetReconstruction_factory<InputT>, JetReconstructionConfig> {

public:
  // algorithm to run
  using Algo     = eicrecon::JetReconstruction<InputT>;
  using FactoryT = JOmniFactory<JetReconstruction_factory<InputT>, JetReconstructionConfig>;

private:
  std::unique_ptr<Algo> m_algo;

  // input collection
  typename FactoryT::template PodioInput<InputT> m_input{this};

  // output collection
  typename FactoryT::template PodioOutput<edm4eic::ReconstructedParticle> m_output{this};

  // parameter bindings
  typename FactoryT::template ParameterRef<float> m_rJet{this, "rJet", FactoryT::config().rJet};
  typename FactoryT::template ParameterRef<float> m_pJet{this, "pJet", FactoryT::config().pJet};
  typename FactoryT::template ParameterRef<double> m_minCstPt{this, "minCstPt",
                                                              FactoryT::config().minCstPt};
  typename FactoryT::template ParameterRef<double> m_maxCstPt{this, "maxCstPt",
                                                              FactoryT::config().maxCstPt};
  typename FactoryT::template ParameterRef<double> m_minJetPt{this, "minJetPt",
                                                              FactoryT::config().minJetPt};
  typename FactoryT::template ParameterRef<double> m_ghostMaxRap{this, "ghostMaxRap",
                                                                 FactoryT::config().ghostMaxRap};
  typename FactoryT::template ParameterRef<double> m_ghostArea{this, "ghostArea",
                                                               FactoryT::config().ghostArea};
  typename FactoryT::template ParameterRef<int> m_numGhostRepeat{this, "numGhostRepeat",
                                                                 FactoryT::config().numGhostRepeat};
  typename FactoryT::template ParameterRef<std::string> m_jetAlgo{this, "jetAlgo",
                                                                  FactoryT::config().jetAlgo};
  typename FactoryT::template ParameterRef<std::string> m_recombScheme{
      this, "recombScheme", FactoryT::config().recombScheme};
  typename FactoryT::template ParameterRef<std::string> m_areaType{this, "areaType",
                                                                   FactoryT::config().areaType};

public:
  void Configure() {
    m_algo = std::make_unique<Algo>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->applyConfig(FactoryT::config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_input()}, {m_output().get()});
  }

}; // end JetReconstruction_factory definition

} // namespace eicrecon
