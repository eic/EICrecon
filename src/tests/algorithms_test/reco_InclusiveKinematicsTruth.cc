// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <podio/ObjectID.h>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>

#include "algorithms/reco/InclusiveKinematicsTruthBare.h"
#include "algorithms/reco/InclusiveKinematicsTruthBorn.h"
#include "algorithms/reco/InclusiveKinematicsTruthDressed.h"
#include "algorithms/reco/InclusiveKinematicsTruthDressedConfig.h"
#include "algorithms/reco/TruthLeptonLine.h"

using Catch::Matchers::WithinRel;
using ROOT::Math::PxPyPzEVector;

namespace {

// Masses as in ParticleSvc
const double m_e = 0.000510998928;
const double m_p = 0.93827;

// Generator record built by hand
struct Record {
  edm4hep::MCParticleCollection parts;

  // Adds a proton, a charged lepton (electron mass) or a massless neutral particle; use the
  // overload with an explicit mass for anything else
  edm4hep::MutableMCParticle add(int32_t pdg, int32_t status, const PxPyPzEVector& p,
                                 float charge = 0) {
    const double mass = (std::abs(charge) > 0 ? m_e : 0.);
    return add(pdg, status, p.Px(), p.Py(), p.Pz(), (pdg == 2212 ? m_p : mass), charge);
  }

  edm4hep::MutableMCParticle add(int32_t pdg, int32_t status, double px, double py, double pz,
                                 double mass, float charge = 0) {
    auto p = parts.create();
    p.setPDG(pdg);
    p.setGeneratorStatus(status);
    p.setMomentum({px, py, pz});
    p.setMass(mass);
    p.setCharge(charge);
    return p;
  }

  static void link(edm4hep::MutableMCParticle parent, edm4hep::MutableMCParticle daughter) {
    parent.addToDaughters(daughter);
    daughter.addToParents(parent);
  }

  // Exchanged spacelike boson, stored as momentum and negative mass like DDG4 does
  edm4hep::MutableMCParticle add_boson(int32_t pdg, int32_t status, const PxPyPzEVector& q) {
    return add(pdg, status, q.Px(), q.Py(), q.Pz(), -std::sqrt(-q.M2()));
  }
};

PxPyPzEVector electron(double px, double py, double pz) {
  return {px, py, pz, std::sqrt(px * px + py * py + pz * pz + m_e * m_e)};
}
PxPyPzEVector photon(double px, double py, double pz) {
  return {px, py, pz, std::sqrt(px * px + py * py + pz * pz)};
}
PxPyPzEVector proton(double pz) { return {0, 0, pz, std::sqrt(pz * pz + m_p * m_p)}; }

// Beams: 18 GeV electrons on 275 GeV protons
const PxPyPzEVector k = electron(0, 0, -18);
const PxPyPzEVector P = proton(275);

struct Expected {
  double x;
  double Q2;
  double y;
};

Expected expected(const PxPyPzEVector& q, const PxPyPzEVector& beam = P) {
  const double Q2 = -q.M2();
  return {Q2 / (2. * q.Dot(beam)), Q2, q.Dot(beam) / k.Dot(beam)};
}

void check(const edm4eic::InclusiveKinematicsCollection& kin, const Expected& e) {
  REQUIRE(kin.size() == 1);
  CHECK_THAT(kin[0].getQ2(), WithinRel(e.Q2, 1e-5));
  CHECK_THAT(kin[0].getX(), WithinRel(e.x, 1e-5));
  CHECK_THAT(kin[0].getY(), WithinRel(e.y, 1e-5));
}

struct Results {
  std::unique_ptr<edm4eic::InclusiveKinematicsCollection> truth =
      std::make_unique<edm4eic::InclusiveKinematicsCollection>();
  std::unique_ptr<edm4eic::InclusiveKinematicsCollection> born =
      std::make_unique<edm4eic::InclusiveKinematicsCollection>();
  std::unique_ptr<edm4eic::InclusiveKinematicsCollection> dressed =
      std::make_unique<edm4eic::InclusiveKinematicsCollection>();
  std::unique_ptr<edm4eic::ReconstructedParticleCollection> scat =
      std::make_unique<edm4eic::ReconstructedParticleCollection>();
  std::unique_ptr<edm4hep::MCParticleCollection> constituents =
      std::make_unique<edm4hep::MCParticleCollection>();
};

Results run(const Record& record, const eicrecon::InclusiveKinematicsTruthDressedConfig& cfg = {}) {
  Results r;

  eicrecon::InclusiveKinematicsTruthBare truth("InclusiveKinematicsTruthBare");
  truth.level(algorithms::LogLevel::kDebug);
  truth.init();
  truth.process({&record.parts}, {r.truth.get()});

  eicrecon::InclusiveKinematicsTruthBorn born("InclusiveKinematicsTruthBorn");
  born.level(algorithms::LogLevel::kDebug);
  born.init();
  born.process({&record.parts}, {r.born.get()});

  eicrecon::InclusiveKinematicsTruthDressed dressed("InclusiveKinematicsTruthDressed");
  dressed.level(algorithms::LogLevel::kDebug);
  dressed.applyConfig(cfg);
  dressed.init();
  dressed.process({&record.parts}, {r.dressed.get(), r.scat.get(), r.constituents.get()});

  return r;
}

// DJANGOH 4.6.21 neutral-current record with an ISR photon: the beam electron emits the
// exchanged boson, the scattered electron and the photon. Returns q.
PxPyPzEVector add_djangoh_isr(Record& r, int32_t base = 0) {
  const auto ef  = electron(3, 1, 18.2);
  const auto isr = photon(0, 0, -2);
  const auto q   = k - ef - isr;
  auto ei_p      = r.add(11, base + 4, k, -1);
  auto boson_p   = r.add_boson(23, base + 21, q);
  r.add(2212, base + 4, P, 1);
  auto ef_p  = r.add(11, base + 1, ef, -1);
  auto isr_p = r.add(22, base + 1, isr);
  Record::link(ei_p, boson_p);
  Record::link(ei_p, ef_p);
  Record::link(ei_p, isr_p);
  return q;
}

} // namespace

TEST_CASE("Truth kinematics: DJANGOH 4.6.21 with ISR", "[InclusiveKinematicsTruth]") {
  Record record;
  const auto q = add_djangoh_isr(record);
  // energy transfer is negative here
  REQUIRE(q.E() < 0);
  const auto ef = electron(3, 1, 18.2);

  const auto r = run(record);
  check(*r.truth, expected(k - ef));
  check(*r.born, expected(q));
  // ISR photon is along the beam, far outside the dressing cone
  check(*r.dressed, expected(k - ef));
  REQUIRE(r.constituents->size() == 1);

  SECTION("photon not attached to the lepton line") {
    // q follows the lepton line, which then misses the photon (the exchanged boson disagrees,
    // which is only reported)
    const auto soft = photon(0, 0, -0.05);
    const auto qs   = k - ef - soft;
    REQUIRE(qs.E() < 0);
    Record unlinked;
    auto ei_p    = unlinked.add(11, 4, k, -1);
    auto boson_p = unlinked.add_boson(23, 21, qs);
    unlinked.add(2212, 4, P, 1);
    auto ef_p = unlinked.add(11, 1, ef, -1);
    unlinked.add(22, 1, soft);
    Record::link(ei_p, boson_p);
    Record::link(ei_p, ef_p);
    const auto ru = run(unlinked);
    check(*ru.truth, expected(k - ef));
    check(*ru.born, expected(k - ef));
    check(*ru.dressed, expected(k - ef));
  }
}

TEST_CASE("Truth kinematics: DJANGOH 4.6.21 with FSR", "[InclusiveKinematicsTruth]") {
  // FSR photon is a daughter of the (post-radiation) scattered electron
  const auto ef  = electron(3, 1, -10);
  const auto fsr = photon(0.6, 0.2, -2);
  const auto q   = k - ef - fsr;
  REQUIRE(q.E() > 0);

  // BeAGLE stores the exchanged boson as a photon with negative mass
  const int32_t boson_pdg = GENERATE(23, 22);

  Record record;
  auto ei_p    = record.add(11, 4, k, -1);
  auto boson_p = record.add_boson(boson_pdg, 21, q);
  record.add(2212, 4, P, 1);
  auto ef_p  = record.add(11, 1, ef, -1);
  auto fsr_p = record.add(22, 1, fsr);
  Record::link(ei_p, boson_p);
  Record::link(ei_p, ef_p);
  Record::link(ef_p, fsr_p);
  // Simulation secondary of the scattered electron, ignored
  auto delta_p = record.add(11, 0, 0.01, 0, 0, m_e, -1);
  Record::link(ef_p, delta_p);

  const auto r = run(record);
  check(*r.truth, expected(k - ef));
  check(*r.born, expected(q));
  check(*r.dressed, expected(q));
  REQUIRE(r.scat->size() == 1);
  REQUIRE((*r.dressed)[0].getScat().isAvailable());
  CHECK_THAT((*r.dressed)[0].getScat().getEnergy(), WithinRel((ef + fsr).E(), 1e-6));
  REQUIRE(r.constituents->size() == 2);
  CHECK((*r.constituents)[0].getObjectID() == ef_p.getObjectID());
  CHECK((*r.constituents)[1].getObjectID() == fsr_p.getObjectID());

  SECTION("parentage does not use the cone") {
    eicrecon::InclusiveKinematicsTruthDressedConfig cfg;
    cfg.coneDeltaR = 0;
    const auto rc  = run(record, cfg);
    check(*rc.dressed, expected(q));
  }
}

TEST_CASE("Truth kinematics: DJANGOH 4.6.10, photons on the beam electron",
          "[InclusiveKinematicsTruth]") {
  // Photon collinear with the scattered electron, but attached to the beam electron
  const auto ef  = electron(3, 1, -10);
  const auto fsr = photon(0.6, 0.2, -2);
  const auto q   = k - ef - fsr;

  Record record;
  auto ei_p    = record.add(11, 4, k, -1);
  auto boson_p = record.add_boson(23, 21, q);
  record.add(2212, 4, P, 1);
  auto ef_p  = record.add(11, 1, ef, -1);
  auto fsr_p = record.add(22, 1, fsr);
  Record::link(ei_p, boson_p);
  Record::link(ei_p, ef_p);
  Record::link(ei_p, fsr_p);

  SECTION("photon inside the cone") {
    const auto r = run(record);
    check(*r.born, expected(q));
    check(*r.dressed, expected(q));
  }
  SECTION("photon outside the cone") {
    eicrecon::InclusiveKinematicsTruthDressedConfig cfg;
    cfg.coneDeltaR = 0;
    const auto r   = run(record, cfg);
    check(*r.dressed, expected(k - ef));
  }
}

TEST_CASE("Truth kinematics: DJANGOH 4.6.10 charged current", "[InclusiveKinematicsTruth]") {
  const auto nu  = photon(-11, 23, 150); // massless neutrino
  const auto isr = photon(0, 0, -1.8);
  const auto q   = k - nu - isr;

  Record record;
  auto ei_p = record.add(11, 4, k, -1);
  auto w_p  = record.add_boson(-24, 21, q);
  record.add(2212, 4, P, 1);
  auto nu_p  = record.add(12, 1, nu);
  auto isr_p = record.add(22, 1, isr);
  Record::link(ei_p, w_p);
  Record::link(ei_p, nu_p);
  Record::link(ei_p, isr_p);
  // A final-state electron from a hadron decay, which is not the scattered lepton
  record.add(11, 1, electron(0.2, 0.1, 1), -1);

  const auto r = run(record);
  check(*r.truth, expected(k - nu));
  check(*r.born, expected(q));
  // neutrinos are not dressed
  check(*r.dressed, expected(k - nu));
  REQUIRE(r.scat->size() == 1);
  CHECK((*r.scat)[0].getPDG() == 12);
}

TEST_CASE("Truth kinematics: Pythia8 noRad", "[InclusiveKinematicsTruth]") {
  // Lepton line 4 -> 42 -> 21 -> 23 -> 44 -> 1, with QCD recoil in the 44 step
  const auto e23 = electron(3, 1, -10);
  const auto e44 = electron(3.05, 0.98, -10.02);

  Record record;
  auto p_p   = record.add(2212, 4, P, 1);
  auto e4_p  = record.add(11, 4, k, -1);
  auto e42_p = record.add(11, 42, k, -1);
  auto e21_p = record.add(11, 21, k, -1);
  auto q21_p = record.add(2, 21, 0, 0, 50, 0);
  auto e23_p = record.add(11, 23, e23, -1);
  auto q23_p = record.add(2, 23, -3, -1, 41, 0.33);
  auto e44_p = record.add(11, 44, e44, -1);
  auto e1_p  = record.add(11, 1, e44, -1);
  Record::link(p_p, q21_p);
  Record::link(e4_p, e42_p);
  Record::link(e42_p, e21_p);
  for (auto in : {e21_p, q21_p}) {
    for (auto out : {e23_p, q23_p}) {
      Record::link(in, out);
    }
  }
  Record::link(e23_p, e44_p);
  Record::link(e44_p, e1_p);

  const auto r = run(record);
  check(*r.truth, expected(k - e44));
  check(*r.born, expected(k - e23));
  check(*r.dressed, expected(k - e44));
  CHECK((*r.dressed)[0].getQ2() == (*r.truth)[0].getQ2());
  CHECK((*r.born)[0].getQ2() != (*r.truth)[0].getQ2());
}

TEST_CASE("Truth kinematics: Pythia8 without status 23 lepton", "[InclusiveKinematicsTruth]") {
  // Lepton line 4 -> 21 -> 1, the hard-scattering outgoing lepton written as final
  const auto e1 = electron(3, 1, -10);

  Record record;
  record.add(2212, 4, P, 1);
  auto e4_p  = record.add(11, 4, k, -1);
  auto e21_p = record.add(11, 21, electron(0, 0, -17.99), -1);
  auto q21_p = record.add(2, 21, 0, 0, 50, 0);
  auto e1_p  = record.add(11, 1, e1, -1);
  auto q23_p = record.add(2, 23, -3, -1, 41, 0.33);
  Record::link(e4_p, e21_p);
  for (auto in : {e21_p, q21_p}) {
    for (auto out : {e1_p, q23_p}) {
      Record::link(in, out);
    }
  }

  const auto r = run(record);
  // Born uses the hard-scattering incoming lepton for q, the others the beam lepton
  check(*r.born, expected(electron(0, 0, -17.99) - e1));
  CHECK((*r.born)[0].getQ2() != (*r.truth)[0].getQ2());
  CHECK((*r.dressed)[0].getQ2() == (*r.truth)[0].getQ2());
}

TEST_CASE("Truth kinematics: mixed frame", "[InclusiveKinematicsTruth]") {
  const auto ef = electron(3, 1, 18.2);

  // Signal status offset: unshifted, or itself shifted with background shifted further
  const int32_t signal_base = GENERATE(0, 2000);

  Record record;
  // Background interaction listed before the signal
  const int32_t bkg_base = signal_base + 1000;
  auto bkg_ei_p          = record.add(11, bkg_base + 4, electron(0, 0, -18.1), -1);
  auto bkg_ef_p          = record.add(11, bkg_base + 1, electron(0.1, 0, -17), -1);
  record.add(2212, bkg_base + 4, proton(274), 1);
  Record::link(bkg_ei_p, bkg_ef_p);
  // Unrelated final-state electron of the signal, listed before the scattered electron
  auto pi0_p = record.add(111, signal_base + 2, 0, 0, 5, 0.135);
  auto dal_p = record.add(11, signal_base + 1, electron(0, 0.1, 2), -1);
  Record::link(pi0_p, dal_p);
  // Signal
  const auto q = add_djangoh_isr(record, signal_base);

  const auto r = run(record);
  check(*r.truth, expected(k - ef));
  check(*r.born, expected(q));
  check(*r.dressed, expected(k - ef));
}

TEST_CASE("Truth kinematics: ion beams", "[InclusiveKinematicsTruth]") {
  // Ag-107, 115 GeV per nucleon
  const int32_t pdg    = 1000471070;
  const double A       = 107;
  const double m_N     = 0.9383;
  const auto P_N       = PxPyPzEVector(0, 0, 115, std::hypot(115, m_N));
  const auto ef        = electron(3, 1, -14);
  const bool total_ion = GENERATE(false, true);

  Record record;
  auto ei_p = record.add(11, 4, k, -1);
  if (total_ion) {
    // nucleus stored as a whole
    record.add(pdg, 4, 0, 0, A * 115, A * m_N, 47);
  } else {
    // nucleus stored per nucleon, as in Sartre
    record.add(pdg, 4, 0, 0, 115, m_N, 47);
  }
  auto ef_p = record.add(11, 1, ef, -1);
  Record::link(ei_p, ef_p);

  const auto r = run(record);
  check(*r.truth, expected(k - ef, P_N));
  check(*r.born, expected(k - ef, P_N));
  check(*r.dressed, expected(k - ef, P_N));
  CHECK_THAT((*r.truth)[0].getNu(), WithinRel((k - ef).Dot(P_N) / m_N, 1e-5));
}

TEST_CASE("Truth kinematics: no beam or no scattered lepton", "[InclusiveKinematicsTruth]") {
  SECTION("particle gun") {
    Record record;
    record.add(11, 1, electron(0, 1, 1), -1);
    const auto r = run(record);
    CHECK(r.truth->empty());
    CHECK(r.born->empty());
    CHECK(r.dressed->empty());
  }
  SECTION("lepton line not ending in the final state") {
    Record record;
    auto ei_p = record.add(11, 4, k, -1);
    record.add(2212, 4, P, 1);
    auto ef_p = record.add(11, 2, electron(3, 1, -14), -1);
    Record::link(ei_p, ef_p);
    const auto r = run(record);
    CHECK(r.truth->empty());
    CHECK(r.born->empty());
    CHECK(r.dressed->empty());
  }
}

TEST_CASE("Truth lepton line: status offsets", "[InclusiveKinematicsTruth]") {
  CHECK(eicrecon::status_base(4) == 0);
  CHECK(eicrecon::status_base(1001) == 1000);
  CHECK(eicrecon::status_base(2021) == 2000);
  CHECK(eicrecon::is_ion(1000471070));
  CHECK_FALSE(eicrecon::is_ion(2212));
  CHECK(eicrecon::ion_mass_number(1000471070) == 107);
  CHECK(eicrecon::ion_mass_number(1000020030) == 3);
  CHECK(eicrecon::is_ion(-1000020030));
  CHECK(eicrecon::ion_mass_number(-1000020030) == 3);
}

TEST_CASE("Truth kinematics: negative dressing cone is rejected", "[InclusiveKinematicsTruth]") {
  eicrecon::InclusiveKinematicsTruthDressed dressed("InclusiveKinematicsTruthDressed");
  eicrecon::InclusiveKinematicsTruthDressedConfig cfg;
  cfg.coneDeltaR = -0.1;
  dressed.applyConfig(cfg);
  CHECK_THROWS(dressed.init());
}
