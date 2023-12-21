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
      // TODO split eta regions into separate factories
      // in reco.cc
      PodioInput<edm4eic::ReconstructedParticle> m_tracks_input {this};
      PodioInput<edm4eic::TrackSegment> m_track_projections_input {this};
      PodioInput<edm4eic::Cluster> m_negative_ecal_input {this};
      PodioInput<edm4eic::Cluster> m_negative_hcal_input {this};
      PodioInput<edm4eic::Cluster> m_central_ecal_input {this};
      PodioInput<edm4eic::Cluster> m_central_hcal_input {this};
      PodioInput<edm4eic::Cluster> m_positive_ecal_input {this};
      PodioInput<edm4eic::Cluster> m_positive_hcal_input {this};

      // output collections
      PodioOutput<edm4eic::ReconstructedParticle> m_particle_output {this};

      // parameter bindings
      // TODO split eta regions into separate factories
      // in reco.cc
      ParameterRef<std::vector<int>> m_flowAlgo {this, "flowAlgo", config().flowAlgo};
      ParameterRef<std::vector<std::string>> m_ecalDetName {this, "ecalDetName", config().ecalDetName};
      ParameterRef<std::vector<std::string>> m_hcalDetName {this, "hcalDetName", config().hcalDetName};
      ParameterRef<std::vector<float>> m_ecalSumRadius {this, "ecalSumRadius", config().ecalSumRadius};
      ParameterRef<std::vector<float>> m_hcalSumRadius {this, "hcalSumRadius", config().hcalSumRadius};
      ParameterRef<std::vector<float>> m_ecalFracSub {this, "ecalFracSub", config().ecalFracSub};
      ParameterRef<std::vector<float>> m_hcalFracSub {this, "hcalFracSub", config().hcalFracSub};

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
        // TODO split eta regions into separate factories
        // in reco.cc
        m_particle_output() = m_algo -> process(
          m_tracks_input(),
          m_track_projections_input(),
          m_negative_ecal_input(),
          m_negative_hcal_input(),
          m_central_ecal_input(),
          m_central_hcal_input(),
          m_positive_ecal_input(),
          m_positive_hcal_input()
        );
      }

  };  // end ParticleFlow_factory definition

}  // end eicrecon namespace
