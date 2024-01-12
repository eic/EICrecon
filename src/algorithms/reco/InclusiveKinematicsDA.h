// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>


namespace eicrecon {

  using InclusiveKinematicsDAAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4hep::MCParticleCollection,
      edm4eic::ReconstructedParticleCollection,
      edm4eic::MCRecoParticleAssociationCollection
    >,
    algorithms::Output<
      edm4eic::InclusiveKinematicsCollection
    >
  >;

  class InclusiveKinematicsDA
  : public InclusiveKinematicsDAAlgorithm {

  public:
    InclusiveKinematicsDA(std::string_view name)
      : InclusiveKinematicsDAAlgorithm{name,
                            {"MCParticles", "inputParticles", "inputAssociations"},
                            {"inclusiveKinematics"},
                            "Determine inclusive kinematics using double-angle method."} {}

    void init(std::shared_ptr<spdlog::logger>& logger);
    void process(const Input&, const Output&) const final;

  private:
    std::shared_ptr<spdlog::logger> m_log;
    double m_proton{0.93827}, m_neutron{0.93957}, m_electron{0.000510998928}, m_crossingAngle{-0.025};
  };

} // namespace eicrecon
