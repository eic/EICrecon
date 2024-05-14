// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>


namespace eicrecon {

  using InclusiveKinematicseSigmaAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4hep::MCParticleCollection,
      edm4eic::ReconstructedParticleCollection,
      edm4eic::HadronicFinalStateCollection
    >,
    algorithms::Output<
      edm4eic::InclusiveKinematicsCollection
    >
  >;

  class InclusiveKinematicseSigma
  : public InclusiveKinematicseSigmaAlgorithm {

  public:
    InclusiveKinematicseSigma(std::string_view name)
      : InclusiveKinematicseSigmaAlgorithm{name,
                            {"MCParticles", "scatteredElectron", "hadronicFinalState"},
                            {"inclusiveKinematics"},
                            "Determine inclusive kinematics using e-Sigma method."} {}

    void init(std::shared_ptr<spdlog::logger>& logger);
    void process(const Input&, const Output&) const final;

  private:
    std::shared_ptr<spdlog::logger> m_log;
    double m_proton{0.93827}, m_neutron{0.93957}, m_electron{0.000510998928}, m_crossingAngle{-0.025};
  };

} // namespace eicrecon
