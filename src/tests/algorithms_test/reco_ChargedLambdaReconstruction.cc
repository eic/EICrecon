// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Dmitry Romanov

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <cmath>

#include "algorithms/reco/ChargedLambdaReconstruction.h"
#include "algorithms/reco/ChargedLambdaReconstructionConfig.h"

using eicrecon::ChargedLambdaReconstruction;
using eicrecon::ChargedLambdaReconstructionConfig;

namespace {

constexpr double M_LAMBDA = 1.115683;
constexpr double M_PROTON = 0.9382720813;
constexpr double M_PION   = 0.13957039;

struct DaughterMomenta {
  edm4hep::Vector3f proton;
  edm4hep::Vector3f pion;
};

/** Two-body decay Lambda -> p pi- of a Lambda moving along +z with momentum
 * lambda_p [GeV]; theta_star is the proton polar angle in the Lambda rest frame,
 * the decay plane is x-z. */
DaughterMomenta lambdaDecayDaughters(double lambda_p, double theta_star) {
  const double e_lambda   = std::hypot(lambda_p, M_LAMBDA);
  const double gamma      = e_lambda / M_LAMBDA;
  const double beta_gamma = lambda_p / M_LAMBDA;

  const double m2 = M_LAMBDA * M_LAMBDA;
  const double q =
      std::sqrt((m2 - std::pow(M_PROTON + M_PION, 2)) * (m2 - std::pow(M_PROTON - M_PION, 2))) /
      (2 * M_LAMBDA);
  const double e_p_star  = std::hypot(q, M_PROTON);
  const double e_pi_star = std::hypot(q, M_PION);

  const double qz = q * std::cos(theta_star);
  const double qx = q * std::sin(theta_star);

  return {{static_cast<float>(qx), 0.F, static_cast<float>(gamma * qz + beta_gamma * e_p_star)},
          {static_cast<float>(-qx), 0.F, static_cast<float>(-gamma * qz + beta_gamma * e_pi_star)}};
}

void setupAlgo(ChargedLambdaReconstruction& algo,
               const ChargedLambdaReconstructionConfig& cfg = {}) {
  algo.level(algorithms::LogLevel::kDebug);
  algo.applyConfig(cfg);
  algo.init();
}

} // namespace

TEST_CASE("ChargedLambdaReconstruction finds a true p pi- pair", "[ChargedLambdaReconstruction]") {
  ChargedLambdaReconstruction algo("ChargedLambdaReconstruction");
  setupAlgo(algo);

  const edm4hep::Vector3f decay_vertex{0.F, 0.F, 6000.F}; // [mm], meters downstream of the IP
  const auto daughters = lambdaDecayDaughters(100.0, M_PI / 2);

  // proton comes in through the Roman-Pot input, pion through tracking
  edm4eic::ReconstructedParticleCollection charged;
  edm4eic::ReconstructedParticleCollection roman_pots;
  edm4eic::ReconstructedParticleCollection off_momentum;

  auto proton = roman_pots.create();
  proton.setCharge(1);
  proton.setMomentum(daughters.proton);
  proton.setReferencePoint(decay_vertex);

  auto pion = charged.create();
  pion.setCharge(-1);
  pion.setMomentum(daughters.pion);
  pion.setReferencePoint(decay_vertex);

  edm4eic::ReconstructedParticleCollection lambdas;
  algo.process({&charged, &roman_pots, &off_momentum}, {&lambdas});

  REQUIRE(lambdas.size() == 1);
  const auto lambda = lambdas[0];
  CHECK(lambda.getPDG() == 3122);
  CHECK(lambda.getCharge() == 0);
  // tolerance is dominated by the float storage of ~85 GeV daughter momenta
  CHECK_THAT(lambda.getMass(), Catch::Matchers::WithinAbs(M_LAMBDA, 2e-3));
  CHECK_THAT(lambda.getReferencePoint().z, Catch::Matchers::WithinAbs(decay_vertex.z, 1.0));
  REQUIRE(lambda.particles_size() == 2);
  CHECK(lambda.getParticles(0) == proton);
  CHECK(lambda.getParticles(1) == pion);
}

TEST_CASE("ChargedLambdaReconstruction rejects a wide-angle pairing",
          "[ChargedLambdaReconstruction]") {
  ChargedLambdaReconstruction algo("ChargedLambdaReconstruction");
  setupAlgo(algo);

  edm4eic::ReconstructedParticleCollection charged;
  edm4eic::ReconstructedParticleCollection roman_pots;
  edm4eic::ReconstructedParticleCollection off_momentum;

  auto proton = roman_pots.create();
  proton.setCharge(1);
  proton.setMomentum({0.F, 0.F, 80.F});
  proton.setReferencePoint({0.F, 0.F, 0.F});

  // ~100 mrad away from the proton: no forward Lambda decays this wide
  auto pion = charged.create();
  pion.setCharge(-1);
  pion.setMomentum({1.F, 0.F, 10.F});
  pion.setReferencePoint({0.F, 0.F, 0.F});

  edm4eic::ReconstructedParticleCollection lambdas;
  algo.process({&charged, &roman_pots, &off_momentum}, {&lambdas});

  CHECK(lambdas.empty());
}

TEST_CASE("ChargedLambdaReconstruction pairs only opposite charges",
          "[ChargedLambdaReconstruction]") {
  ChargedLambdaReconstruction algo("ChargedLambdaReconstruction");
  setupAlgo(algo);

  const auto daughters = lambdaDecayDaughters(100.0, M_PI / 2);

  edm4eic::ReconstructedParticleCollection charged;
  edm4eic::ReconstructedParticleCollection roman_pots;
  edm4eic::ReconstructedParticleCollection off_momentum;

  auto proton = roman_pots.create();
  proton.setCharge(1);
  proton.setMomentum(daughters.proton);
  proton.setReferencePoint({0.F, 0.F, 6000.F});

  // same kinematics as a real pion daughter, but reconstructed as positive:
  // it must not enter the pion pool
  auto not_a_pion = charged.create();
  not_a_pion.setCharge(1);
  not_a_pion.setMomentum(daughters.pion);
  not_a_pion.setReferencePoint({0.F, 0.F, 6000.F});

  edm4eic::ReconstructedParticleCollection lambdas;
  algo.process({&charged, &roman_pots, &off_momentum}, {&lambdas});

  CHECK(lambdas.empty());
}

TEST_CASE("ChargedLambdaReconstruction honors the configured mass window",
          "[ChargedLambdaReconstruction]") {
  ChargedLambdaReconstructionConfig cfg;
  cfg.massWindow = 1e-9; // narrower than the float-precision kinematics of the test pair
  ChargedLambdaReconstruction algo("ChargedLambdaReconstruction");
  setupAlgo(algo, cfg);

  const auto daughters = lambdaDecayDaughters(100.0, M_PI / 2);

  edm4eic::ReconstructedParticleCollection charged;
  edm4eic::ReconstructedParticleCollection roman_pots;
  edm4eic::ReconstructedParticleCollection off_momentum;

  auto proton = roman_pots.create();
  proton.setCharge(1);
  proton.setMomentum(daughters.proton);
  proton.setReferencePoint({0.F, 0.F, 6000.F});

  auto pion = charged.create();
  pion.setCharge(-1);
  pion.setMomentum(daughters.pion);
  pion.setReferencePoint({0.F, 0.F, 6000.F});

  edm4eic::ReconstructedParticleCollection lambdas;
  algo.process({&charged, &roman_pots, &off_momentum}, {&lambdas});

  CHECK(lambdas.empty());
}
