// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
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

  try {
    m_gammaZMax =
        m_cfg.gammaZMaxOffset + m_detector->constant<double>(m_cfg.offsetPositionName) / dd4hep::mm;
  } catch (std::runtime_error&) {
    m_gammaZMax = m_cfg.gammaZMaxOffset + 35800;
    trace("Failed to get {} from the detector, using default value of {}", m_cfg.offsetPositionName,
          m_gammaZMax);
  }

  if (m_cfg.neutronScaleCorrCoeffHcal.size() < 3) {
    error("Invalid configuration.  m_cfg.neutronScaleCorrCoeffHcal should have at least 3 "
          "parameters");
    throw std::runtime_error("Invalid configuration.  m_cfg.neutronScaleCorrCoeffHcal should have "
                             "at least 3 parameters");
  }
  if (m_cfg.gammaScaleCorrCoeffHcal.size() < 3) {
    error("Invalid configuration.  m_cfg.gamma_scale_corr_coeff_ecal should have at least 3 "
          "parameters");
    throw std::runtime_error("Invalid configuration.  m_cfg.gamma_scale_corr_coeff_ecal should "
                             "have at least 3 parameters");
  }
  trace("gamma detection params:   max length={},   max width={},   max z={}", m_cfg.gammaMaxLength,
        m_cfg.gammaMaxWidth, m_gammaZMax);
}
/** calculates the correction for a given uncorrected total energy and a set of coefficients*/
double FarForwardNeutralsReconstruction::calc_corr(double Etot, const std::vector<double>& coeffs) {
  return coeffs[0] + coeffs[1] / sqrt(Etot) + coeffs[2] / Etot;
}

/**
     check that the cluster position is within the correct range,
     and that the sqrt(largest eigenvalue) is less than gamma_max_length,
     and that the sqrt(second largest eigenvalue) is less than gamma_max_width
  */
bool FarForwardNeutralsReconstruction::isGamma(const edm4eic::Cluster& cluster) const {
  double l1 = sqrt(cluster.getShapeParameters(4)) * dd4hep::mm;
  double l2 = sqrt(cluster.getShapeParameters(5)) * dd4hep::mm;
  double l3 = sqrt(cluster.getShapeParameters(6)) * dd4hep::mm;

  //z in the local coordinates
  double z = (cluster.getPosition().z * cos(m_cfg.globalToProtonRotation) +
              cluster.getPosition().x * sin(m_cfg.globalToProtonRotation)) *
             dd4hep::mm;
  trace("z recon = {}", z);
  trace("l1 = {}, l2 = {}, l3 = {}", l1, l2, l3);
  bool isZMoreThanMax = (z > m_gammaZMax);
  bool isLengthMoreThanMax =
      (l1 > m_cfg.gammaMaxLength || l2 > m_cfg.gammaMaxLength || l3 > m_cfg.gammaMaxLength);
  bool areWidthsMoreThanMax = static_cast<int>(l1 > m_cfg.gammaMaxWidth) +
                                  static_cast<int>(l2 > m_cfg.gammaMaxWidth) +
                                  static_cast<int>(l3 > m_cfg.gammaMaxWidth) >=
                              2;
  return !(isZMoreThanMax || isLengthMoreThanMax || areWidthsMoreThanMax);
}

void FarForwardNeutralsReconstruction::process(
    const FarForwardNeutralsReconstruction::Input& input,
    const FarForwardNeutralsReconstruction::Output& output) const {
  const auto [clustersHcal] = input;
  auto [out_neutrals]       = output;

  double Etot_hcal = 0;
  double Emax      = 0;
  edm4hep::Vector3f n_position;
  for (const auto& cluster : *clustersHcal) {
    double E = cluster.getEnergy();

    if (isGamma(cluster)) {
      auto rec_part = out_neutrals->create();
      rec_part.setPDG(22);
      edm4hep::Vector3f position = cluster.getPosition();
      double corr                = calc_corr(E, m_cfg.gammaScaleCorrCoeffHcal);
      E                          = E / (1 + corr);
      double p                   = E;
      double r                   = edm4hep::utils::magnitude(position);
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
    if (E > Emax) {
      Emax       = E;
      n_position = cluster.getPosition();
    }
  }

  double Etot                   = Etot_hcal;
  static const double m_neutron = m_particleSvc.particle(2112).mass;
  int n_neutrons                = 0;
  if (Etot > 0 && Emax > 0) {
    auto rec_part = out_neutrals->create();
    double corr   = calc_corr(Etot, m_cfg.neutronScaleCorrCoeffHcal);
    Etot_hcal     = Etot_hcal / (1 + corr);
    Etot          = Etot_hcal;
    rec_part.setEnergy(Etot);
    rec_part.setPDG(2112);
    double p                   = sqrt(Etot * Etot - m_neutron * m_neutron);
    double r                   = edm4hep::utils::magnitude(n_position);
    edm4hep::Vector3f momentum = n_position * (p / r);
    rec_part.setMomentum(momentum);
    rec_part.setReferencePoint(n_position);
    rec_part.setCharge(0);
    rec_part.setMass(m_neutron);
    for (const auto& cluster : *clustersHcal) {
      rec_part.addToClusters(cluster);
    }
    n_neutrons = 1;
  } else {
    n_neutrons = 0;
  }
  debug("Found {} neutron candidates", n_neutrons);
}
} // namespace eicrecon
