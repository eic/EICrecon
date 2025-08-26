// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#include "services/geometry/dd4hep/DD4hep_service.h"

// Event Model related classes
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <algorithms/fardetectors/FarDetectorLinearTracking.h>

#include <extensions/jana/JOmniFactory.h>
#include <spdlog/logger.h>

namespace eicrecon {

class FarDetectorLinearTracking_factory
    : public JOmniFactory<FarDetectorLinearTracking_factory, FarDetectorLinearTrackingConfig> {

public:
  using AlgoT = eicrecon::FarDetectorLinearTracking;

private:
  std::unique_ptr<AlgoT> m_algo;

  VariadicPodioInput<edm4eic::Measurement2D> m_hits_input{this};
  PodioInput<edm4eic::MCRecoTrackerHitAssociation> m_hits_association_input{this};
  PodioOutput<edm4eic::Track> m_tracks_output{this};
  PodioOutput<edm4eic::MCRecoTrackParticleAssociation> m_tracks_association_output{this};

  ParameterRef<std::size_t> n_layer{this, "numLayers", config().n_layer};
  ParameterRef<std::size_t> layer_hits_max{this, "layerHitsMax", config().layer_hits_max};
  ParameterRef<float> chi2_max{this, "chi2Max", config().chi2_max};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {

    try {
      std::vector<gsl::not_null<const edm4eic::Measurement2DCollection*>> hits;
      for (const auto& hit : m_hits_input()) {
        hits.push_back(gsl::not_null<const edm4eic::Measurement2DCollection*>{hit});
      }

      // Prepare the input tuple
      auto input = std::make_tuple(hits, m_hits_association_input());

      m_algo->process(input, {m_tracks_output().get(), m_tracks_association_output().get()});
    } catch (std::exception& e) {
      throw JException(e.what());
    }
  }
};

} // namespace eicrecon
