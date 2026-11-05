// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <edm4eic/unit_system.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/utils/vector_utils.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "TruthLeptonLine.h"

namespace eicrecon {

namespace {

  // Guard against malformed (cyclic) generator records
  constexpr std::size_t max_line_length = 100;

  // Nuclei stored with a mass above this are stored as a whole, not per nucleon
  // (the lightest nucleus with A > 1, the deuteron, has a mass of 1.876 GeV)
  constexpr double max_mass_per_nucleon = 1.5 * edm4eic::unit::GeV;

} // namespace

std::optional<edm4hep::MCParticle>
find_signal_beam_lepton(const edm4hep::MCParticleCollection& mcparts) {
  std::optional<edm4hep::MCParticle> beam;
  for (const auto& p : mcparts) {
    const auto status = p.getGeneratorStatus();
    if (status <= 0 || status % 1000 != 4 || std::abs(p.getPDG()) != 11) {
      continue;
    }
    if (!beam.has_value() || status_base(status) < status_base(beam->getGeneratorStatus())) {
      beam = p;
    }
  }
  return beam;
}

std::optional<edm4hep::MCParticle> find_beam_hadron(const edm4hep::MCParticleCollection& mcparts,
                                                    int32_t base) {
  for (const auto& p : mcparts) {
    const auto pdg = p.getPDG();
    if (p.getGeneratorStatus() == base + 4 && (pdg == 2212 || pdg == 2112 || is_ion(pdg))) {
      return p;
    }
  }
  return std::nullopt;
}

std::optional<LeptonLineTruth>
find_lepton_line_truth(const edm4hep::MCParticleCollection& mcparts) {
  const auto beam = find_signal_beam_lepton(mcparts);
  if (!beam.has_value()) {
    return std::nullopt;
  }

  LeptonLineTruth truth;
  const int32_t base = status_base(beam->getGeneratorStatus());
  truth.status_base  = base;

  // Generator particles of this interaction (simulation secondaries have status 0)
  const auto in_interaction = [base](const edm4hep::MCParticle& p) {
    return p.getGeneratorStatus() != 0 && status_base(p.getGeneratorStatus()) == base;
  };

  // Descend along the lepton line: the charged lepton or, for charged-current events, its neutrino
  const int32_t pdg_lepton   = beam->getPDG();
  const int32_t pdg_neutrino = pdg_lepton + (pdg_lepton > 0 ? 1 : -1);
  truth.line.push_back(*beam);
  while (truth.line.size() < max_line_length) {
    std::optional<edm4hep::MCParticle> next;
    for (const auto& d : truth.line.back().getDaughters()) {
      if (!in_interaction(d) || (d.getPDG() != pdg_lepton && d.getPDG() != pdg_neutrino)) {
        continue;
      }
      if (!next.has_value() || d.getEnergy() > next->getEnergy()) {
        next = d;
      }
    }
    if (!next.has_value()) {
      break;
    }
    truth.line.push_back(*next);
  }
  if (truth.line.size() < 2 || truth.line.back().getGeneratorStatus() != base + 1) {
    return std::nullopt;
  }

  truth.beam_hadron = find_beam_hadron(mcparts, base);

  // Collect the exchanged boson, hard-scattering leptons, and photons radiated from the line
  std::optional<std::size_t> boson_parent;
  for (std::size_t i = 0; i < truth.line.size(); ++i) {
    const auto status = truth.line[i].getGeneratorStatus() - base;
    if (status == 21 && !truth.hard_in.has_value()) {
      truth.hard_in = i;
    }
    if (status == 23 && truth.hard_in.has_value() && !truth.hard_out.has_value()) {
      truth.hard_out = i;
    }
    for (const auto& d : truth.line[i].getDaughters()) {
      if (!in_interaction(d)) {
        continue;
      }
      const auto pdg = std::abs(d.getPDG());
      // Exchanged boson: spacelike, stored with negative mass, not in the final state
      if ((pdg == 22 || pdg == 23 || pdg == 24) && d.getMass() < 0 &&
          d.getGeneratorStatus() != base + 1) {
        if (!truth.boson.has_value()) {
          truth.boson  = d;
          boson_parent = i;
        }
        continue;
      }
      if (pdg == 22 &&
          std::none_of(truth.photons.begin(), truth.photons.end(), [&d](const auto& photon) {
            return photon.particle.getObjectID() == d.getObjectID();
          })) {
        truth.photons.push_back({d, i});
      }
    }
  }

  // Without a separate hard-scattering outgoing lepton (status 23), the lepton following the
  // hard-scattering incoming lepton leaves the hard scattering (Pythia8 may write it as final)
  if (truth.hard_in.has_value() && !truth.hard_out.has_value() &&
      *truth.hard_in + 1 < truth.line.size()) {
    truth.hard_out = *truth.hard_in + 1;
  }

  // The lepton emitting the exchanged boson, or the hard-scattering outgoing lepton, separates
  // the incoming from the outgoing side of the lepton line
  if (boson_parent.has_value()) {
    truth.first_outgoing = *boson_parent + 1;
  } else if (truth.hard_out.has_value()) {
    truth.first_outgoing = truth.hard_out;
  }

  return truth;
}

ROOT::Math::PxPyPzEVector four_momentum(const edm4hep::MCParticle& particle, double mass) {
  const auto& p = particle.getMomentum();
  return {p.x, p.y, p.z, std::hypot(edm4hep::utils::magnitude(p), mass)};
}

ROOT::Math::PxPyPzEVector four_momentum(const edm4hep::MCParticle& particle) {
  const auto& p = particle.getMomentum();
  return {p.x, p.y, p.z, particle.getEnergy()};
}

ROOT::Math::PxPyPzEVector boson_four_momentum(const edm4hep::MCParticle& boson,
                                              double energy_estimate) {
  const auto& p     = boson.getMomentum();
  const double m    = boson.getMass();
  const double m2   = (m < 0 ? -m * m : m * m);
  const double e    = std::sqrt(std::max(0.0, p.x * p.x + p.y * p.y + p.z * p.z + m2));
  const double sign = (energy_estimate < 0 ? -1.0 : 1.0);
  return {p.x, p.y, p.z, sign * e};
}

BeamPerNucleon beam_per_nucleon(const edm4hep::MCParticle& hadron, double nucleon_mass) {
  if (!is_ion(hadron.getPDG())) {
    return {four_momentum(hadron, nucleon_mass), nucleon_mass};
  }
  const auto& p = hadron.getMomentum();
  double mass   = hadron.getMass();
  double scale  = 1.0;
  if (mass > max_mass_per_nucleon) {
    scale = 1.0 / ion_mass_number(hadron.getPDG());
  }
  const double px = scale * p.x;
  const double py = scale * p.y;
  const double pz = scale * p.z;
  mass *= scale;
  return {{px, py, pz, std::sqrt(px * px + py * py + pz * pz + mass * mass)}, mass};
}

} // namespace eicrecon
