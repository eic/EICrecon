// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul

#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <gsl/pointers>
#include <stdexcept>
#include <vector>

#include "FarForwardNeutralsReconstruction.h"

/**
 Creates photon candidate Reconstructed Particles, using clusters which fulfill cuts on the position of their CoG position, length (sqrt of largest eigenvector of their moment matrix), and width (sqrt of second largest eigenvector of their moment matrix).  Its energy is the energy of the cluster times a correction factor.
 Also creates a "neutron candidate" Reconstructed Particle consisting of all remaining clusters in the collection. Its energy is the sum of the energies of the constituent clusters
 times a correction factor, and its direction is the direction from the origin to the position
 of the most energetic cluster.  The correction factors in both cases are given by 1/(1+c[0]+c[1]/sqrt(E)+c[2]/E),
 where c is the coefficients and E is the uncorrected energy in GeV.  Different correction coefficients are used for photons vs for neutrons.  This form was chosen
 empirically based on the discrepancies in single-neutron and single-photon MC simulations between the uncorrected
 reconstructed energies and the truth energies of the neutrons.  The parameter values should be co-tuned with those of the clustering algorithm being used.
 */

namespace eicrecon {

    void FarForwardNeutralsReconstruction::init() {
      if (m_cfg.n_scale_corr_coeff_hcal.size() < 3) {
        error("Invalid configuration.  m_cfg.n_scale_corr_coeff_hcal should have at least 3 parameters");
        throw std::runtime_error("Invalid configuration.  m_cfg.n_scale_corr_coeff_hcal should have at least 3 parameters");
      }
      if (m_cfg.gamma_scale_corr_coeff_hcal.size() < 3) {
        error("Invalid configuration.  m_cfg.gamma_scale_corr_coeff_ecal should have at least 3 parameters");
        throw std::runtime_error("Invalid configuration.  m_cfg.gamma_scale_corr_coeff_ecal should have at least 3 parameters");
      }
    }
    /** calculates the correction for a given uncorrected total energy and a set of coefficients*/
    double FarForwardNeutralsReconstruction::calc_corr(double Etot, const std::vector<double>& coeffs) const{
      return coeffs[0]+coeffs[1]/sqrt(Etot)+coeffs[2]/Etot;
    }

  /**
     check that the cluster position is within the correct range,
     and that the sqrt(largest eigenvalue) is less than gamma_max_length,
     and that the sqrt(second largest eigenvalue) is less than gamma_max_width
  */
    bool FarForwardNeutralsReconstruction::isGamma(const edm4eic::Cluster& cluster) const{
      double l1=sqrt(cluster.getShapeParameters(4))*dd4hep::mm;
      double l2=sqrt(cluster.getShapeParameters(5))*dd4hep::mm;
      double l3=sqrt(cluster.getShapeParameters(6))*dd4hep::mm;

      //z in the local coordinates
      double z= (cluster.getPosition().z*cos(m_cfg.rot_y)+cluster.getPosition().x*sin(m_cfg.rot_y))*dd4hep::mm;

      if (z> m_cfg.gamma_zmax)
        return false;
      if (l1> m_cfg.gamma_max_length || l2> m_cfg.gamma_max_length || l3 > m_cfg.gamma_max_length)
        return false;
      if ((l1> m_cfg.gamma_max_width) + (l2> m_cfg.gamma_max_width) + (l3 > m_cfg.gamma_max_width)>2)
        return false;
      return true;

    }



    void FarForwardNeutralsReconstruction::process(const FarForwardNeutralsReconstruction::Input& input,
                      const FarForwardNeutralsReconstruction::Output& output) const {
      const auto [clustersHcal] = input;
      auto [out_neutrons, out_gammas] = output;

      double Etot_hcal=0;
      double Emax=0;
      edm4hep::Vector3f n_position;
      for (const auto& cluster : *clustersHcal) {
          double E = cluster.getEnergy();

          if(isGamma(cluster)){
            auto rec_part = out_gammas->create();
            rec_part.setPDG(22);
            edm4hep::Vector3f position = cluster.getPosition();
            double corr=calc_corr(E,m_cfg.gamma_scale_corr_coeff_hcal);
            E=E/(1+corr);
            double p = E;
            double r = edm4hep::utils::magnitude(position);
            edm4hep::Vector3f momentum = position * (p / r);
            rec_part.setEnergy(E);
            rec_part.setMomentum(momentum);
            rec_part.setReferencePoint(position);
            rec_part.setCharge(0);
            rec_part.setMass(0);
            rec_part.addToClusters(cluster);
            continue;
          }

          Etot_hcal += E;
          if (E > Emax){
            Emax = E;
            n_position = cluster.getPosition();
          }
      }

      double Etot=Etot_hcal;
      if (Etot > 0 && Emax > 0){
          auto rec_part = out_neutrons->create();
          double corr=calc_corr(Etot,m_cfg.n_scale_corr_coeff_hcal);
          Etot_hcal=Etot_hcal/(1+corr);
          Etot=Etot_hcal;
          rec_part.setEnergy(Etot);
          rec_part.setPDG(2112);
          double p = sqrt(Etot*Etot-m_neutron*m_neutron);
          double r = edm4hep::utils::magnitude(n_position);
          edm4hep::Vector3f momentum = n_position * (p / r);
          rec_part.setMomentum(momentum);
          rec_part.setReferencePoint(n_position);
          rec_part.setCharge(0);
          rec_part.setMass(m_neutron);
          for (const auto& cluster : *clustersHcal){
            rec_part.addToClusters(cluster);
          }
      }

    }
}
