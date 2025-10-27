// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/TrackParametersCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "services/geometry/acts/ACTSGeo_service.h"
#include "algorithms/tracking/TrackParamTruthInit.h"
#include "algorithms/tracking/TrackParamTruthInitConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class TrackParamTruthInit_factory
    : public JOmniFactory<TrackParamTruthInit_factory, TrackParamTruthInitConfig> {

private:
  using AlgoT = eicrecon::TrackParamTruthInit;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::EventHeader> m_headers_input{this};
  PodioInput<edm4hep::MCParticle> m_particles_input{this};
  PodioOutput<edm4eic::TrackParameters> m_parameters_output{this};

  ParameterRef<double> m_maxVertexX{this, "MaxVertexX", config().maxVertexX,
                                    "Maximum abs(vertex x) for truth tracks turned into seed"};
  ParameterRef<double> m_maxVertexY{this, "MaxVertexY", config().maxVertexY,
                                    "Maximum abs(vertex y) for truth tracks turned into seed"};
  ParameterRef<double> m_maxVertexZ{this, "MaxVertexZ", config().maxVertexZ,
                                    "Maximum abs(vertex z) for truth tracks turned into seed"};
  ParameterRef<double> m_minMomentum{this, "MinMomentum", config().minMomentum,
                                     "Minimum momentum for truth tracks turned into seed"};
  ParameterRef<double> m_maxEtaForward{
      this, "MaxEtaForward", config().maxEtaForward,
      "Maximum forward abs(eta) for truth tracks turned into seed"};
  ParameterRef<double> m_maxEtaBackward{
      this, "MaxEtaBackward", config().maxEtaBackward,
      "Maximum backward abs(eta) for truth tracks turned into seed"};
  ParameterRef<double> m_momentumSmear{
      this, "MomentumSmear", config().momentumSmear,
      "Momentum magnitude fraction to use as width of gaussian smearing"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};
  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_headers_input(), m_particles_input()}, {m_parameters_output().get()});
  }
};

} // namespace eicrecon
