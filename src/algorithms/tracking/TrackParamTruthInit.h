// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <memory>
#include <random>
#include <string>
#include <string_view>

#include "ActsGeometryProvider.h"
#include "TrackParamTruthInitConfig.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TrackParamTruthInitAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::MCParticleCollection>,
                          algorithms::Output<edm4eic::TrackParametersCollection>>;

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
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc{m_actsSvc.acts_geometry_provider()};

  const algorithms::ParticleSvc& m_particleSvc{algorithms::ParticleSvc::instance()};

  mutable std::default_random_engine generator; // TODO: need something more appropriate here
  mutable std::normal_distribution<double> m_normDist;
};

} // namespace eicrecon
