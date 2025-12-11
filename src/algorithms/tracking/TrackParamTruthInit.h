// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <memory>
#include <string>
#include <string_view>

#include "TrackParamTruthInitConfig.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "services/particle/ParticleSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

// Forward declaration
namespace eicrecon {
class ActsDD4hepDetector;
}

namespace eicrecon {

using TrackParamTruthInitAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::EventHeaderCollection, edm4hep::MCParticleCollection>,
    algorithms::Output<edm4eic::TrackSeedCollection, edm4eic::TrackParametersCollection>>;

class TrackParamTruthInit : public TrackParamTruthInitAlgorithm,
                            public WithPodConfig<TrackParamTruthInitConfig> {

public:
  TrackParamTruthInit(std::string_view name)
      : TrackParamTruthInitAlgorithm{name,
                                     {"inputMCParticles"},
                                     {"outputTrackParameters"},
                                     "create track seeds from truth information"} {}

  void init() final {};

  void process(const Input& input, const Output& output) const final;

private:
  const algorithms::ActsSvc& m_actsSvc{algorithms::ActsSvc::instance()};
  std::shared_ptr<const eicrecon::ActsDD4hepDetector> m_acts_detector{m_actsSvc.detector()};

  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  const algorithms::UniqueIDGenSvc& m_uid      = algorithms::UniqueIDGenSvc::instance();
};

} // namespace eicrecon
