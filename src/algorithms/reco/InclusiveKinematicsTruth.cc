// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#include <algorithm>
#include <cmath>
#include <vector>

#include "Beam.h"
#include "Boost.h"
#include "InclusiveKinematicsTruth.h"

#include "Math/Vector4D.h"
using ROOT::Math::PxPyPzEVector;

// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/InclusiveKinematicsCollection.h"
#include "edm4eic/vector_utils.h"

#include "ParticlesWithAssociation.h"

namespace eicrecon {

  void InclusiveKinematicsTruth::init(std::shared_ptr<spdlog::logger> logger) {
    m_log = logger;

    // m_pidSvc = service("ParticleSvc");
    // if (!m_pidSvc) {
    //   error() << "Unable to locate Particle Service. "
    //           << "Make sure you have ParticleSvc in the configuration."
    //           << endmsg;
    //   return StatusCode::FAILURE;
    // }
  }

  std::vector<edm4eic::InclusiveKinematics*> InclusiveKinematicsTruth::execute(
    std::vector<const edm4hep::MCParticle *> mcparts) {

    // Resulting inclusive kinematics
    std::vector<edm4eic::InclusiveKinematics *> kinematics;

    // Loop over generated particles to get incoming electron and proton beams
    // and the scattered electron. In the presence of QED radition on the incoming
    // or outgoing electron line, the vertex kinematics will be different than the
    // kinematics calculated using the scattered electron as done here.
    // Also need to update for CC events.

    // Get incoming electron beam
    const auto ei_coll = find_first_beam_electron(mcparts);
    if (ei_coll.size() == 0) {
      m_log->debug("No beam electron found");
      return kinematics;
    }
    const auto ei_p = ei_coll[0]->getMomentum();
    const auto ei_p_mag = edm4eic::magnitude(ei_p);
    const auto ei_mass = m_electron;
    const PxPyPzEVector ei(ei_p.x, ei_p.y, ei_p.z, std::hypot(ei_p_mag, ei_mass));

    // Get incoming hadron beam
    const auto pi_coll = find_first_beam_hadron(mcparts);
    if (pi_coll.size() == 0) {
      m_log->debug("No beam hadron found");
      return kinematics;
    }
    const auto pi_p = pi_coll[0]->getMomentum();
    const auto pi_p_mag = edm4eic::magnitude(pi_p);
    const auto pi_mass = pi_coll[0]->getPDG() == 2212 ? m_proton : m_neutron;
    const PxPyPzEVector pi(pi_p.x, pi_p.y, pi_p.z, std::hypot(pi_p_mag, pi_mass));

    // Get first scattered electron
    // Scattered electron. Currently taken as first status==1 electron in HEPMC record,
    // which seems to be correct based on a cursory glance at the Pythia8 output. In the future,
    // it may be better to trace back each final-state electron and see which one originates from
    // the beam.
    const auto ef_coll = find_first_scattered_electron(mcparts);
    if (ef_coll.size() == 0) {
      m_log->debug("No truth scattered electron found");
      return kinematics;
    }
    const auto ef_p = ef_coll[0]->getMomentum();
    const auto ef_p_mag = edm4eic::magnitude(ef_p);
    const auto ef_mass = m_electron;
    const PxPyPzEVector ef(ef_p.x, ef_p.y, ef_p.z, std::hypot(ef_p_mag, ef_mass));

    // DIS kinematics calculations
    const auto q = ei - ef;
    const auto q_dot_pi = q.Dot(pi);
    const auto Q2 = -q.Dot(q);
    const auto y = q_dot_pi / ei.Dot(pi);
    const auto nu = q_dot_pi / pi_mass;
    const auto x = Q2 / (2.*q_dot_pi);
    const auto W = sqrt(pi_mass*pi_mass + 2.*q_dot_pi - Q2);
    edm4eic::MutableInclusiveKinematics kin(x, Q2, W, y, nu);

    m_log->debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(),
            kin.getQ2(), kin.getW(), kin.getY(), kin.getNu());

    kinematics.push_back(new edm4eic::InclusiveKinematics(kin));

    return kinematics;
  }

} // namespace Jug::Reco
