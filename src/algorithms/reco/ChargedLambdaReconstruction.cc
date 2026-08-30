// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Dmitry Romanov

#include <TVector3.h>
#include <edm4eic/Vertex.h>
#include <edm4hep/Vector3f.h>
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <tuple>
#include <vector>

#include "ChargedLambdaReconstruction.h"

namespace {

/** Closest approach of two straight lines r + t*d (d normalized).
 *
 * Returns the distance and the midpoint between the two closest points. For
 * near-parallel lines (a degenerate pairing, not a physical V0) the closest points
 * are ill-defined; fall back to the perpendicular distance between the lines and the
 * midpoint of the reference points. */
struct LineApproach {
  double distance;
  TVector3 midpoint;
};

LineApproach closestApproach(const TVector3& r1, const TVector3& d1, const TVector3& r2,
                             const TVector3& d2) {
  const TVector3 cross = d1.Cross(d2);
  const double denom   = cross.Mag2();
  const TVector3 dr    = r2 - r1;
  if (denom < 1e-12) {
    const TVector3 perp = dr - dr.Dot(d1) * d1;
    return {perp.Mag(), 0.5 * (r1 + r2)};
  }
  const double t1   = dr.Cross(d2).Dot(cross) / denom;
  const double t2   = dr.Cross(d1).Dot(cross) / denom;
  const TVector3 p1 = r1 + t1 * d1;
  const TVector3 p2 = r2 + t2 * d2;
  return {(p2 - p1).Mag(), 0.5 * (p1 + p2)};
}

} // namespace

namespace eicrecon {

void ChargedLambdaReconstruction::init() {
  m_proton_mass = m_particleSvc.particle(2212).mass;
  m_pion_mass   = m_particleSvc.particle(211).mass;
  m_lambda_mass = m_particleSvc.particle(3122).mass;
}

void ChargedLambdaReconstruction::process(const ChargedLambdaReconstruction::Input& input,
                                          const ChargedLambdaReconstruction::Output& output) const {
  const auto [charged, roman_pots, off_momentum] = input;
  auto [out_lambdas]                             = output;

  using ParticleT = edm4eic::ReconstructedParticle;

  // --------------------------------------------------------------------------
  // Collect the daughter pools. Roman-Pot and off-momentum candidates are
  // transfer-matrix reconstructions under the proton hypothesis, so all of them
  // enter the proton pool; tracking candidates are routed by charge.
  // --------------------------------------------------------------------------

  std::vector<ParticleT> protons;
  std::vector<ParticleT> pions;

  for (const auto& part : *charged) {
    if (part.getCharge() > 0) {
      protons.push_back(part);
    } else if (part.getCharge() < 0) {
      pions.push_back(part);
    }
  }
  for (const edm4eic::ReconstructedParticleCollection* coll :
       {roman_pots.get(), off_momentum.get()}) {
    for (const auto& part : *coll) {
      protons.push_back(part);
    }
  }

  if (protons.empty() || pions.empty()) {
    debug("no pairs to build: {} proton candidates, {} pion candidates", protons.size(),
          pions.size());
    return;
  }

  // --------------------------------------------------------------------------
  // Evaluate every (p, pi-) pairing with the V0 cut chain.
  // --------------------------------------------------------------------------

  for (const auto& proton : protons) {
    const auto pp = proton.getMomentum();
    const auto rp = proton.getReferencePoint();
    const TVector3 p_mom(pp.x, pp.y, pp.z);
    const TVector3 p_ref(rp.x, rp.y, rp.z);
    const double p_mag = p_mom.Mag();
    if (p_mag < 1e-9) {
      continue;
    }

    for (const auto& pion : pions) {
      const auto pip = pion.getMomentum();
      const auto rip = pion.getReferencePoint();
      const TVector3 pi_mom(pip.x, pip.y, pip.z);
      const TVector3 pi_ref(rip.x, rip.y, rip.z);
      const double pi_mag = pi_mom.Mag();
      if (pi_mag < 1e-9) {
        continue;
      }

      const double opening = p_mom.Angle(pi_mom);
      if (opening > m_cfg.openingAngleMax) {
        continue;
      }

      const TVector3 pair_mom = p_mom + pi_mom;
      if (pair_mom.Theta() > m_cfg.pairThetaMax) {
        continue;
      }

      const auto approach =
          closestApproach(p_ref, p_mom.Unit(), pi_ref, pi_mom.Unit()); // [mm], EDM units
      if (approach.distance > m_cfg.dcaMax) {
        continue;
      }
      if (approach.midpoint.Z() < m_cfg.vertexZMin || approach.midpoint.Z() > m_cfg.vertexZMax) {
        continue;
      }

      if (approach.midpoint.Mag() > 1e-9 && pair_mom.Angle(approach.midpoint) > m_cfg.pointingMax) {
        continue;
      }

      const double mom_asym = (p_mag - pi_mag) / (p_mag + pi_mag);
      if (mom_asym < m_cfg.momAsymMin) {
        continue;
      }

      // invariant mass under the (m_p, m_pi) hypothesis; input energies are not used
      // because tracking-based candidates carry no mass assignment
      const double energy = std::sqrt(p_mag * p_mag + m_proton_mass * m_proton_mass) +
                            std::sqrt(pi_mag * pi_mag + m_pion_mass * m_pion_mass);
      const double mass2  = energy * energy - pair_mom.Mag2();
      const double mass   = std::sqrt(std::max(0.0, mass2));
      if (std::abs(mass - m_lambda_mass) > m_cfg.massWindow) {
        continue;
      }

      auto lambda = out_lambdas->create();
      lambda.setPDG(3122);
      lambda.setCharge(0);
      lambda.setMass(mass);
      lambda.setEnergy(energy);
      lambda.setMomentum({static_cast<float>(pair_mom.X()), static_cast<float>(pair_mom.Y()),
                          static_cast<float>(pair_mom.Z())});
      lambda.setReferencePoint({static_cast<float>(approach.midpoint.X()),
                                static_cast<float>(approach.midpoint.Y()),
                                static_cast<float>(approach.midpoint.Z())});
      lambda.addToParticles(proton);
      lambda.addToParticles(pion);

      trace("Lambda candidate: m = {:.4f} GeV, p = {:.2f} GeV, decay-z proxy = {:.0f} mm, "
            "dca = {:.1f} mm, opening = {:.1f} mrad",
            mass, pair_mom.Mag(), approach.midpoint.Z(), approach.distance, opening * 1e3);
    }
  }

  debug("built {} Lambda candidates from {} proton and {} pion candidates", out_lambdas->size(),
        protons.size(), pions.size());
}

} // namespace eicrecon
