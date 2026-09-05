// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Dmitry Romanov

#include <TVector3.h>
#include <edm4eic/Vertex.h>
#include <edm4hep/Vector3f.h>
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <limits>
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
  if (denom < std::numeric_limits<float>::epsilon()) {
    const TVector3 perp = dr - dr.Dot(d1) * d1;
    return {perp.Mag(), 0.5 * (r1 + r2)};
  }
  const double t1       = dr.Cross(d2).Dot(cross) / denom;
  const double t2       = dr.Cross(d1).Dot(cross) / denom;
  const TVector3 point1 = r1 + t1 * d1;
  const TVector3 point2 = r2 + t2 * d2;
  return {(point2 - point1).Mag(), 0.5 * (point1 + point2)};
}

} // namespace

namespace eicrecon {

void ChargedLambdaReconstruction::init() {
  const double mass_min = std::max(0.0, m_lambda_mass - m_cfg.massWindow);
  const double mass_max = m_lambda_mass + m_cfg.massWindow;
  m_mass2_min           = mass_min * mass_min;
  m_mass2_max           = mass_max * mass_max;
}

void ChargedLambdaReconstruction::process(const ChargedLambdaReconstruction::Input& input,
                                          const ChargedLambdaReconstruction::Output& output) const {
  const auto [charged_particles, roman_pots, off_momentum] = input;
  auto [out_lambdas]                                       = output;

  using ParticleT = edm4eic::ReconstructedParticle;

  // --------------------------------------------------------------------------
  // Collect the daughter pools. Roman-Pot and off-momentum candidates are
  // transfer-matrix reconstructions under the proton hypothesis, so all of them
  // enter the proton pool; tracking candidates are routed by charge.
  // --------------------------------------------------------------------------

  std::vector<ParticleT> protons;
  std::vector<ParticleT> pions;

  for (const auto& particle : *charged_particles) {
    if (particle.getCharge() > 0) {
      protons.push_back(particle);
    } else if (particle.getCharge() < 0) {
      pions.push_back(particle);
    }
  }
  for (const edm4eic::ReconstructedParticleCollection* far_forward_coll :
       {roman_pots.get(), off_momentum.get()}) {
    for (const auto& particle : *far_forward_coll) {
      protons.push_back(particle);
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
    const auto proton_mom = proton.getMomentum();
    const auto proton_ref = proton.getReferencePoint();
    const TVector3 p_mom(proton_mom.x, proton_mom.y, proton_mom.z);
    const TVector3 p_ref(proton_ref.x, proton_ref.y, proton_ref.z);
    const double p_mag = p_mom.Mag();
    if (p_mag < std::numeric_limits<float>::epsilon()) {
      continue;
    }

    for (const auto& pion : pions) {
      const auto pion_mom = pion.getMomentum();
      const auto pion_ref = pion.getReferencePoint();
      const TVector3 pi_mom(pion_mom.x, pion_mom.y, pion_mom.z);
      const TVector3 pi_ref(pion_ref.x, pion_ref.y, pion_ref.z);
      const double pi_mag = pi_mom.Mag();
      if (pi_mag < std::numeric_limits<float>::epsilon()) {
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

      const auto approach = closestApproach(p_ref, p_mom.Unit(), pi_ref, pi_mom.Unit()); // [mm], EDM units
      if (approach.distance > m_cfg.dcaMax) {
        continue;
      }
      if (approach.midpoint.Z() < m_cfg.vertexZMin || approach.midpoint.Z() > m_cfg.vertexZMax) {
        continue;
      }

      if (approach.midpoint.Mag() > std::numeric_limits<float>::epsilon() &&
          pair_mom.Angle(approach.midpoint) > m_cfg.pointingMax) {
        continue;
      }

      const double mom_asym = (p_mag - pi_mag) / (p_mag + pi_mag);
      if (mom_asym < m_cfg.momAsymMin) {
        continue;
      }

      // invariant mass under the (m_p, m_pi) hypothesis; input energies are not used
      // because tracking-based candidates carry no mass assignment
      const double energy = std::hypot(p_mag, m_proton_mass) + std::hypot(pi_mag, m_pion_mass);
      const double mass2  = energy * energy - pair_mom.Mag2();
      if (mass2 < m_mass2_min || mass2 > m_mass2_max) {
        continue;
      }
      const double mass = std::sqrt(mass2);

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

      trace("Lambda candidate: m = {:.4f} GeV, p = {:.2f} GeV, decay-z proxy = {:.0f} mm, dca = {:.1f} mm, opening = {:.1f} mrad",
            mass, pair_mom.Mag(), approach.midpoint.Z(), approach.distance, opening * 1e3);
    }
  }

  debug("built {} Lambda candidates from {} proton and {} pion candidates", out_lambdas->size(),
        protons.size(), pions.size());
}

} // namespace eicrecon
