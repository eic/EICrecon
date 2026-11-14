// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Sebouh Paul, Baptiste Fraisse

#pragma once

#include "algorithms/reco/FarForwardNeutralsReconstruction.h"
#include "algorithms/reco/FarForwardNeutralsReconstructionConfig.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class FarForwardNeutralsReconstruction_factory
    : public JOmniAlgorithmFactory<FarForwardNeutralsReconstruction_factory,
                                   eicrecon::FarForwardNeutralsReconstruction,
                                   FarForwardNeutralsReconstructionConfig> {
private:
  ParameterRef<std::string> m_offset_position_name{this, "offsetPositionName",
                                                   config().offsetPositionName};

  ParameterRef<std::vector<double>> m_n_scale_corr_coeff_hcal_zdc{
      this, "neutronScaleCorrCoeffHcalZDC", config().neutronScaleCorrCoeffHcalZDC};

  ParameterRef<std::vector<double>> m_gamma_scale_corr_coeff_hcal_zdc{
      this, "gammaScaleCorrCoeffHcalZDC", config().gammaScaleCorrCoeffHcalZDC};

  ParameterRef<std::vector<double>> m_n_scale_corr_coeff_b0ecal{
      this, "neutronScaleCorrCoeffB0Ecal", config().neutronScaleCorrCoeffB0Ecal};

  ParameterRef<std::vector<double>> m_gamma_scale_corr_coeff_b0ecal{
      this, "gammaScaleCorrCoeffB0Ecal", config().gammaScaleCorrCoeffB0Ecal};

  ParameterRef<std::vector<double>> m_n_scale_corr_coeff_ecalendcapp{
      this, "neutronScaleCorrCoeffEcalEndcapP", config().neutronScaleCorrCoeffEcalEndcapP};

  ParameterRef<std::vector<double>> m_gamma_scale_corr_coeff_ecalendcapp{
      this, "gammaScaleCorrCoeffEcalEndcapP", config().gammaScaleCorrCoeffEcalEndcapP};

  ParameterRef<std::vector<double>> m_n_scale_corr_coeff_lfhcal{
      this, "neutronScaleCorrCoeffLFHCAL", config().neutronScaleCorrCoeffLFHCAL};

  ParameterRef<std::vector<double>> m_gamma_scale_corr_coeff_lfhcal{
      this, "gammaScaleCorrCoeffLFHCAL", config().gammaScaleCorrCoeffLFHCAL};

  ParameterRef<double> m_cluster_emin_hcal_zdc{this, "clusterEminHcalZDC",
                                               config().clusterEminHcalZDC};

  ParameterRef<double> m_cluster_emin_b0ecal{this, "clusterEminB0Ecal", config().clusterEminB0Ecal};

  ParameterRef<double> m_cluster_emin_ecalendcapp{this, "clusterEminEcalEndcapP",
                                                  config().clusterEminEcalEndcapP};

  ParameterRef<double> m_cluster_emin_lfhcal{this, "clusterEminLFHCAL", config().clusterEminLFHCAL};

  ParameterRef<double> m_global_to_proton_rotation{this, "globalToProtonRotation",
                                                   config().globalToProtonRotation};

  ParameterRef<double> m_gamma_zmax_offset{this, "gammaZMaxOffset", config().gammaZMaxOffset};

  ParameterRef<double> m_gamma_max_length{this, "gammaMaxLength", config().gammaMaxLength};

  ParameterRef<double> m_gamma_max_width{this, "gammaMaxWidth", config().gammaMaxWidth};
};

} // namespace eicrecon
