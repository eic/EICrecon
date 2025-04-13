//
// Copyright 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/pid/IrtDebugging.h"
#include "extensions/jana/JOmniFactory.h"

class CherenkovDetector;

#include <edm4eic/RawTrackerHitCollection.h>

namespace eicrecon {

class IrtDebugging_factory :
    public JOmniFactory<IrtDebugging_factory, IrtDebuggingConfig> {

private:
  using AlgoT = eicrecon::IrtDebugging;
    std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::MCParticle> m_mc_particles_input {this};
  PodioInput<edm4eic::ReconstructedParticle> m_recoparticles_input {this};
  PodioInput<edm4eic::MCRecoParticleAssociation> m_recoassocs_input {this};
  PodioInput<edm4eic::TrackSegment> m_aerogel_tracks_input {this};
  PodioInput<edm4hep::SimTrackerHit> m_sim_hits_input {this};
  
  PodioOutput<edm4eic::IrtDebugInfo> m_irt_debug_info_output {this};
  
  Service<DD4hep_service> m_dd4hep_service {this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    
    m_algo->init(m_dd4hep_service(), config(), logger());
  }
  
  void ChangeRun(int64_t run_number) {
  }
  
  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({
	m_mc_particles_input(),
	m_recoparticles_input(),
	m_recoassocs_input(),
	m_aerogel_tracks_input(),
	m_sim_hits_input()
      },
      {
	m_irt_debug_info_output().get()
      }
      );
  }
};

} // eicrecon
