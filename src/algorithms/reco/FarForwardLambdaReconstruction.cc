// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul
// Update/modification 2026 by Baptiste Fraisse

#include <Evaluator/DD4hepUnits.h>
#include <TVector3.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <stdexcept>
#include <vector>
#include <type_traits>

#include "FarForwardLambdaReconstruction.h"
#include "TLorentzVector.h"

namespace eicrecon {

// helpers 

static inline void toTVector3(TVector3& v1, const edm4hep::Vector3f& v2) {
  v1.SetXYZ(v2.x, v2.y, v2.z);
}

void FarForwardLambdaReconstruction::init() {
  try {
    m_zMax = m_detector->constant<double>(m_cfg.offsetPositionName);
  } catch (std::runtime_error&) {
    m_zMax = 35800;
    trace("Failed to get {} from the detector, using default value of {}", m_cfg.offsetPositionName, m_zMax);
  }
}

// reconstruction machinery from n+g+g

bool FarForwardLambdaReconstruction::reconstruct_from_triplet(
    const edm4eic::ReconstructedParticle& n_in,
    const edm4eic::ReconstructedParticle& g1_in,
    const edm4eic::ReconstructedParticle& g2_in,
    edm4eic::ReconstructedParticleCollection* out_lambdas,
    edm4eic::ReconstructedParticleCollection* out_decay_products) const
{
  static const double m_neutron = m_particleSvc.particle(2112).mass;
  static const double m_pi0     = m_particleSvc.particle(111).mass;
  static const double m_lambda  = m_particleSvc.particle(3122).mass;

  const double En = n_in.getEnergy();
  const double E1 = g1_in.getEnergy();
  const double E2 = g2_in.getEnergy();

  if (En <= m_neutron) return false;
  const double pn = std::sqrt(std::max(0.0, En*En - m_neutron*m_neutron));

  TVector3 xn, x1, x2;

  const auto& rn  = n_in.getReferencePoint();
  const auto& rg1 = g1_in.getReferencePoint();
  const auto& rg2 = g2_in.getReferencePoint();

  xn.SetXYZ(rn.x * dd4hep::mm,
            rn.y * dd4hep::mm,
            rn.z * dd4hep::mm);

  x1.SetXYZ(rg1.x * dd4hep::mm,
            rg1.y * dd4hep::mm,
            rg1.z * dd4hep::mm);

  x2.SetXYZ(rg2.x * dd4hep::mm,
            rg2.y * dd4hep::mm,
            rg2.z * dd4hep::mm);

  xn.RotateY(-m_cfg.globalToProtonRotation);
  x1.RotateY(-m_cfg.globalToProtonRotation);
  x2.RotateY(-m_cfg.globalToProtonRotation);

  TVector3 vtx(0, 0, 0);
  double f  = 0.0;
  double df = 0.5;

  const double theta_open_expected = 2 * std::asin(m_pi0 / (2 * std::sqrt(E1 * E2)));

  TLorentzVector n, g1, g2, lambda;

  for (int i = 0; i < m_cfg.iterations; i++) {
    n      = { pn * (xn - vtx).Unit(), En };
    g1     = { E1 * (x1 - vtx).Unit(), E1 };
    g2     = { E2 * (x2 - vtx).Unit(), E2 };
    lambda = n + g1 + g2;

    const double theta_open = g1.Angle(g2.Vect());
    if (theta_open > theta_open_expected)      f -= df;
    else if (theta_open < theta_open_expected) f += df;

    vtx = lambda.Vect() * (f * m_zMax / lambda.Z());
    df /= 2.0;
  }

  const double mass_rec = lambda.M();
  if (std::abs(mass_rec - m_lambda) > m_cfg.lambdaMassWindow) return false;

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
  rec_lambda.setMomentum({static_cast<float>(lambda.X()),
                          static_cast<float>(lambda.Y()),
                          static_cast<float>(lambda.Z())});
  rec_lambda.setReferencePoint({static_cast<float>(vtx.X()),
                                static_cast<float>(vtx.Y()),
                                static_cast<float>(vtx.Z())});
  rec_lambda.setCharge(0);
  rec_lambda.setMass(mass_rec);

  // --- cm neutron
  auto neutron_cm = out_decay_products->create();
  neutron_cm.setPDG(2112);
  neutron_cm.setEnergy(n.E());
  neutron_cm.setMomentum({static_cast<float>(n.X()),
                          static_cast<float>(n.Y()),
                          static_cast<float>(n.Z())});
  neutron_cm.setReferencePoint({static_cast<float>(vtx.X()),
                                static_cast<float>(vtx.Y()),
                                static_cast<float>(vtx.Z())});
  neutron_cm.setCharge(0);
  neutron_cm.setMass(m_neutron);

  // --- cm gammas
  auto gamma1_cm = out_decay_products->create();
  gamma1_cm.setPDG(22);
  gamma1_cm.setEnergy(g1.E());
  gamma1_cm.setMomentum({static_cast<float>(g1.X()),
                         static_cast<float>(g1.Y()),
                         static_cast<float>(g1.Z())});
  gamma1_cm.setReferencePoint({static_cast<float>(vtx.X()),
                               static_cast<float>(vtx.Y()),
                               static_cast<float>(vtx.Z())});
  gamma1_cm.setCharge(0);
  gamma1_cm.setMass(0);

  auto gamma2_cm = out_decay_products->create();
  gamma2_cm.setPDG(22);
  gamma2_cm.setEnergy(g2.E());
  gamma2_cm.setMomentum({static_cast<float>(g2.X()),
                         static_cast<float>(g2.Y()),
                         static_cast<float>(g2.Z())});
  gamma2_cm.setReferencePoint({static_cast<float>(vtx.X()),
                               static_cast<float>(vtx.Y()),
                               static_cast<float>(vtx.Z())});
  gamma2_cm.setCharge(0);
  gamma2_cm.setMass(0);

  // --- links
  rec_lambda.addToParticles(n_in);
  rec_lambda.addToParticles(g1_in);
  rec_lambda.addToParticles(g2_in);

  neutron_cm.addToParticles(n_in);
  gamma1_cm.addToParticles(g1_in);
  gamma2_cm.addToParticles(g2_in);

  return true;
}

// selection of the best triplets n+g+g

void FarForwardLambdaReconstruction::process(
    const FarForwardLambdaReconstruction::Input& input,
    const FarForwardLambdaReconstruction::Output& output) const
{
  const auto [neutralsHcal, neutralsB0, neutralsEcalEndcapP, neutralsLFHCAL] = input;
  auto [out_lambdas, out_decay_products] = output;

  const double m_pi0    = m_particleSvc.particle(111).mass;
  const double m_lambda = m_particleSvc.particle(3122).mass;

  // (A) collect by detector category
  std::vector<edm4eic::ReconstructedParticle> neutrons_zdc, neutrons_other;
  std::vector<edm4eic::ReconstructedParticle> gammas_zdc, gammas_other;

  // ZDC Hcal
  for (auto part : *neutralsHcal) {
    if (part.getPDG() == 2112) neutrons_zdc.push_back(part);
    if (part.getPDG() == 22)   gammas_zdc.push_back(part);
  }

  // B0 Ecal photons
  for (auto part : *neutralsB0) {
    if (part.getPDG() == 22) gammas_other.push_back(part);
  }

  // EndcapP Ecal photons
  for (auto part : *neutralsEcalEndcapP) {
    if (part.getPDG() == 22) gammas_other.push_back(part);
  }

  // LFHCAL neutrons
  for (auto part : *neutralsLFHCAL) {
    if (part.getPDG() == 2112) neutrons_other.push_back(part);
  }

  if (neutrons_zdc.empty() && neutrons_other.empty()) return;

  // gamma pool
  using GammaT = typename std::decay_t<decltype(gammas_zdc)>::value_type;
  std::vector<GammaT> gamma_pool;
  std::vector<uint8_t> gamma_is_zdc;

  gamma_pool.reserve(gammas_zdc.size() + gammas_other.size());
  gamma_is_zdc.reserve(gammas_zdc.size() + gammas_other.size());

  for (const auto& g : gammas_zdc)   { gamma_pool.push_back(g); gamma_is_zdc.push_back(1); }
  for (const auto& g : gammas_other) { gamma_pool.push_back(g); gamma_is_zdc.push_back(0); }

  if (gamma_pool.size() < 2) return;

  // invariant mass helpers (ranking only)

  auto invMass2 = [](const edm4eic::ReconstructedParticle& a,
                     const edm4eic::ReconstructedParticle& b) {
    const auto pa = a.getMomentum();
    const auto pb = b.getMomentum();
    const double Ea = a.getEnergy();
    const double Eb = b.getEnergy();
    const double px = pa.x + pb.x;
    const double py = pa.y + pb.y;
    const double pz = pa.z + pb.z;
    const double E  = Ea + Eb;
    return E*E - (px*px + py*py + pz*pz);
  };

  auto invMass2_3 = [](const edm4eic::ReconstructedParticle& a,
                       const edm4eic::ReconstructedParticle& b,
                       const edm4eic::ReconstructedParticle& c) {
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
    return E*E - (px*px + py*py + pz*pz);
  };

  // (1) pi0 candidates
  struct Pi0Pair {
    int i, j;
    int cat;             // 0:2ZDC, 1:1ZDC, 2:0ZDC
    double dmpi0_cand;
  };

  std::vector<Pi0Pair> pi0_pairs;
  pi0_pairs.reserve(gamma_pool.size() * (gamma_pool.size() - 1) / 2);

  for (size_t i = 0; i < gamma_pool.size(); ++i) {
    for (size_t j = i + 1; j < gamma_pool.size(); ++j) {
      const auto& g1 = gamma_pool[i];
      const auto& g2 = gamma_pool[j];

      const double m2 = invMass2(g1, g2);
      if (m2 <= 0.0) continue;

      const double m  = std::sqrt(m2);
      const double dm = std::abs(m - m_pi0);
      if (dm > m_cfg.pi0Window * m_pi0) continue;

      const bool z1 = gamma_is_zdc[i];
      const bool z2 = gamma_is_zdc[j];
      const int cat = (z1 && z2) ? 0 : ((z1 || z2) ? 1 : 2);

      pi0_pairs.push_back({(int)i, (int)j, cat, dm});
    }
  }
  if (pi0_pairs.empty()) return;

  // (2) Lambda candidates pool
  struct LambdaCand {
    int g_i, g_j;
    int n_idx;
    int n_cat;   // 0: ZDC neutron, 1: other neutron
    int g_cat;   // 0:2ZDC, 1:1ZDC, 2:0ZDC
    double chi2;
    double E;
    double pz;
  };

  std::vector<LambdaCand> cands;
  cands.reserve(64);

  auto try_neutrons = [&](const auto& neutron_list, int n_cat) {
    for (const auto& pp : pi0_pairs) {
      const auto& g1 = gamma_pool[pp.i];
      const auto& g2 = gamma_pool[pp.j];

      for (size_t in = 0; in < neutron_list.size(); ++in) {
        const auto& n = neutron_list[in];

        const double m2L = invMass2_3(n, g1, g2);
        if (m2L <= 0.0) continue;

        const double mL = std::sqrt(m2L);
        if (std::abs(mL - m_lambda) > m_cfg.lambdaMassWindow * m_lambda) continue;

        const double dL = (mL - m_lambda);

        // same score as your new code
        const double chi2 =
          (pp.dmpi0_cand / (m_cfg.pi0Window * m_pi0)) * (pp.dmpi0_cand / (m_cfg.pi0Window * m_pi0)) +
          (dL / (m_cfg.lambdaMassWindow * m_lambda)) * (dL / (m_cfg.lambdaMassWindow * m_lambda));

        const double E  = n.getEnergy() + g1.getEnergy() + g2.getEnergy();

        const auto pn  = n.getMomentum();
        const auto pg1 = g1.getMomentum();
        const auto pg2 = g2.getMomentum();

        const double px = pn.x + pg1.x + pg2.x;
        const double py = pn.y + pg1.y + pg2.y;
        const double pz = pn.z + pg1.z + pg2.z;

        cands.push_back({pp.i, pp.j, (int)in, n_cat, pp.cat, chi2, E, pz});
      }
    }
  };

  if (!neutrons_zdc.empty())   try_neutrons(neutrons_zdc,   0);
  if (!neutrons_other.empty()) try_neutrons(neutrons_other, 1);

  if (cands.empty()) return;

  // (3) best candidate selection (your policy)
  auto better = [&](const LambdaCand& a, const LambdaCand& b) -> bool {
    if (a.n_cat != b.n_cat) return a.n_cat < b.n_cat;  // prefer ZDC neutron
    if (a.g_cat != b.g_cat) return a.g_cat < b.g_cat;  // prefer 2ZDC gammas
    if (a.chi2  != b.chi2)  return a.chi2  < b.chi2;
    if (a.pz    != b.pz)    return a.pz > b.pz;
    return a.E > b.E;
  };

  int best_k = 0;
  for (int k = 1; k < (int)cands.size(); ++k) {
    if (better(cands[k], cands[best_k])) best_k = k;
  }

  const auto& best = cands[best_k];

  const auto& g1 = gamma_pool[best.g_i];
  const auto& g2 = gamma_pool[best.g_j];
  const auto& n  = (best.n_cat == 0) ? neutrons_zdc[best.n_idx] : neutrons_other[best.n_idx];

  // (4) lambda reconstruction machinery
  reconstruct_from_triplet(n, g1, g2, out_lambdas, out_decay_products);
}

} // namespace eicrecon