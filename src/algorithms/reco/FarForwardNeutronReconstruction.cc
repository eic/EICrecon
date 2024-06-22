// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Sebouh Paul

#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <math.h>
#include <optional>
#include <gsl/pointers>
#include <vector>
#include <stdexcept>

#include "FarForwardNeutronReconstruction.h"

/**
 Creates a "neutron candidate" Reconstructed Particle consisting of all clusters in a
 given ClusterCollection.  Its energy is the sum of the energies of the constituent clusters
 times a correction factor, and its direction is the direction from the origin to the position
 of the most energetic cluster.  The correction factor is given by 1/(1+c[0]+c[1]/sqrt(E)+c[2]/E),
 where c is the coefficients and E is the uncorrected energy in GeV.  This form was chosen
 empirically based on the discrepancies in single-neutron MC simulations between the uncorrected
 reconstructed energies and the truth energies of the neutrons.
 */

namespace eicrecon {

    void FarForwardNeutronReconstruction::init() {
      if (m_cfg.scale_corr_coeff.size() < 3) {
        error("Invalid configuration.  m_cfg.scale_corr_coeff should have at least 3 parameters");
        throw std::runtime_error("Invalid configuration.  m_cfg.scale_corr_coeff should have at least 3 parameters");
      }
    }
    double FarForwardNeutronReconstruction::calc_corr(double Etot) const{
      auto coeffs=m_cfg.scale_corr_coeff;
      return coeffs[0]+coeffs[1]/sqrt(Etot)+coeffs[2]/Etot;
    }
    void FarForwardNeutronReconstruction::process(const FarForwardNeutronReconstruction::Input& input,
                      const FarForwardNeutronReconstruction::Output& output) const {
      const auto [clustersHcal,clustersEcal] = input;
      auto [out_neutrons] = output;
      
      double Etot=0;
      double Emax=0;
      double x=0;
      double y=0;
      double z=0;
      for (const auto& cluster : *clustersHcal) {
          double E = cluster.getEnergy();
          Etot+=E;
          if(E>Emax){
            Emax=E;
            x=cluster.getPosition().x;
            y=cluster.getPosition().y;
            z=cluster.getPosition().z;
          }
      }
      for (const auto& cluster : *clustersEcal) {
          double E = cluster.getEnergy();
          Etot+=E;
      }
      if (Etot>0){
          auto rec_part = out_neutrons->create();
          double corr=calc_corr(Etot);
          Etot=Etot/(1+corr);
          rec_part.setEnergy(Etot);
          rec_part.setPDG(2112);
          double p=sqrt(Etot*Etot-m_neutron*m_neutron);
          double r=sqrt(x*x+y*y+z*z);
          double px=p*x/r;
          double py=p*y/r;
          double pz=p*z/r;
          rec_part.setMomentum({(float)px, (float)py, (float)pz});
          rec_part.setCharge(0);
          rec_part.setMass(m_neutron);
          for (const auto& cluster : *clustersHcal){
            rec_part.addToClusters(cluster);
          }
	  for (const auto& cluster : *clustersEcal){
            rec_part.addToClusters(cluster);
          }
      }
        //m_log->debug("Found {} neutron candidates", out_neutrons->size());

    }
}
