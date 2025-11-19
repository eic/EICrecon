// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul

#include <Evaluator/DD4hepUnits.h>
#include <TVector3.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <stdexcept>
#include <vector>

#include "FarForwardSigma0Reconstruction.h"
#include "TLorentzVector.h"
/**
Creates Sigma0 candidates from a neutron, two photons from a pi0 decay, and another photon,
determining which pair belongs to the pi0 based on the reconstructed mass of the intermediatelambda candidate.  
 */

namespace eicrecon {

void FarForwardSigma0Reconstruction::init() {

}

///* converts one type of vector format to another */
//void toTVector3(TVector3& v1, const edm4hep::Vector3f& v2) { v1.SetXYZ(v2.x, v2.y, v2.z); }

void FarForwardSigma0Reconstruction::process(
    const FarForwardSigma0Reconstruction::Input& input,
    const FarForwardSigma0Reconstruction::Output& output) const {
  const auto [neutrals,lambdas]                        = input;
  auto [out_sigmas, out_decay_products]                = output;

  std::vector<edm4eic::ReconstructedParticle> gammas   = {};
  for (auto part : *neutrals) {
    if (part.getPDG() == 22) {
      gammas.push_back(part);
    }
  }



  static const double m_sigma0   = m_particleSvc.particle(3212).mass;
  static const double m_lambda  = m_particleSvc.particle(3122).mass;

  for (auto lambda : *lambdas){
    for (auto gamma: gammas) {
      
      //only match a gamma to a lambda candidate that does not contain that gamma
      bool found=0;
      for (auto daughter: lambda.getParticles()){
	if (daughter==gamma){
	  found=1;
	  break;
	}
      }
      if(found) continue;
      
      TLorentzVector p_lambda;
      p_lambda={{lambda.getMomentum().x, lambda.getMomentum().y, lambda.getMomentum().z}, lambda.getEnergy()};
      TLorentzVector p_gamma;
      p_gamma={{gamma.getMomentum().x, gamma.getMomentum().y, gamma.getMomentum().z}, gamma.getEnergy()};

      TLorentzVector p_sigma=p_lambda+p_gamma;
      

      double mass_rec = p_sigma.M();
      if (std::abs(mass_rec - m_sigma0) > m_cfg.sigma0MaxMassDev) {
	continue;
      }

     
      auto b = -p_sigma.BoostVector();
      p_lambda.Boost(b);
      p_gamma.Boost(b);


      auto rec_sigma = out_sigmas->create();
      rec_sigma.setPDG(3212);

      rec_sigma.setEnergy(p_sigma.E());
      rec_sigma.setMomentum({static_cast<float>(p_sigma.X()), static_cast<float>(p_sigma.Y()),
                                static_cast<float>(p_sigma.Z())});
      rec_sigma.setReferencePoint({0,0,0});
                                      
      rec_sigma.setCharge(0);
      rec_sigma.setMass(mass_rec);

      auto lambda_cm = out_decay_products->create();
      lambda_cm.setPDG(3122);
      lambda_cm.setEnergy(p_lambda.E());
      lambda_cm.setMomentum(
			    {static_cast<float>(p_lambda.X()), static_cast<float>(p_lambda.Y()), static_cast<float>(p_lambda.Z())});
        //neutron_cm.setReferencePoint({static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()),
        //                              static_cast<float>(vtx.Z())});
      lambda_cm.setCharge(0);
      lambda_cm.setMass(m_lambda);
        //link the reconstructed lambda to the input neutron,
        // and the cm neutron to the input neutron
      rec_sigma.addToParticles(lambda);
      lambda_cm.addToParticles(lambda);

        
      auto gamma_cm = out_decay_products->create();
      gamma_cm.setPDG(22);
      gamma_cm.setEnergy(p_gamma.E());
      gamma_cm.setMomentum(
			   {static_cast<float>(p_gamma.X()), static_cast<float>(p_gamma.Y()), static_cast<float>(p_gamma.Z())});
      gamma_cm.setReferencePoint({0,0,0});
                                     
      gamma_cm.setCharge(0);
      gamma_cm.setMass(0);
      gamma_cm.addToParticles(gamma);
      rec_sigma.addToParticles(gamma);
      }
    }
  }

} // namespace eicrecon
