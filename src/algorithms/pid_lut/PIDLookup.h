// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <random>
#include <string>
#include <string_view>

#include "PIDLookupConfig.h"
#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "services/pid_lut/PIDLookupTable.h"

namespace eicrecon {

using PIDLookupAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection,
                                            edm4eic::MCRecoParticleAssociationCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection,
                                             edm4eic::MCRecoParticleAssociationCollection,
                                             edm4hep::ParticleIDCollection>>;

class PIDLookup : public PIDLookupAlgorithm, public WithPodConfig<PIDLookupConfig> {

public:
  PIDLookup(std::string_view name)
      : PIDLookupAlgorithm{name,
                           {"inputParticlesCollection", "inputParticleAssociationsCollection"},
                           {"outputParticlesCollection", "outputParticleAssociationsCollection",
                            "outputParticleIDCollection"},
                           ""} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  mutable std::mt19937 m_gen{};
  mutable std::uniform_real_distribution<double> m_dist{0, 1};
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  const PIDLookupTable* m_lut;
};

} // namespace eicrecon
