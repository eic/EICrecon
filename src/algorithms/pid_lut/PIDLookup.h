// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <stdint.h>
#include <gsl/pointers>
#include <string>
#include <string_view>

#include "PIDLookupConfig.h"
#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "services/pid_lut/PIDLookupTable.h"

namespace eicrecon {

using PIDLookupAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::EventHeaderCollection, edm4eic::ReconstructedParticleCollection,
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
  int32_t m_system;
  const algorithms::UniqueIDGenSvc& m_uid      = algorithms::UniqueIDGenSvc::instance();
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  const PIDLookupTable* m_lut;
};

} // namespace eicrecon
