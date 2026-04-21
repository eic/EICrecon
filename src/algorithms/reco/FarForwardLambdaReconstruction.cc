// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul, Baptiste Fraisse

#include <Evaluator/DD4hepUnits.h>
#include <TVector3.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/Vertex.h>
#include <edm4hep/Vector3f.h>
#include <stdint.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "FarForwardLambdaReconstruction.h"
#include "TLorentzVector.h"

namespace eicrecon {

void FarForwardLambdaReconstruction::init() {
  try {
    m_zMax = m_detector->constant<double>(m_cfg.offsetPositionName);
  } catch (std::runtime_error&) {
    m_zMax = 35800;
    trace("Failed to get {} from the detector, using default value of {}", m_cfg.offsetPositionName,
          m_zMax);
  }
}

// reconstruction machinery from n+g+g

bool FarForwardLambdaReconstruction::reconstruct_from_triplet(
    const edm4eic::ReconstructedParticle& n_in, const edm4eic::ReconstructedParticle& g1_in,
    const edm4eic::ReconstructedParticle& g2_in,
    edm4eic::ReconstructedParticleCollection* out_lambdas,
    edm4eic::ReconstructedParticleCollection* out_decay_products) const {
  static const double m_neutron = m_particleSvc.particle(2112).mass;
  static const double m_pi0     = m_particleSvc.particle(111).mass;
  static const double m_lambda  = m_particleSvc.particle(3122).mass;

  const double En = n_in.getEnergy();
  const double E1 = g1_in.getEnergy();
  const double E2 = g2_in.getEnergy();

  if (En <= m_neutron)
    return false;
  const double pn = std::sqrt(std::max(0.0, En * En - m_neutron * m_neutron));

  TVector3 xn, x1, x2;

  const auto& rn  = n_in.getReferencePoint();
  const auto& rg1 = g1_in.getReferencePoint();
  const auto& rg2 = g2_in.getReferencePoint();

  xn.SetXYZ(rn.x * dd4hep::mm, rn.y * dd4hep::mm, rn.z * dd4hep::mm);
  x1.SetXYZ(rg1.x * dd4hep::mm, rg1.y * dd4hep::mm, rg1.z * dd4hep::mm);
  x2.SetXYZ(rg2.x * dd4hep::mm, rg2.y * dd4hep::mm, rg2.z * dd4hep::mm);

  xn.RotateY(-m_cfg.globalToProtonRotation);
  x1.RotateY(-m_cfg.globalToProtonRotation);
  x2.RotateY(-m_cfg.globalToProtonRotation);

  TVector3 vtx(0, 0, 0);
  double f  = 0.0;
  double df = 0.5;

  const double theta_open_expected = 2 * std::asin(m_pi0 / (2 * std::sqrt(E1 * E2)));

  TLorentzVector n, g1, g2, lambda;

  for (int i = 0; i < m_cfg.iterations; i++) {
    n      = {pn * (xn - vtx).Unit(), En};
    g1     = {E1 * (x1 - vtx).Unit(), E1};
    g2     = {E2 * (x2 - vtx).Unit(), E2};
    lambda = n + g1 + g2;

    const double theta_open = g1.Angle(g2.Vect());
    if (theta_open > theta_open_expected)
      f -= df;
    else if (theta_open < theta_open_expected)
      f += df;

    vtx = lambda.Vect() * (f * m_zMax / lambda.Z());
    df /= 2.0;
  }

  const double mass_rec = lambda.M();
  if (std::abs(mass_rec - m_lambda) > m_cfg.lambdaMassWindow)
    return false;

  // rotate everything back to lab coordinates
  vtx.RotateY(m_cfg.globalToProtonRotation);
  lambda.RotateY(m_cfg.globalToProtonRotation);
  n.RotateY(m_cfg.globalToProtonRotation);
  g1.RotateY(m_cfg.globalToProtonRotation);
  g2.RotateY(m_cfg.globalToProtonRotation);

  // boost decay products to Lambda rest frame
  auto b = -lambda.BoostVector();
  n.Boost(b);
  g1.Boost(b);
  g2.Boost(b);

  // vertex back to EDM units (mm)
  vtx = vtx * (1.0 / dd4hep::mm);

  // saving reco lambda
  auto rec_lambda = out_lambdas->create();
  rec_lambda.setPDG(3122);
  rec_lambda.setEnergy(lambda.E());
  rec_lambda.setMomentum({static_cast<float>(lambda.X()), static_cast<float>(lambda.Y()),
                          static_cast<float>(lambda.Z())});
  rec_lambda.setReferencePoint(
      {static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()), static_cast<float>(vtx.Z())});
  rec_lambda.setCharge(0);
  rec_lambda.setMass(mass_rec);

  // --- cm neutron
  auto neutron_cm = out_decay_products->create();
  neutron_cm.setPDG(2112);
  neutron_cm.setEnergy(n.E());
  neutron_cm.setMomentum(
      {static_cast<float>(n.X()), static_cast<float>(n.Y()), static_cast<float>(n.Z())});
  neutron_cm.setReferencePoint(
      {static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()), static_cast<float>(vtx.Z())});
  neutron_cm.setCharge(0);
  neutron_cm.setMass(m_neutron);

  // --- cm gammas
  auto gamma1_cm = out_decay_products->create();
  gamma1_cm.setPDG(22);
  gamma1_cm.setEnergy(g1.E());
  gamma1_cm.setMomentum(
      {static_cast<float>(g1.X()), static_cast<float>(g1.Y()), static_cast<float>(g1.Z())});
  gamma1_cm.setReferencePoint(
      {static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()), static_cast<float>(vtx.Z())});
  gamma1_cm.setCharge(0);
  gamma1_cm.setMass(0);

  auto gamma2_cm = out_decay_products->create();
  gamma2_cm.setPDG(22);
  gamma2_cm.setEnergy(g2.E());
  gamma2_cm.setMomentum(
      {static_cast<float>(g2.X()), static_cast<float>(g2.Y()), static_cast<float>(g2.Z())});
  gamma2_cm.setReferencePoint(
      {static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()), static_cast<float>(vtx.Z())});
  gamma2_cm.setCharge(0);
  gamma2_cm.setMass(0);

  rec_lambda.addToParticles(n_in);
  rec_lambda.addToParticles(g1_in);
  rec_lambda.addToParticles(g2_in);

  neutron_cm.addToParticles(n_in);
  gamma1_cm.addToParticles(g1_in);
  gamma2_cm.addToParticles(g2_in);

  return true;
}

void FarForwardLambdaReconstruction::process(
    const FarForwardLambdaReconstruction::Input& input,
    const FarForwardLambdaReconstruction::Output& output) const {
  const auto [neutralsHcal, neutralsB0, neutralsEcalEndcapP, neutralsLFHCAL] = input;
  auto [out_lambdas, out_decay_products]                                     = output;

  const double m_pi0    = m_particleSvc.particle(111).mass;
  const double m_lambda = m_particleSvc.particle(3122).mass;

  // --------------------------------------------------------------------------
  // Step 1: collect neutral candidates by broad detector category
  //
  // We keep two categories for ranking purposes:
  //   - ZDC-preferred neutrals
  //   - other far-forward neutrals
  // while centralizing the detector-specific collection handling in a single
  // local table to avoid duplicating hardcoded loops throughout the algorithm.
  // --------------------------------------------------------------------------

  std::vector<edm4eic::ReconstructedParticle> neutrons_zdc, neutrons_other;
  std::vector<edm4eic::ReconstructedParticle> gammas_zdc, gammas_other;

  struct NeutralInputDesc {
    const edm4eic::ReconstructedParticleCollection* coll;
    bool use_gamma;
    bool use_neutron;
    bool is_zdc;
  };

  const std::array<NeutralInputDesc, 4> input_descs{{
      {neutralsHcal, true, true, true},
      {neutralsB0, true, false, false},
      {neutralsEcalEndcapP, true, false, false},
      {neutralsLFHCAL, false, true, false},
  }};

  for (const auto& desc : input_descs) {
    for (const auto& part : *desc.coll) {
      if (desc.use_neutron && part.getPDG() == 2112) {
        (desc.is_zdc ? neutrons_zdc : neutrons_other).push_back(part);
      }
      if (desc.use_gamma && part.getPDG() == 22) {
        (desc.is_zdc ? gammas_zdc : gammas_other).push_back(part);
      }
    }
  }

  if (neutrons_zdc.empty() && neutrons_other.empty()) {
    return;
  }

  // --------------------------------------------------------------------------
  // Step 2: build a unified photon pool while retaining whether each photon
  // comes from the ZDC-preferred category.
  // --------------------------------------------------------------------------

  using ParticleT = edm4eic::ReconstructedParticle;

  std::vector<ParticleT> gamma_pool;
  std::vector<uint8_t> gamma_is_zdc;

  gamma_pool.reserve(gammas_zdc.size() + gammas_other.size());
  gamma_is_zdc.reserve(gammas_zdc.size() + gammas_other.size());

  for (const auto& g : gammas_zdc) {
    gamma_pool.push_back(g);
    gamma_is_zdc.push_back(1);
  }
  for (const auto& g : gammas_other) {
    gamma_pool.push_back(g);
    gamma_is_zdc.push_back(0);
  }

  if (gamma_pool.size() < 2) {
    return;
  }

  // --------------------------------------------------------------------------
  // Invariant-mass helpers
  // --------------------------------------------------------------------------

  auto invMass2 = [](const ParticleT& a, const ParticleT& b) {
    const auto pa = a.getMomentum();
    const auto pb = b.getMomentum();

    const double Ea = a.getEnergy();
    const double Eb = b.getEnergy();

    const double px = pa.x + pb.x;
    const double py = pa.y + pb.y;
    const double pz = pa.z + pb.z;
    const double E  = Ea + Eb;

    return E * E - (px * px + py * py + pz * pz);
  };

  auto invMass2_3 = [](const ParticleT& a, const ParticleT& b, const ParticleT& c) {
    const auto pa = a.getMomentum();
    const auto pb = b.getMomentum();
    const auto pc = c.getMomentum();

    const double Ea = a.getEnergy();
    const double Eb = b.getEnergy();
    const double Ec = c.getEnergy();

    const double px = pa.x + pb.x + pc.x;
    const double py = pa.y + pb.y + pc.y;
    const double pz = pa.z + pb.z + pc.z;
    const double E  = Ea + Eb + Ec;

    return E * E - (px * px + py * py + pz * pz);
  };

  // --------------------------------------------------------------------------
  // Step 3: build pi0 candidates from photon pairs
  // --------------------------------------------------------------------------

  enum class GammaCategory : uint8_t { TwoZDC = 0, OneZDC = 1, ZeroZDC = 2 };

  struct Pi0Pair {
    int i;
    int j;
    GammaCategory cat;
    double dmpi0_cand;
  };

  std::vector<Pi0Pair> pi0_pairs;
  pi0_pairs.reserve(gamma_pool.size() * (gamma_pool.size() - 1) / 2);

  for (size_t i = 0; i < gamma_pool.size(); ++i) {
    for (size_t j = i + 1; j < gamma_pool.size(); ++j) {
      const auto& g1 = gamma_pool[i];
      const auto& g2 = gamma_pool[j];

      const double m2 = invMass2(g1, g2);
      if (m2 <= 0.0) {
        continue;
      }

      const double m  = std::sqrt(m2);
      const double dm = std::abs(m - m_pi0);
      if (dm > m_cfg.pi0Window * m_pi0) {
        continue;
      }

      const bool z1 = gamma_is_zdc[i];
      const bool z2 = gamma_is_zdc[j];

      const GammaCategory cat = (z1 && z2)   ? GammaCategory::TwoZDC
                                : (z1 || z2) ? GammaCategory::OneZDC
                                             : GammaCategory::ZeroZDC;

      pi0_pairs.push_back({static_cast<int>(i), static_cast<int>(j), cat, dm});
    }
  }

  if (pi0_pairs.empty()) {
    return;
  }

  // --------------------------------------------------------------------------
  // Step 4: build Lambda candidate pool from pi0-neutron combinations
  // --------------------------------------------------------------------------

  enum class NeutronCategory : uint8_t { ZDC = 0, Other = 1 };

  struct LambdaCand {
    int g_i;
    int g_j;
    int n_idx;
    NeutronCategory n_cat;
    GammaCategory g_cat;
    double chi2;
    double E;
    double pz;
  };

  std::vector<LambdaCand> cands;
  cands.reserve(64);

  auto try_neutrons = [&](const auto& neutron_list, NeutronCategory n_cat) {
    for (const auto& pp : pi0_pairs) {
      const auto& g1 = gamma_pool[pp.i];
      const auto& g2 = gamma_pool[pp.j];

      for (size_t in = 0; in < neutron_list.size(); ++in) {
        const auto& n = neutron_list[in];

        const double m2L = invMass2_3(n, g1, g2);
        if (m2L <= 0.0) {
          continue;
        }

        const double mL = std::sqrt(m2L);
        const double dL = mL - m_lambda;

        if (std::abs(dL) > m_cfg.lambdaMassWindow * m_lambda) {
          continue;
        }

        const double pi0_term    = pp.dmpi0_cand / (m_cfg.pi0Window * m_pi0);
        const double lambda_term = dL / (m_cfg.lambdaMassWindow * m_lambda);

        const double chi2 = pi0_term * pi0_term + lambda_term * lambda_term;

        const double E = n.getEnergy() + g1.getEnergy() + g2.getEnergy();

        const auto pn  = n.getMomentum();
        const auto pg1 = g1.getMomentum();
        const auto pg2 = g2.getMomentum();

        const double pz = pn.z + pg1.z + pg2.z;

        cands.push_back({pp.i, pp.j, static_cast<int>(in), n_cat, pp.cat, chi2, E, pz});
      }
    }
  };

  if (!neutrons_zdc.empty()) {
    try_neutrons(neutrons_zdc, NeutronCategory::ZDC);
  }
  if (!neutrons_other.empty()) {
    try_neutrons(neutrons_other, NeutronCategory::Other);
  }

  if (cands.empty()) {
    return;
  }

  // --------------------------------------------------------------------------
  // Step 5: rank candidates and keep the best one
  // Preference order:
  //   1. ZDC neutron over other neutron
  //   2. more ZDC photons in the pi0 candidate
  //   3. lower combined mass-based chi2
  //   4. larger forward momentum pz
  //   5. larger total energy
  // --------------------------------------------------------------------------

  auto better = [&](const LambdaCand& a, const LambdaCand& b) -> bool {
    if (a.n_cat != b.n_cat) {
      return static_cast<int>(a.n_cat) < static_cast<int>(b.n_cat);
    }
    if (a.g_cat != b.g_cat) {
      return static_cast<int>(a.g_cat) < static_cast<int>(b.g_cat);
    }
    if (a.chi2 != b.chi2) {
      return a.chi2 < b.chi2;
    }
    if (a.pz != b.pz) {
      return a.pz > b.pz;
    }
    return a.E > b.E;
  };

  int best_k = 0;
  for (int k = 1; k < static_cast<int>(cands.size()); ++k) {
    if (better(cands[k], cands[best_k])) {
      best_k = k;
    }
  }

  const auto& best = cands[best_k];

  const auto& g1 = gamma_pool[best.g_i];
  const auto& g2 = gamma_pool[best.g_j];
  const auto& n =
      (best.n_cat == NeutronCategory::ZDC) ? neutrons_zdc[best.n_idx] : neutrons_other[best.n_idx];

  reconstruct_from_triplet(n, g1, g2, out_lambdas, out_decay_products);
}

} // namespace eicrecon
