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
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <vector>

#include "algorithms/reco/HadronicFinalState.h"
#include "algorithms/reco/InclusiveKinematicsJB.h"
#include "algorithms/reco/ScatteredElectronsTruth.h"

using Catch::Matchers::WithinRel;
using ROOT::Math::PxPyPzEVector;

namespace {

// Masses as in ParticleSvc
const double m_e = 0.000510998928;
const double m_p = 0.93827;

PxPyPzEVector electron(double px, double py, double pz) {
  return {px, py, pz, std::sqrt(px * px + py * py + pz * pz + m_e * m_e)};
}
PxPyPzEVector massless(double px, double py, double pz) {
  return {px, py, pz, std::sqrt(px * px + py * py + pz * pz)};
}

// Beams: 18 GeV electrons on 275 GeV protons
const PxPyPzEVector k = electron(0, 0, -18);
const PxPyPzEVector P{0, 0, 275, std::sqrt(275. * 275. + m_p * m_p)};

// Generator record built by hand, with a reconstructed particle and an association for every
// final-state particle except neutrinos
struct Event {
  edm4hep::MCParticleCollection beam_electrons;
  edm4hep::MCParticleCollection beam_hadrons;
  edm4hep::MCParticleCollection parts;
  edm4eic::ReconstructedParticleCollection rcparts;
  edm4eic::MCRecoParticleAssociationCollection rcassoc;

  Event() {
    beam_electrons.setSubsetCollection();
    beam_hadrons.setSubsetCollection();
  }

  edm4hep::MutableMCParticle add(int32_t pdg, int32_t status, const PxPyPzEVector& p,
                                 float charge = 0) {
    auto mc = parts.create();
    mc.setPDG(pdg);
    mc.setGeneratorStatus(status);
    mc.setMomentum({p.Px(), p.Py(), p.Pz()});
    mc.setMass(p.M());
    mc.setCharge(charge);
    if (status == 4) {
      (std::abs(pdg) == 11 ? beam_electrons : beam_hadrons).push_back(mc);
    }
    const bool neutrino = (std::abs(pdg) == 12 || std::abs(pdg) == 14 || std::abs(pdg) == 16);
    if (status == 1 && !neutrino) {
      auto rc = rcparts.create();
      rc.setPDG(pdg);
      rc.setMomentum(
          {static_cast<float>(p.Px()), static_cast<float>(p.Py()), static_cast<float>(p.Pz())});
      rc.setEnergy(p.E());
      rc.setCharge(charge);
      auto assoc = rcassoc.create();
      assoc.setRec(rc);
      assoc.setSim(mc);
      assoc.setWeight(1.);
    }
    return mc;
  }

  static void link(edm4hep::MutableMCParticle parent, edm4hep::MutableMCParticle daughter) {
    parent.addToDaughters(daughter);
    daughter.addToParents(parent);
  }

  // Reconstructed particle associated with a generator particle
  podio::ObjectID reco_of(const edm4hep::MCParticle& mc) const {
    for (const auto& assoc : rcassoc) {
      if (assoc.getSim().getObjectID() == mc.getObjectID()) {
        return assoc.getRec().getObjectID();
      }
    }
    return {};
  }
};

struct Results {
  std::unique_ptr<edm4eic::ReconstructedParticleCollection> electrons =
      std::make_unique<edm4eic::ReconstructedParticleCollection>();
  std::unique_ptr<edm4eic::HadronicFinalStateCollection> hfs =
      std::make_unique<edm4eic::HadronicFinalStateCollection>();
  std::unique_ptr<edm4eic::InclusiveKinematicsCollection> jb =
      std::make_unique<edm4eic::InclusiveKinematicsCollection>();
};

Results run(const Event& event) {
  Results r;

  eicrecon::ScatteredElectronsTruth scat("ScatteredElectronsTruth");
  scat.level(algorithms::LogLevel::kTrace);
  scat.init();
  scat.process({&event.parts, &event.rcparts, &event.rcassoc}, {r.electrons.get()});

  eicrecon::HadronicFinalState hfs("HadronicFinalState");
  hfs.level(algorithms::LogLevel::kDebug);
  hfs.init();
  hfs.process(
      {&event.beam_electrons, &event.beam_hadrons, &event.parts, &event.rcparts, &event.rcassoc},
      {r.hfs.get()});

  eicrecon::InclusiveKinematicsJB jb("InclusiveKinematicsJB");
  jb.level(algorithms::LogLevel::kDebug);
  jb.init();
  jb.process({&event.beam_electrons, &event.beam_hadrons, r.electrons.get(), r.hfs.get()},
             {r.jb.get()});

  return r;
}

} // namespace

TEST_CASE("Truth scattered electron: Pythia8 with a Dalitz electron listed first",
          "[ScatteredElectronsTruth][HadronicFinalState]") {
  // Lepton line 4 -> 42 -> 21 -> 23 -> 44 -> 1, with a pi0 -> e+ e- gamma Dalitz decay
  // written before the final-state scattered electron
  const auto e23 = electron(3, 1, -10);
  const auto e44 = electron(3.05, 0.98, -10.02);

  Event event;
  auto p_p   = event.add(2212, 4, P, 1);
  auto e4_p  = event.add(11, 4, k, -1);
  auto e42_p = event.add(11, 42, k, -1);
  auto e21_p = event.add(11, 21, k, -1);
  auto q21_p = event.add(2, 21, massless(0, 0, 50));
  auto e23_p = event.add(11, 23, e23, -1);
  auto q23_p = event.add(2, 23, massless(-3, -1, 41));
  auto e44_p = event.add(11, 44, e44, -1);
  auto pi0_p = event.add(111, 2, PxPyPzEVector{-1, -0.5, 6, std::sqrt(37.25 + 0.135 * 0.135)});
  auto dal_e = event.add(11, 1, electron(-0.4, -0.2, 2.5), -1);
  auto dal_p = event.add(-11, 1, electron(-0.5, -0.25, 3), 1);
  auto dal_g = event.add(22, 1, massless(-0.1, -0.05, 0.5));
  auto e1_p  = event.add(11, 1, e44, -1);
  Event::link(p_p, q21_p);
  Event::link(e4_p, e42_p);
  Event::link(e42_p, e21_p);
  for (auto in : {e21_p, q21_p}) {
    for (auto out : {e23_p, q23_p}) {
      Event::link(in, out);
    }
  }
  Event::link(e23_p, e44_p);
  Event::link(e44_p, e1_p);
  Event::link(q23_p, pi0_p);
  for (auto d : {dal_e, dal_p, dal_g}) {
    Event::link(pi0_p, d);
  }

  const auto r = run(event);

  // The scattered electron is the end of the lepton line, not the Dalitz electron
  REQUIRE(r.electrons->size() == 1);
  CHECK((*r.electrons)[0].getObjectID() == event.reco_of(e1_p));

  // The Dalitz electron is part of the hadronic final state, the scattered electron is not
  REQUIRE(r.hfs->size() == 1);
  const auto hadrons = (*r.hfs)[0].getHadrons();
  REQUIRE(hadrons.size() == 3);
  for (const auto& h : hadrons) {
    CHECK(h.getObjectID() != event.reco_of(e1_p));
  }
  CHECK(std::ranges::any_of(
      hadrons, [&](const auto& h) { return h.getObjectID() == event.reco_of(dal_e); }));
}

TEST_CASE("Truth scattered electron: positron beam", "[ScatteredElectronsTruth]") {
  // An unrelated final-state electron is listed before the scattered positron
  const auto ef = electron(3, 1, -10);

  Event event;
  event.add(2212, 4, P, 1);
  auto ei_p = event.add(-11, 4, k, 1);
  event.add(11, 1, electron(0.2, 0.1, 1), -1);
  auto ef_p = event.add(-11, 1, ef, 1);
  Event::link(ei_p, ef_p);

  const auto r = run(event);
  REQUIRE(r.electrons->size() == 1);
  CHECK((*r.electrons)[0].getObjectID() == event.reco_of(ef_p));
  CHECK((*r.electrons)[0].getPDG() == -11);
}

TEST_CASE("Truth scattered electron: charged current",
          "[ScatteredElectronsTruth][HadronicFinalState]") {
  // Lepton line ends in a neutrino; a final-state electron from a hadron decay is not the
  // scattered lepton. The proton beam has the crossing angle assumed by HadronicFinalState.
  const double crossing_angle = -0.025;
  const PxPyPzEVector P_x{275. * std::sin(crossing_angle), 0, 275. * std::cos(crossing_angle),
                          P.E()};
  const auto nu  = massless(-11, 23, 150);
  const auto q   = k - nu;
  const auto dec = electron(0.2, 0.1, 1);

  Event event;
  auto ei_p = event.add(11, 4, k, -1);
  event.add(2212, 4, P_x, 1);
  auto nu_p = event.add(12, 1, nu);
  Event::link(ei_p, nu_p);
  auto dec_p = event.add(11, 1, dec, -1);
  // Rest of the hadronic final state, as a single particle
  auto x_p = event.add(2212, 1, P_x + q - dec, 1);

  const auto r = run(event);
  CHECK(r.electrons->empty());

  // All reconstructed particles are in the hadronic final state
  REQUIRE(r.hfs->size() == 1);
  const auto hadrons = (*r.hfs)[0].getHadrons();
  REQUIRE(hadrons.size() == 2);
  CHECK(hadrons[0].getObjectID() == event.reco_of(dec_p));
  CHECK(hadrons[1].getObjectID() == event.reco_of(x_p));

  // Jacquet-Blondel kinematics, without scattered electron
  REQUIRE(r.jb->size() == 1);
  CHECK_FALSE((*r.jb)[0].getScat().isAvailable());
  CHECK_THAT((*r.jb)[0].getQ2(), WithinRel(-q.M2(), 1e-3));
  CHECK_THAT((*r.jb)[0].getY(), WithinRel(q.Dot(P_x) / k.Dot(P_x), 1e-3));
}

TEST_CASE("Truth scattered electron: no lepton line", "[ScatteredElectronsTruth]") {
  // Particle gun: no beam particles
  Event event;
  event.add(11, 1, electron(0.2, 0.1, 1), -1);

  const auto r = run(event);
  CHECK(r.electrons->empty());
  CHECK(r.hfs->empty());
}
