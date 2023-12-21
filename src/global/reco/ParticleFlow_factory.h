// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) Derek Anderson

#pragma once

#include <services/geometry/dd4hep/DD4hep_service.h>
#include <extensions/jana/JOmniFactory.h>
#include "algorithms/reco/ParticleFlow.h"

namespace eicrecon {

  class ParticleFlow_factory : public JOmniFactory<ParticleFlow_factory, ParticleFlowConfig> {

    private:

      // algorithm to run
      using AlgoT = eicrecon::ParticleFlow;
      std::unique_ptr<AlgoT> m_algo;

      // input collections
      PodioInput<edm4eic::ReconstructedParticle> m_tracks_input {this};
      PodioInput<edm4eic::TrackSegment> m_track_projections_input {this};
      PodioInput<edm4eic::Cluster> m_ecal_input {this};
      PodioInput<edm4eic::Cluster> m_hcal_input {this};

      // output collections
      PodioOutput<edm4eic::ReconstructedParticle> m_particle_output {this};

      // parameter bindings
      ParameterRef<uint8_t> m_flowAlgo {this, "flowAlgo", config().flowAlgo};
      ParameterRef<std::string> m_ecalDetName {this, "ecalDetName", config().ecalDetName};
      ParameterRef<std::string> m_hcalDetName {this, "hcalDetName", config().hcalDetName};
      ParameterRef<float> m_ecalSumRadius {this, "ecalSumRadius", config().ecalSumRadius};
      ParameterRef<float> m_hcalSumRadius {this, "hcalSumRadius", config().hcalSumRadius};
      ParameterRef<float> m_ecalFracSub {this, "ecalFracSub", config().ecalFracSub};
      ParameterRef<float> m_hcalFracSub {this, "hcalFracSub", config().hcalFracSub};

      // geometry service
      Service<DD4hep_service> m_geoSvc {this};

    public:

      void Configure() {
        // TODO the following line might become relevant when I switch
        // over to inherting from algorithm...
        //m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo = std::make_unique<AlgoT>();
        m_algo -> applyConfig(config());
        m_algo -> init(m_geoSvc().detector(), logger());
      }

      void ChangeRun(int64_t run_number) {
        /* nothing to do here */
      }

      void Process(int64_t run_number, int64_t event_number) {
        m_particle_output() = m_algo -> process(
          m_tracks_input(),
          m_track_projections_input(),
          m_ecal_input(),
          m_hcal_input()
        );
      }

  };  // end ParticleFlow_factory definition

}  // end eicrecon namespace
