// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include "extensions/jana/JOmniFactory.h"

#include "detectors/BTOF/BTOFChargeSharing.h"
#include <iostream>

namespace eicrecon {

class BTOFChargeSharing_factory : public JOmniFactory<BTOFChargeSharing_factory, TOFHitDigiConfig> {
private:
  // Underlying algorithm
  std::unique_ptr<eicrecon::BTOFChargeSharing> m_algo;

  // Declare inputs
  PodioInput<edm4hep::SimTrackerHit> m_in_sim_track{this}; //, "TOFBarrelRawHits"};

  // Declare outputs
  PodioOutput<edm4hep::SimTrackerHit> m_out_reco_particles{this};

  // Declare services here, e.g.
  Service<DD4hep_service> m_geoSvc{this};

  ParameterRef<double> m_sigma_sharingx{this, "sigma_sharingx", config().sigma_sharingx};
  ParameterRef<double> m_sigma_sharingy{this, "sigma_sharingy", config().sigma_sharingy};
public:
  void Configure() {
    // This is called when the factory is instantiated.
    // Use this callback to make sure the algorithm is configured.
    // The logger, parameters, and services have all been fetched before this is called
    m_algo = std::make_unique<eicrecon::BTOFChargeSharing>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    // Pass config object to algorithm
    m_algo->applyConfig(config());

    // If we needed geometry, we'd obtain it like so
    // m_algo->init(m_geoSvc().detector(), m_geoSvc().converter(), logger());
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {
    // This is called whenever the run number is changed.
    // Use this callback to retrieve state that is keyed off of run number.
    // This state should usually be managed by a Service.
    // Note: You usually don't need this, because you can declare a Resource instead.
  }

  void Process(int64_t run_number, uint64_t event_number) {
    // This is called on every event.
    // Use this callback to call your Algorithm using all inputs and outputs
    // The inputs will have already been fetched for you at this point.
    m_algo->process({m_in_sim_track()}, {m_out_reco_particles().get()});
    // JANA will take care of publishing the outputs for you.
  }
};
} // namespace eicrecon
