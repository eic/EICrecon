// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul

#include <Evaluator/DD4hepUnits.h>
#include <TVector3.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <stdexcept>
#include <vector>

#include "FarForwardLambdaReconstruction.h"
#include "TLorentzVector.h"
/**
Creates Lambda candidates from a neutron and two photons from a pi0 decay
 */

namespace eicrecon {

void FarForwardLambdaReconstruction::init() {

  try {
    m_zMax = m_detector->constant<double>(m_cfg.offsetPositionName);
  } catch (std::runtime_error&) {
    m_zMax = 35800; // default value
    trace("Failed to get {} from the detector, using default value of {}", m_cfg.offsetPositionName,
          m_zMax);
  }
}

/* converts one type of vector format to another */
void toTVector3(TVector3& v1, const edm4hep::Vector3f& v2) { v1.SetXYZ(v2.x, v2.y, v2.z); }

void FarForwardLambdaReconstruction::process(
    const FarForwardLambdaReconstruction::Input& input,
    const FarForwardLambdaReconstruction::Output& output) const {
  const auto [neutrals]                                = input;
  auto [out_lambdas, out_decay_products]               = output;
  std::vector<edm4eic::ReconstructedParticle> neutrons = {};
  std::vector<edm4eic::ReconstructedParticle> gammas   = {};
  for (auto part : *neutrals) {
    if (part.getPDG() == 2112) {
      neutrons.push_back(part);
    }
    if (part.getPDG() == 22) {
      gammas.push_back(part);
    }
  }

  if (neutrons.empty() || gammas.size() < 2) {
    return;
  }

  static const double m_neutron = m_particleSvc.particle(2112).mass;
  static const double m_pi0     = m_particleSvc.particle(111).mass;
  static const double m_lambda  = m_particleSvc.particle(3122).mass;

  for (std::size_t i_n = 0; i_n < neutrons.size(); i_n++) {
    for (std::size_t i_1 = 0; i_1 < gammas.size() - 1; i_1++) {
      for (std::size_t i_2 = i_1 + 1; i_2 < gammas.size(); i_2++) {

        double En = neutrons[i_n].getEnergy();
        double pn = sqrt(En * En - m_neutron * m_neutron);
        double E1 = gammas[i_1].getEnergy();
        double E2 = gammas[i_2].getEnergy();
        TVector3 xn;
        TVector3 x1;
        TVector3 x2;

        toTVector3(xn, neutrons[0].getReferencePoint() * dd4hep::mm);
        toTVector3(x1, gammas[0].getReferencePoint() * dd4hep::mm);
        toTVector3(x2, gammas[1].getReferencePoint() * dd4hep::mm);

        xn.RotateY(-m_cfg.globalToProtonRotation);
        x1.RotateY(-m_cfg.globalToProtonRotation);
        x2.RotateY(-m_cfg.globalToProtonRotation);

        debug("nx recon = {}, g1x recon = {}, g2x recon = {}", xn.X(), x1.X(), x2.X());
        debug("nz recon = {}, g1z recon = {}, g2z recon = {}, z face = {}", xn.Z(), x1.Z(), x2.Z(),
              m_zMax);

        TVector3 vtx(0, 0, 0);
        double f                   = 0;
        double df                  = 0.5;
        double theta_open_expected = 2 * asin(m_pi0 / (2 * sqrt(E1 * E2)));
        TLorentzVector n;
        TLorentzVector g1;
        TLorentzVector g2;
        TLorentzVector lambda;
        for (int i = 0; i < m_cfg.iterations; i++) {
          n                 = {pn * (xn - vtx).Unit(), En};
          g1                = {E1 * (x1 - vtx).Unit(), E1};
          g2                = {E2 * (x2 - vtx).Unit(), E2};
          double theta_open = g1.Angle(g2.Vect());
          lambda            = n + g1 + g2;
          if (theta_open > theta_open_expected) {
            f -= df;
          } else if (theta_open < theta_open_expected) {
            f += df;
          }

          vtx = lambda.Vect() * (f * m_zMax / lambda.Z());
          df /= 2;
        }

        double mass_rec = lambda.M();
        if (std::abs(mass_rec - m_lambda) > m_cfg.lambdaMaxMassDev) {
          continue;
        }

        // rotate everything back to the lab coordinates.
        vtx.RotateY(m_cfg.globalToProtonRotation);
        lambda.RotateY(m_cfg.globalToProtonRotation);
        n.RotateY(m_cfg.globalToProtonRotation);
        g1.RotateY(m_cfg.globalToProtonRotation);
        g2.RotateY(m_cfg.globalToProtonRotation);

        auto b = -lambda.BoostVector();
        n.Boost(b);
        g1.Boost(b);
        g2.Boost(b);

        //convert vertex to mm:
        vtx = vtx * (1 / dd4hep::mm);

        auto rec_lambda = out_lambdas->create();
        rec_lambda.setPDG(3122);

        rec_lambda.setEnergy(lambda.E());
        rec_lambda.setMomentum({static_cast<float>(lambda.X()), static_cast<float>(lambda.Y()),
                                static_cast<float>(lambda.Z())});
        rec_lambda.setReferencePoint({static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()),
                                      static_cast<float>(vtx.Z())});
        rec_lambda.setCharge(0);
        rec_lambda.setMass(mass_rec);

        auto neutron_cm = out_decay_products->create();
        neutron_cm.setPDG(2112);
        neutron_cm.setEnergy(n.E());
        neutron_cm.setMomentum(
            {static_cast<float>(n.X()), static_cast<float>(n.Y()), static_cast<float>(n.Z())});
        neutron_cm.setReferencePoint({static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()),
                                      static_cast<float>(vtx.Z())});
        neutron_cm.setCharge(0);
        neutron_cm.setMass(m_neutron);
        //link the reconstructed lambda to the input neutron,
        // and the cm neutron to the input neutron
        rec_lambda.addToParticles(neutrons[i_n]);
        neutron_cm.addToParticles(neutrons[i_n]);

        auto gamma1_cm = out_decay_products->create();
        gamma1_cm.setPDG(22);
        gamma1_cm.setEnergy(g1.E());
        gamma1_cm.setMomentum(
            {static_cast<float>(g1.X()), static_cast<float>(g1.Y()), static_cast<float>(g1.Z())});
        gamma1_cm.setReferencePoint({static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()),
                                     static_cast<float>(vtx.Z())});
        gamma1_cm.setCharge(0);
        gamma1_cm.setMass(0);
        rec_lambda.addToParticles(gammas[i_1]);
        gamma1_cm.addToParticles(gammas[i_1]);

        auto gamma2_cm = out_decay_products->create();
        gamma2_cm.setPDG(22);
        gamma2_cm.setEnergy(g2.E());
        gamma2_cm.setMomentum(
            {static_cast<float>(g2.X()), static_cast<float>(g2.Y()), static_cast<float>(g2.Z())});
        gamma2_cm.setReferencePoint({static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()),
                                     static_cast<float>(vtx.Z())});
        gamma2_cm.setCharge(0);
        gamma2_cm.setMass(0);
        rec_lambda.addToParticles(gammas[i_2]);
        gamma2_cm.addToParticles(gammas[i_2]);
      }
    }
  }
}
} // namespace eicrecon
