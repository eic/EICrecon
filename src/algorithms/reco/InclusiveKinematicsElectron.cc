// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Wouter Deconinck, Tooba Ali

#include <algorithm>
#include <cmath>
#include <vector>

#include "Beam.h"
#include "Boost.h"
#include "InclusiveKinematicsElectron.h"

#include "Math/Vector4D.h"
using ROOT::Math::PxPyPzEVector;

// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/MCRecoParticleAssociationCollection.h"
#include "edm4eic/ReconstructedParticleCollection.h"
#include "edm4eic/InclusiveKinematicsCollection.h"
#include "ParticlesWithAssociation.h"

namespace eicrecon {

  void InclusiveKinematicsElectron::init(std::shared_ptr<spdlog::logger> logger) {
    m_log = logger;
    // m_pidSvc = service("ParticleSvc");
    // if (!m_pidSvc) {
    //   m_log->debug("Unable to locate Particle Service. "
    //     "Make sure you have ParticleSvc in the configuration.");
    // }
  }

  std::vector<edm4eic::InclusiveKinematics*> InclusiveKinematicsElectron::execute(
    std::vector<const edm4hep::MCParticle *> mcparts,
    std::vector<const edm4eic::ReconstructedParticle *> rcparts,
    std::vector<const edm4eic::MCRecoParticleAssociation *> rcassoc) {

    // Resulting inclusive kinematics
    std::vector<edm4eic::InclusiveKinematics *> kinematics;

    // 1. find_if
    //const auto mc_first_electron = std::find_if(
    //  mcparts.begin(),
    //  mcparts.end(),
    //  [](const auto& p){ return p.getPDG() == 11; });

    // 2a. simple loop over iterator (post-increment)
    //auto mc_first_electron = mcparts.end();
    //for (auto p = mcparts.begin(); p != mcparts.end(); p++) {
    //  if (p.getPDG() == 11) {
    //    mc_first_electron = p;
    //    break;
    //  }
    //}
    // 2b. simple loop over iterator (pre-increment)
    //auto mc_first_electron = mcparts.end();
    //for (auto p = mcparts.begin(); p != mcparts.end(); ++p) {
    //  if (p.getPDG() == 11) {
    //    mc_first_electron = p;
    //    break;
    //  }
    //}

    // 3. pre-initialized simple loop
    //auto mc_first_electron = mcparts.begin();
    //for (; mc_first_electron != mcparts.end(); ++mc_first_electron) {
    //  if (mc_first_electron.getPDG() == 11) {
    //    break;
    //  }
    //}

    // 4a. iterator equality
    //if (mc_first_electron == mcparts.end()) {
    //  debug() << "No electron found" << endmsg;
    //  return StatusCode::FAILURE;
    //}
    // 4b. iterator inequality
    //if (!(mc_first_electron != mcparts.end())) {
    //  debug() << "No electron found" << endmsg;
    //  return StatusCode::FAILURE;
    //}

    // 5. ranges and views
    //auto is_electron = [](const auto& p){ return p.getPDG() == 11; };
    //for (const auto& e: mcparts | std::views::filter(is_electron)) {
    //  break;
    //}

    // Get incoming electron beam
    const auto ei_coll = find_first_beam_electron(mcparts);
    if (ei_coll.size() == 0) {
      m_log->debug("No beam electron found");
      return kinematics;
    }
    const PxPyPzEVector ei(
      round_beam_four_momentum(
        ei_coll[0]->getMomentum(),
        m_electron,
        {-5.0, -10.0, -18.0},
        0.0)
      );

    // Get incoming hadron beam
    const auto pi_coll = find_first_beam_hadron(mcparts);
    if (pi_coll.size() == 0) {
      m_log->debug("No beam hadron found");
      return kinematics;
    }
    const PxPyPzEVector pi(
      round_beam_four_momentum(
        pi_coll[0]->getMomentum(),
        pi_coll[0]->getPDG() == 2212 ? m_proton : m_neutron,
        {41.0, 100.0, 275.0},
        m_crossingAngle)
      );

    // Get first scattered electron
    const auto ef_coll = find_first_scattered_electron(mcparts);
    if (ef_coll.size() == 0) {
      m_log->debug("No truth scattered electron found");
      return kinematics;
    }
    // Associate first scattered electron with reconstructed electrons
    //const auto ef_assoc = std::find_if(
    //  rcassoc.begin(),
    //  rcassoc.end(),
    //  [&ef_coll](const auto& a){ return a.getSimID() == ef_coll[0].getObjectID().index; });
    auto ef_assoc = rcassoc.begin();
    for (; ef_assoc != rcassoc.end(); ++ef_assoc) {
      if ((*ef_assoc)->getSimID() == (unsigned) ef_coll[0]->getObjectID().index) {
        break;
      }
    }
    if (!(ef_assoc != rcassoc.end())) {
      m_log->debug("Truth scattered electron not in reconstructed particles");
      return kinematics;
    }
    const auto ef_rc{(*ef_assoc)->getRec()};
    const auto ef_rc_id{ef_rc.getObjectID().index};

    // Loop over reconstructed particles to get outgoing scattered electron
    // Use the true scattered electron from the MC information
    std::vector<PxPyPzEVector> electrons;
    for (const auto& p: rcparts) {
      if (p->getObjectID().index == ef_rc_id) {
        electrons.emplace_back(p->getMomentum().x, p->getMomentum().y, p->getMomentum().z, p->getEnergy());
        break;
      }
    }

    // If no scattered electron was found
    if (electrons.size() == 0) {
      m_log->debug("No scattered electron found");
      return kinematics;
    }

    // DIS kinematics calculations
    const auto ef = electrons.front();
    const auto q = ei - ef;
    const auto q_dot_pi = q.Dot(pi);
    const auto Q2 = -q.Dot(q);
    const auto y = q_dot_pi / ei.Dot(pi);
    const auto nu = q_dot_pi / m_proton;
    const auto x = Q2 / (2. * q_dot_pi);
    const auto W = sqrt( + 2.*q_dot_pi - Q2);
    edm4eic::MutableInclusiveKinematics kin(x, Q2, W, y, nu);
    kin.setScat(ef_rc);

    m_log->debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(),
            kin.getQ2(), kin.getW(), kin.getY(), kin.getNu());

    kinematics.push_back(new edm4eic::InclusiveKinematics(kin));

    return kinematics;
  }


} // namespace Jug::Reco
