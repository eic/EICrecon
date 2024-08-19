// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Sebouh Paul

#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <math.h>
#include <gsl/pointers>
#include <stdexcept>
#include <vector>

#include "FarForwardNeutronReconstruction.h"

/**
 Creates a "neutron candidate" Reconstructed Particle consisting of all clusters from a
 specified hadronic calorimeter and (optionally) from a specified Electromagnetic calorimeter.
 Its energy is the sum of the energies of the constituent clusters
 times a correction factor, and its direction is the direction from the origin to the position
 of the most energetic cluster.  The correction factor is given by 1/(1+c[0]+c[1]/sqrt(E)+c[2]/E),
 where c is the coefficients and E is the uncorrected energy in GeV.  This form was chosen
 empirically based on the discrepancies in single-neutron MC simulations between the uncorrected
 reconstructed energies and the truth energies of the neutrons.  Separate correction factors
 are included for the Hcal and Ecal.
 */

namespace eicrecon {

    void FarForwardNeutronReconstruction::init() {
      if (m_cfg.scale_corr_coeff_hcal.size() < 3) {
        error("Invalid configuration.  m_cfg.scale_corr_coeff_hcal should have at least 3 parameters");
        throw std::runtime_error("Invalid configuration.  m_cfg.scale_corr_coeff_hcal should have at least 3 parameters");
      }
      if (m_cfg.scale_corr_coeff_ecal.size() < 3) {
        error("Invalid configuration.  m_cfg.scale_corr_coeff_ecal should have at least 3 parameters");
        throw std::runtime_error("Invalid configuration.  m_cfg.scale_corr_coeff_ecal should have at least 3 parameters");
      }
    }
    /** calculates the correction for a given uncorrected total energy and a set of coefficients*/
    double FarForwardNeutronReconstruction::calc_corr(double Etot, const std::vector<double>& coeffs) const{
      return coeffs[0]+coeffs[1]/sqrt(Etot)+coeffs[2]/Etot;
    }
    void FarForwardNeutronReconstruction::process(const FarForwardNeutronReconstruction::Input& input,
                      const FarForwardNeutronReconstruction::Output& output) const {
      const auto [clustersHcal,clustersEcal] = input;
      auto [out_neutrons] = output;

      double Etot_hcal=0, Etot_ecal=0;
      double Emax=0;
      edm4hep::Vector3f position;
      for (const auto& cluster : *clustersHcal) {
          double E = cluster.getEnergy();
          Etot_hcal += E;
          if (E > Emax){
            Emax = E;
            position = cluster.getPosition();
          }
      }
      for (const auto& cluster : *clustersEcal) {
          double E = cluster.getEnergy();
          Etot_ecal+=E;
      }
      double Etot=Etot_hcal+Etot_ecal;
      if (Etot > 0 && Emax > 0){
          auto rec_part = out_neutrons->create();
          double corr=calc_corr(Etot,m_cfg.scale_corr_coeff_hcal);
          Etot_hcal=Etot_hcal/(1+corr);
          corr=calc_corr(Etot,m_cfg.scale_corr_coeff_ecal);
          Etot_ecal=Etot_ecal/(1+corr);
          Etot=Etot_hcal+Etot_ecal;
          rec_part.setEnergy(Etot);
          rec_part.setPDG(2112);
          double p = sqrt(Etot*Etot-m_neutron*m_neutron);
          double r = edm4hep::utils::magnitude(position);
          edm4hep::Vector3f momentum = position * (p / r);
          rec_part.setMomentum(momentum);
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
