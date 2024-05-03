// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#include <random>

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/utils/vector_utils.h>

#include "PIDLookupConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "services/pid_lut/PIDLookupTable_service.h"

class PIDLookupTable_service;

namespace eicrecon {

using PIDLookupAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection,
                                            edm4eic::MCRecoParticleAssociationCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection, edm4hep::ParticleIDCollection>>;

class PIDLookup : public PIDLookupAlgorithm, public WithPodConfig<PIDLookupConfig> {

public:
  PIDLookup(std::string_view name)
      : PIDLookupAlgorithm{name,
                           {"inputParticlesCollection", "inputParticleAssociationsCollection"},
                           {"outputParticlesCollection"},
                           ""} {}

  void init(PIDLookupTable_service&);
  void process(const Input&, const Output&) const final;

private:
  mutable std::mt19937 m_gen{};
  mutable std::uniform_real_distribution<double> m_dist{0, 1};
  const PIDLookupTable* m_lut;
};

} // namespace eicrecon