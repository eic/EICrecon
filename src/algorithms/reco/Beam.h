// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <Math/Vector4D.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <algorithm>
#include <fmt/format.h>
#include <set>
#include <stdexcept>
#include <vector>
#include <cmath>

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

template <class T> auto find_first_with_pdg(const T* parts, const std::set<int32_t>& pdg) {
  T c;
  c.setSubsetCollection();
  const auto it = std::find_if(parts->begin(), parts->end(),
                               [&pdg](const auto& p) { return pdg.count(p.getPDG()) > 0; });
  if (it != parts->end()) {
    c.push_back(*it);
  }
  return c;
}

template <class T>
auto find_first_with_status_pdg(const T* parts, const std::set<int32_t>& status,
                                const std::set<int32_t>& pdg) {
  T c;
  c.setSubsetCollection();
  const auto it = std::find_if(parts->begin(), parts->end(), [&status, &pdg](const auto& p) {
    return status.count(p.getGeneratorStatus()) > 0 && pdg.count(p.getPDG()) > 0;
  });
  if (it != parts->end()) {
    c.push_back(*it);
  }
  return c;
}

inline auto find_first_beam_electron(const edm4hep::MCParticleCollection* mcparts) {
  return find_first_with_status_pdg(mcparts, {4}, {11});
}

inline auto find_first_beam_hadron(const edm4hep::MCParticleCollection* mcparts) {
  return find_first_with_status_pdg(mcparts, {4}, {2212, 2112});
}

inline auto find_first_scattered_electron(const edm4hep::MCParticleCollection* mcparts) {
  return find_first_with_status_pdg(mcparts, {1}, {11});
}

inline auto find_first_scattered_electron(const edm4eic::ReconstructedParticleCollection* rcparts) {
  return find_first_with_pdg(rcparts, {11});
}

// Canonical beam momentum allowlists used by all kinematics algorithms.
// Electron beam: negative pz (beam goes in -z direction).
inline const std::vector<float> electron_beam_pz_set{-5.0, -10.0, -18.0};
// Hadron beam: positive pz (beam goes in +z direction).
inline const std::vector<float> hadron_beam_pz_set{41.0, 100.0, 130.0, 250.0, 275.0};

template <typename Vector3>
PxPyPzEVector round_beam_four_momentum(const Vector3& p_in, const float mass,
                                       const std::vector<float>& pz_set,
                                       const float crossing_angle = 0.0) {
  // Find the closest pz within 10% relative tolerance
  float best_pz    = 0.0F;
  float best_err   = 0.1F; // 10% tolerance — entries above this are not accepted
  bool found_match = false;
  for (const auto& pz : pz_set) {
    const float err = std::abs(p_in.z / pz - 1);
    if (err < best_err) {
      best_err    = err;
      best_pz     = pz;
      found_match = true;
    }
  }
  if (!found_match) {
    throw std::runtime_error(
        fmt::format("round_beam_four_momentum: no match for beam momentum {:.3f} GeV within 10%% "
                    "of any of the allowed values",
                    p_in.z));
  }
  PxPyPzEVector p_out;
  p_out.SetPz(best_pz);
  p_out.SetPx(p_out.Pz() * sin(crossing_angle));
  p_out.SetPz(p_out.Pz() * cos(crossing_angle));
  p_out.SetE(std::hypot(p_out.Px(), p_out.Pz(), mass));
  return p_out;
}

} // namespace eicrecon
