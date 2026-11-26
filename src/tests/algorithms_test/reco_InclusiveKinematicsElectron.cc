// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <cmath>
#include <memory>

#include "algorithms/reco/InclusiveKinematicsElectron.h"

using Catch::Matchers::WithinRel;
using ROOT::Math::PxPyPzEVector;

namespace {

struct Event {
  edm4hep::MCParticleCollection beam_electrons;
  edm4hep::MCParticleCollection beam_hadrons;
  edm4eic::ReconstructedParticleCollection escat;
  edm4eic::HadronicFinalStateCollection hfs;

  Event() {
    // 18 GeV electrons on 275 GeV protons, with -25 mrad crossing angle
    auto ei = beam_electrons.create();
    ei.setPDG(11);
    ei.setMomentum({0., 0., -18.});
    ei.setMass(0.000510998928);
    auto pi = beam_hadrons.create();
    pi.setPDG(2212);
    pi.setMomentum({275. * std::sin(-0.025), 0., 275. * std::cos(-0.025)});
    pi.setMass(0.93827);
  }

  void add_scattered_electron(float px, float py, float pz, float E) {
    auto e = escat.create();
    e.setMomentum({px, py, pz});
    e.setEnergy(E);
    e.setCharge(-1);
  }

  std::unique_ptr<edm4eic::InclusiveKinematicsCollection> run() const {
    auto kin = std::make_unique<edm4eic::InclusiveKinematicsCollection>();
    eicrecon::InclusiveKinematicsElectron algo("InclusiveKinematicsElectron");
    algo.level(algorithms::LogLevel::kDebug);
    algo.init();
    algo.process({&beam_electrons, &beam_hadrons, &escat, &hfs}, {kin.get()});
    return kin;
  }
};

} // namespace

TEST_CASE("Electron method: physical event", "[InclusiveKinematicsElectron]") {
  Event event;
  // Electron scattered backward with moderate y
  event.add_scattered_electron(5., 3., -12., std::sqrt(25. + 9. + 144.));
  const auto kin = event.run();
  REQUIRE(kin->size() == 1);
  const auto& k = (*kin)[0];
  CHECK(std::isfinite(k.getW()));
  CHECK(k.getX() > 0);
  CHECK(k.getX() < 1);
  CHECK(k.getY() > 0);
  CHECK(k.getY() < 1);
  // W^2 = M^2 + Q^2 (1 - x) / x
  const double m_p = 0.93827;
  CHECK_THAT(k.getW() * k.getW(),
             WithinRel(m_p * m_p + k.getQ2() * (1. - k.getX()) / k.getX(), 1e-4));
}

TEST_CASE("Electron method: x > 1 from low-y resolution is skipped",
          "[InclusiveKinematicsElectron]") {
  // Reconstructed scattered electron of a pythia8 NC DIS 18x275 event (true x = 0.70,
  // y = 0.023) whose energy is measured 1.6% high, yielding x = 2.2 and W^2 < 0
  Event event;
  event.add_scattered_electron(-8.661763F, -15.750382F, -13.479928F, 22.467957F);
  const auto kin = event.run();
  CHECK(kin->empty());
}

TEST_CASE("Electron method: y < 0 is skipped", "[InclusiveKinematicsElectron]") {
  // E - pz > 2 E_beam
  Event event;
  event.add_scattered_electron(1., 0., -20., std::sqrt(1. + 400.));
  const auto kin = event.run();
  CHECK(kin->empty());
}
