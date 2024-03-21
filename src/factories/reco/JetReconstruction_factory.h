// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Zhongling Ji, Derek Anderson

#pragma once

#include <string>
#include "extensions/jana/JOmniFactory.h"
#include "algorithms/reco/JetReconstruction.h"
#include "algorithms/reco/JetReconstructionConfig.h"

namespace eicrecon {

    class JetReconstruction_factory : public JOmniFactory<JetReconstruction_factory, JetReconstructionConfig> {

    public: 
      // algorithm to run
      using Algo = eicrecon::JetReconstruction;

    private:
      std::unique_ptr<Algo> m_algo;

      // input collection
      PodioInput<edm4eic::ReconstructedParticle> m_input {this};

      // output collection
      PodioOutput<edm4eic::ReconstructedParticle> m_output {this};

      // parameter bindings
      ParameterRef<float>       m_rJet {this, "rJet", config().rJet};
      ParameterRef<float>       m_pJet {this, "pJet", config().pJet};
      ParameterRef<double>      m_minCstPt {this, "minCstPt", config().minCstPt};
      ParameterRef<double>      m_maxCstPt {this, "maxCstPt", config().maxCstPt};
      ParameterRef<double>      m_minJetPt {this, "minJetPt", config().minJetPt};
      ParameterRef<double>      m_ghostMaxRap {this, "ghostMaxRap", config().ghostMaxRap};
      ParameterRef<double>      m_ghostArea {this, "ghostArea", config().ghostArea};
      ParameterRef<int>         m_numGhostRepeat {this, "numGhostRepeat", config().numGhostRepeat};
      ParameterRef<std::string> m_jetAlgo {this, "jetAlgo", config().jetAlgo};
      ParameterRef<std::string> m_recombScheme {this, "recombScheme", config().recombScheme};
      ParameterRef<std::string> m_areaType {this, "areaType", config().areaType};

    public:

      void Configure() {
        m_algo = std::make_unique<Algo>(this->GetPrefix());
        m_algo->applyConfig(config());
        m_algo->init(logger());
      }

      void ChangeRun(int64_t run_number) {
        /* nothing to do */
      }

      void Process(int64_t run_number, int64_t event_number) {
        m_algo->process(
	  {m_input()},
	  {m_output().get()}
        );
      }

    };  // end JetReconstruction_factory definition

}  // end eicrecon namespace
