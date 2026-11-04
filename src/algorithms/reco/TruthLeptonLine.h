// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <Math/Vector4D.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/MCParticleCollection.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace eicrecon {

/// Generator status offset of the interaction a particle belongs to. When several interactions
/// are merged into one frame, background interactions have their generator status shifted by
/// multiples of 1000.
inline int32_t status_base(int32_t status) { return status - status % 1000; }

/// Whether a PDG code denotes a nucleus (10LZZZAAAI)
inline bool is_ion(int32_t pdg) { return pdg >= 1000000000; }

/// Mass number A of a nucleus PDG code (10LZZZAAAI)
inline int32_t ion_mass_number(int32_t pdg) { return (pdg / 10) % 1000; }

/// A real photon radiated directly from the lepton line
struct LeptonLinePhoton {
  edm4hep::MCParticle particle;
  /// Index in LeptonLineTruth::line of the lepton the photon was emitted from
  std::size_t parent_index;
};

/// Lepton side of a single DIS interaction, as recorded by the event generator in MCParticles.
///
/// The lepton line is found by descending from the beam lepton through generator daughters with
/// the same lepton flavor (charged lepton or its neutrino, for charged-current events), so the
/// scattered lepton is the final-state end of the line rather than the first final-state lepton
/// in the record. Supported topologies:
///  - DJANGOH, BeAGLE: beam lepton -> exchanged spacelike boson + scattered lepton (+ photons),
///    with FSR photons as daughters of the scattered lepton (DJANGOH 4.6.21) or all photons as
///    daughters of the beam lepton (DJANGOH 4.6.10);
///  - Pythia8: beam lepton (4) -> 42 -> 21 (hard incoming) -> 23 (hard outgoing) -> 44 -> 1.
struct LeptonLineTruth {
  /// Lepton line, beam lepton first, final-state scattered lepton last
  std::vector<edm4hep::MCParticle> line;
  /// Beam hadron (nucleon or nucleus) of the same interaction, if found
  std::optional<edm4hep::MCParticle> beam_hadron;
  /// Exchanged spacelike boson (gamma*, Z, W) emitted from the lepton line, if stored
  std::optional<edm4hep::MCParticle> boson;
  /// Indices in `line` of the hard-scattering incoming (status 21) and outgoing (status 23)
  /// leptons, if stored (Pythia8)
  std::optional<std::size_t> hard_in;
  std::optional<std::size_t> hard_out;
  /// Index in `line` of the first lepton after the hard scattering, if the record identifies it
  std::optional<std::size_t> first_outgoing;
  /// Real photons radiated directly from the lepton line
  std::vector<LeptonLinePhoton> photons;
  /// Generator status offset of this interaction
  int32_t status_base{0};

  const edm4hep::MCParticle& beam_lepton() const { return line.front(); }
  const edm4hep::MCParticle& scattered_lepton() const { return line.back(); }

  /// Whether a photon was emitted after the hard scattering, according to the record
  bool is_fsr(const LeptonLinePhoton& photon) const {
    return first_outgoing.has_value() && photon.parent_index >= *first_outgoing;
  }
};

/// Find the beam lepton (status 4 electron or positron) of the signal interaction: the one with
/// the lowest generator status offset, and the first in the record among those.
std::optional<edm4hep::MCParticle>
find_signal_beam_lepton(const edm4hep::MCParticleCollection& mcparts);

/// Find the beam hadron (status 4 proton, neutron or nucleus) with the given generator status
/// offset, first in the record.
std::optional<edm4hep::MCParticle> find_beam_hadron(const edm4hep::MCParticleCollection& mcparts,
                                                    int32_t status_base);

/// Find the lepton line of the signal interaction. Returns std::nullopt when there is no beam
/// lepton, or the lepton line does not end in a final-state lepton.
std::optional<LeptonLineTruth> find_lepton_line_truth(const edm4hep::MCParticleCollection& mcparts);

/// Four-momentum of a generator particle, using the given mass
ROOT::Math::PxPyPzEVector four_momentum(const edm4hep::MCParticle& particle, double mass);

/// Four-momentum of a generator particle, using its stored mass
ROOT::Math::PxPyPzEVector four_momentum(const edm4hep::MCParticle& particle);

/// Four-momentum of the exchanged spacelike boson. EDM4hep stores only its momentum and its
/// (negative) mass, so the sign of the energy is lost; it is taken from `energy_estimate`, the
/// boson energy expected from momentum conservation on the lepton line.
ROOT::Math::PxPyPzEVector boson_four_momentum(const edm4hep::MCParticle& boson,
                                              double energy_estimate);

/// Beam hadron four-momentum and mass per nucleon
struct BeamPerNucleon {
  ROOT::Math::PxPyPzEVector momentum;
  double mass;
};

/// Four-momentum and mass per nucleon of a beam hadron. Nucleons use `nucleon_mass` (ignored for
/// nuclei). Nuclei use their stored mass, and are divided by the mass number A unless already
/// stored per nucleon (stored mass below that of the deuteron, as written by e.g. Sartre).
BeamPerNucleon beam_per_nucleon(const edm4hep::MCParticle& hadron, double nucleon_mass);

/// Inclusive DIS variables
struct DISVariables {
  double x;
  double Q2;
  double W;
  double y;
  double nu;
};

/// Inclusive DIS variables from the beam lepton k, the beam hadron P with mass M (per nucleon),
/// and the four-momentum transfer q
inline DISVariables dis_variables(const ROOT::Math::PxPyPzEVector& k,
                                  const ROOT::Math::PxPyPzEVector& P, double M,
                                  const ROOT::Math::PxPyPzEVector& q) {
  const auto q_dot_P = q.Dot(P);
  const auto Q2      = -q.Dot(q);
  const auto y       = q_dot_P / k.Dot(P);
  const auto nu      = q_dot_P / M;
  const auto x       = Q2 / (2. * q_dot_P);
  const auto W       = std::sqrt(M * M + 2. * q_dot_P - Q2);
  return {x, Q2, W, y, nu};
}

} // namespace eicrecon
