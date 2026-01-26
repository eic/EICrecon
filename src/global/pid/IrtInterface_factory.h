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

#include "algorithms/pid/IrtInterface.h"
#include "extensions/jana/JOmniFactory.h"

namespace IRT2 {
class CherenkovDetector;
};
#include <edm4eic/RawTrackerHitCollection.h>

namespace eicrecon {

class IrtInterface_factory : public JOmniFactory<IrtInterface_factory, IrtConfig> {

private:
  using AlgoT = eicrecon::IrtInterface;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::MCParticle> m_mc_particles_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_recoparticles_input{this};
  PodioInput<edm4eic::MCRecoParticleAssociation> m_recoassocs_input{this};
  PodioInput<edm4eic::TrackSegment> m_track_projections_input{this};
  PodioInput<edm4hep::SimTrackerHit> m_sim_hits_input{this};

  PodioOutput<edm4eic::IrtRadiatorInfo> m_irt_radiator_output{this};
  PodioOutput<edm4eic::IrtParticle> m_irt_particle_output{this};
  PodioOutput<edm4eic::IrtEvent> m_irt_event_output{this};

  Service<DD4hep_service> m_dd4hep_service{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));

    m_algo->init(m_dd4hep_service(), config(), logger());
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process(
        {m_mc_particles_input(), m_recoparticles_input(), m_recoassocs_input(),
         m_track_projections_input(), m_sim_hits_input()},
        {m_irt_radiator_output().get(), m_irt_particle_output().get(), m_irt_event_output().get()});
  }
};

} // namespace eicrecon
