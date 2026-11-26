// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Sort keys for SortSubsetCollection_factory<edm4eic::ReconstructedParticle, key>. They are
// inline functions in a header (not local to a plugin), so that the factory type is the same
// wherever it is named (e.g. in the Python bindings, src/python).

#pragma once

#include <edm4eic/ReconstructedParticle.h>

#include <cmath>

namespace eicrecon {

/// Transverse momentum of a reconstructed particle
inline double reco_particle_pt(const edm4eic::ReconstructedParticle& particle) {
  return std::hypot(particle.getMomentum().x, particle.getMomentum().y);
}

} // namespace eicrecon
