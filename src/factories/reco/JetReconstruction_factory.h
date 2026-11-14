// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Zhongling Ji, Derek Anderson

#pragma once

#include <string>
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "algorithms/reco/JetReconstruction.h"
#include "algorithms/reco/JetReconstructionConfig.h"

namespace eicrecon {

template <typename InputT>
class JetReconstruction_factory
    : public JOmniAlgorithmFactory<JetReconstruction_factory<InputT>, JetReconstruction<InputT>,
                                   JetReconstructionConfig> {
public:
  using FactoryT = JOmniAlgorithmFactory<JetReconstruction_factory<InputT>,
                                         JetReconstruction<InputT>, JetReconstructionConfig>;

private:
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
}; // end JetReconstruction_factory definition

} // namespace eicrecon
