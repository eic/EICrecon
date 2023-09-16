// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#include <algorithm>
#include <cmath>
#include <vector>

#include "Beam.h"
#include "Boost.h"
#include "InclusiveKinematicsJB.h"

#include <Math/Vector4D.h>
using ROOT::Math::PxPyPzEVector;

// Event Model related classes
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/InclusiveKinematicsCollection.h>

namespace eicrecon {

  void InclusiveKinematicsJB::init(std::shared_ptr<spdlog::logger> logger) {
    m_log = logger;
    // m_pidSvc = service("ParticleSvc");
    // if (!m_pidSvc) {
    //   m_log->debug("Unable to locate Particle Service. "
    //     "Make sure you have ParticleSvc in the configuration.");
    // }
  }

  std::unique_ptr<edm4eic::InclusiveKinematicsCollection> InclusiveKinematicsJB::execute(
    const edm4hep::MCParticleCollection& mcparts,
    const edm4eic::ReconstructedParticleCollection& rcparts,
    const edm4eic::MCRecoParticleAssociationCollection& rcassoc) {

    // Resulting inclusive kinematics
    auto kinematics = std::make_unique<edm4eic::InclusiveKinematicsCollection>();
    // Get incoming electron beam
    const auto ei_coll = find_first_beam_electron(mcparts);
    if (ei_coll.size() == 0) {
      m_log->debug("No beam electron found");
      return kinematics;
    }
    const PxPyPzEVector ei(
      round_beam_four_momentum(
        ei_coll[0].getMomentum(),
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
        pi_coll[0].getMomentum(),
        pi_coll[0].getPDG() == 2212 ? m_proton : m_neutron,
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
    //  [&ef_coll](const auto& a){ return a.getSim().getObjectID().index == ef_coll[0].getObjectID().index; });
    auto ef_assoc = rcassoc.begin();
    for (; ef_assoc != rcassoc.end(); ++ef_assoc) {
      if (ef_assoc->getSim().getObjectID().index == (unsigned) ef_coll[0].getObjectID().index) {
        break;
      }
    }
    if (!(ef_assoc != rcassoc.end())) {
      m_log->debug("Truth scattered electron not in reconstructed particles");
      return kinematics;
    }
    const auto ef_rc{ef_assoc->getRec()};
    const auto ef_rc_id{ef_rc.getObjectID().index};

    // Loop over reconstructed particles to get all outgoing particles other than the scattered electron
    // -----------------------------------------------------------------
    // Right now, everything is taken from Reconstructed particles branches.
    //
    // This means the tracking detector is used for charged particles to calculate the momentum,
    // and the magnitude of this momentum plus the true PID to calculate the energy.
    // No requirement is made that these particles produce a hit in any other detector
    //
    // Using the Reconstructed particles branches also means that the reconstruction for neutrals is done using the
    // calorimeter(s) information for the energy and angles, and then using this energy and the true PID to get the
    // magnitude of the momentum.
    // -----------------------------------------------------------------

    // Sums in colinear frame
    double pxsum = 0;
    double pysum = 0;
    double pzsum = 0;
    double Esum = 0;

    // Get boost to colinear frame
    auto boost = determine_boost(ei, pi);

    for (const auto& p: rcparts) {
      // Get the scattered electron index and angle
      if (p.getObjectID().index == ef_rc_id) {

      // Sum over all particles other than scattered electron
      } else {
        // Lorentz vector in lab frame
        PxPyPzEVector hf_lab(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
        // Boost to colinear frame
        PxPyPzEVector hf_boosted = apply_boost(boost, hf_lab);

        pxsum += hf_boosted.Px();
        pysum += hf_boosted.Py();
        pzsum += hf_boosted.Pz();
        Esum += hf_boosted.E();
      }
    }

    // DIS kinematics calculations
    auto sigma_h = Esum - pzsum;
    auto ptsum = sqrt(pxsum*pxsum + pysum*pysum);

    // Sigma zero or negative
    if (sigma_h <= 0) {
      m_log->debug("Sigma zero or negative");
      return kinematics;
    }

    // Calculate kinematic variables
    const auto y_jb = sigma_h / (2.*ei.energy());
    const auto Q2_jb = ptsum*ptsum / (1. - y_jb);
    const auto x_jb = Q2_jb / (4.*ei.energy()*pi.energy()*y_jb);
    const auto nu_jb = Q2_jb / (2.*m_proton*x_jb);
    const auto W_jb = sqrt(m_proton*m_proton + 2*m_proton*nu_jb - Q2_jb);
    auto kin = kinematics->create(x_jb, Q2_jb, W_jb, y_jb, nu_jb);
    kin.setScat(ef_rc);

    m_log->debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(),
            kin.getQ2(), kin.getW(), kin.getY(), kin.getNu());

    return kinematics;
  }


} // namespace Jug::Reco
