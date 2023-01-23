// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#include "Gaudi/Algorithm.h"
#include "GaudiAlg/GaudiTool.h"
#include "GaudiAlg/Producer.h"
#include "GaudiAlg/Transformer.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/PhysicalConstants.h"
#include <algorithm>
#include <cmath>

#include "JugBase/IParticleSvc.h"
#include "JugBase/DataHandle.h"

#include "JugBase/Utilities/Beam.h"

#include "Math/Vector4D.h"
using ROOT::Math::PxPyPzEVector;

// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/InclusiveKinematicsCollection.h"

#include "edm4eic/vector_utils.h"

namespace Jug::Fast {

class InclusiveKinematicsTruth : public GaudiAlgorithm {
private:
  DataHandle<edm4hep::MCParticleCollection> m_inputMCParticleCollection{
    "inputMCParticles",
    Gaudi::DataHandle::Reader,
    this};
  DataHandle<edm4eic::InclusiveKinematicsCollection> m_outputInclusiveKinematicsCollection{
    "outputInclusiveKinematics",
    Gaudi::DataHandle::Writer,
    this};

  SmartIF<IParticleSvc> m_pidSvc;
  double m_proton{0};
  double m_neutron{0};
  double m_electron{0};

public:
  InclusiveKinematicsTruth(const std::string& name, ISvcLocator* svcLoc)
      : GaudiAlgorithm(name, svcLoc) {
    declareProperty("inputMCParticles", m_inputMCParticleCollection, "MCParticles");
    declareProperty("outputInclusiveKinematics", m_outputInclusiveKinematicsCollection, "InclusiveKinematicsTruth");
  }

  StatusCode initialize() override {
    if (GaudiAlgorithm::initialize().isFailure()) {
      return StatusCode::FAILURE;
    }

    m_pidSvc = service("ParticleSvc");
    if (!m_pidSvc) {
      error() << "Unable to locate Particle Service. "
              << "Make sure you have ParticleSvc in the configuration."
              << endmsg;
      return StatusCode::FAILURE;
    }
    m_proton = m_pidSvc->particle(2212).mass;
    m_neutron = m_pidSvc->particle(2112).mass;
    m_electron = m_pidSvc->particle(11).mass;

    return StatusCode::SUCCESS;
  }

  StatusCode execute() override {
    // input collections
    const auto& mcparts = *(m_inputMCParticleCollection.get());
    // output collection
    auto& out_kinematics = *(m_outputInclusiveKinematicsCollection.createAndPut());

    // Loop over generated particles to get incoming electron and proton beams
    // and the scattered electron. In the presence of QED radition on the incoming
    // or outgoing electron line, the vertex kinematics will be different than the
    // kinematics calculated using the scattered electron as done here.
    // Also need to update for CC events.

    // Get incoming electron beam
    const auto ei_coll = Jug::Base::Beam::find_first_beam_electron(mcparts);
    if (ei_coll.size() == 0) {
      if (msgLevel(MSG::DEBUG)) {
        debug() << "No beam electron found" << endmsg;
      }
      return StatusCode::SUCCESS;
    }
    const auto ei_p = ei_coll[0].getMomentum();
    const auto ei_p_mag = edm4eic::magnitude(ei_p);
    const auto ei_mass = m_electron;
    const PxPyPzEVector ei(ei_p.x, ei_p.y, ei_p.z, std::hypot(ei_p_mag, ei_mass));

    // Get incoming hadron beam
    const auto pi_coll = Jug::Base::Beam::find_first_beam_hadron(mcparts);
    if (pi_coll.size() == 0) {
      if (msgLevel(MSG::DEBUG)) {
        debug() << "No beam hadron found" << endmsg;
      }
      return StatusCode::SUCCESS;
    }
    const auto pi_p = pi_coll[0].getMomentum();
    const auto pi_p_mag = edm4eic::magnitude(pi_p);
    const auto pi_mass = pi_coll[0].getPDG() == 2212 ? m_proton : m_neutron;
    const PxPyPzEVector pi(pi_p.x, pi_p.y, pi_p.z, std::hypot(pi_p_mag, pi_mass));

    // Get first scattered electron
    // Scattered electron. Currently taken as first status==1 electron in HEPMC record,
    // which seems to be correct based on a cursory glance at the Pythia8 output. In the future,
    // it may be better to trace back each final-state electron and see which one originates from
    // the beam.
    const auto ef_coll = Jug::Base::Beam::find_first_scattered_electron(mcparts);
    if (ef_coll.size() == 0) {
      if (msgLevel(MSG::DEBUG)) {
        debug() << "No truth scattered electron found" << endmsg;
      }
      return StatusCode::SUCCESS;
    }
    const auto ef_p = ef_coll[0].getMomentum();
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
    const auto kin = out_kinematics.create(x, Q2, W, y, nu);

    // Debugging output
    if (msgLevel(MSG::DEBUG)) {
      debug() << "pi = " << pi << endmsg;
      debug() << "ei = " << ei << endmsg;
      debug() << "ef = " << ef << endmsg;
      debug() << "q = " << q << endmsg;
      debug() << "x,Q2,W,y,nu = "
              << kin.getX() << ","
              << kin.getQ2() << ","
              << kin.getW() << ","
              << kin.getY() << ","
              << kin.getNu()
              << endmsg;
    }

    return StatusCode::SUCCESS;
  }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
DECLARE_COMPONENT(InclusiveKinematicsTruth)

} // namespace Jug::Fast
