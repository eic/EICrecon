// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// ROOT
#include <TH1D.h>
#include <TH2D.h>
#include <TMath.h>

// data model
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/utils/vector_utils.h>

// EICrecon
#include <spdlog/spdlog.h>
#include "Tools.h"

namespace eicrecon {

  // bin settings
  //---------------------------------------------------------------
  class Binning {
    public:
      static constexpr int    momentum_bins = 100;
      static constexpr int    momentum_max  = 60;
      static constexpr int    npe_bins      = 100;
      static constexpr double npe_max       = 100;
      static constexpr int    theta_bins    = 100;
      static constexpr double theta_max     = 1000;
      static constexpr int    phi_bins      = 100;
      static int pdg_bins() { return Tools::GetNumPDGs() + 1; }
  };


  // analysis for one radiator
  //---------------------------------------------------------------
  class RadiatorAnalysis {
    public:
      RadiatorAnalysis(TString rad_name) : m_rad_name(rad_name) {
        m_npe_dist = new TH1D(
            "npe_dist_"+m_rad_name,
            "Overall NPE for "+m_rad_name+";NPE",
            Binning::npe_bins, 0, Binning::npe_max
            );
        m_theta_dist = new TH1D(
            "theta_dist_"+m_rad_name,
            "Estimated Cherenkov Angle for "+m_rad_name+";#theta [mrad]",
            Binning::theta_bins, 0, Binning::theta_max
            );
        m_photon_theta_vs_phi = new TH2D(
            "photon_theta_vs_phi_"+m_rad_name,
            "Estimated Photon #theta vs #phi for "+m_rad_name+";#phi [rad];#theta [mrad]",
            Binning::phi_bins, -TMath::Pi(), TMath::Pi(),
            Binning::theta_bins, 0, Binning::theta_max
            );
        m_highest_weight_dist = new TH1D(
            "highest_weight_dist_"+m_rad_name,
            "Highest PDG Weight for "+m_rad_name+";PDG",
            Binning::pdg_bins(), 0, Binning::pdg_bins()
            );
        m_npe_vs_p = new TH2D(
            "npe_vs_p_"+m_rad_name,
            "Overall NPE vs. Particle Momentum for "+m_rad_name+";p [GeV];#theta [mrad]",
            Binning::momentum_bins, 0, Binning::momentum_max,
            Binning::npe_bins, 0, Binning::npe_max
            );
        m_theta_vs_p = new TH2D(
            "theta_vs_p_"+m_rad_name,
            "Estimated Cherenkov Angle vs. Particle Momentum for "+m_rad_name+";p [GeV];#theta [mrad]",
            Binning::momentum_bins, 0, Binning::momentum_max,
            Binning::theta_bins, 0, Binning::theta_max
            );
        m_highest_weight_vs_p = new TH2D(
            "highest_weight_vs_p_"+m_rad_name,
            "Highest PDG Weight vs. Particle Momentum for "+m_rad_name+";p [GeV];#theta [mrad]",
            Binning::momentum_bins, 0, Binning::momentum_max,
            Binning::pdg_bins(), 0, Binning::pdg_bins()
            );
      }
      ~RadiatorAnalysis() {};
      TString GetRadiatorName() { return m_rad_name; }

      // histograms
      TH1D *m_npe_dist;
      TH1D *m_theta_dist;
      TH2D *m_photon_theta_vs_phi;
      TH1D *m_highest_weight_dist;
      TH2D *m_npe_vs_p;
      TH2D *m_theta_vs_p;
      TH2D *m_highest_weight_vs_p;

    private:
      TString m_rad_name;
  };


  // main algorithm class, radiator-independent analysis
  //---------------------------------------------------------------
  class IrtCherenkovParticleIDAnalysis {

    public:
      IrtCherenkovParticleIDAnalysis() = default;
      ~IrtCherenkovParticleIDAnalysis() {}

      // algorithm methods
      void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
      void AlgorithmProcess(std::vector<const edm4eic::CherenkovParticleID*> cherenkov_pids);
      void AlgorithmFinish();

    private:
      std::shared_ptr<spdlog::logger> m_log;
      std::unordered_map<int,std::shared_ptr<RadiatorAnalysis>> m_radiator_histos; // radiator ID -> RadiatorAnalysis

  };

}
