// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// ROOT
#include <TH1D.h>
#include <TH2D.h>
#include <TMath.h>

// data model
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/utils/vector_utils.h>
#include <edm4hep/utils/kinematics.h>

// EICrecon
#include <spdlog/spdlog.h>
#include "Tools.h"

namespace eicrecon {

  // analysis for one radiator
  //---------------------------------------------------------------
  class RadiatorAnalysis {
    public:
      RadiatorAnalysis(TString rad_name);
      ~RadiatorAnalysis() {};
      TString GetRadiatorName() { return m_rad_name; }

      // histograms
      // - distributions
      TH1D *m_npe_dist;
      TH1D *m_theta_dist;
      TH1D *m_thetaResid_dist;
      TH1D *m_mcWavelength_dist;
      TH1D *m_mcRindex_dist;
      TH1D *m_highestWeight_dist;
      TH2D *m_photonTheta_vs_photonPhi;
      // - momentum scans
      TH2D *m_npe_vs_p;
      TH2D *m_theta_vs_p;
      TH2D *m_thetaResid_vs_p;
      TH2D *m_highestWeight_vs_p;

      // binning
      static constexpr int    n_bins         = 100;
      static constexpr int    momentum_bins  = 500;
      static constexpr int    momentum_max   = 70;
      static constexpr int    npe_bins       = 100;
      static constexpr double npe_max        = 100;
      static constexpr int    nphot_max      = 400;
      static constexpr int    theta_bins     = 1500;
      static constexpr double theta_max      = 300;
      static constexpr double thetaResid_max = 100;
      static constexpr int    phi_bins       = 100;
      static int pdg_bins() { return Tools::GetNumPDGs() + 1; }

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
      void AlgorithmProcess(
          std::vector<const edm4hep::MCParticle*>          mc_parts,
          std::vector<const edm4hep::SimTrackerHit*>       sim_hits,
          std::vector<const edm4eic::CherenkovParticleID*> cherenkov_pids
          );
      void AlgorithmFinish();

    private:

      // histograms (both radiators combined)
      TH2D *m_nphot_vs_p;
      TH1D *m_nphot_vs_p__transient; // transient (not written)

      // map: radiator ID -> RadiatorAnalysis object
      std::unordered_map<decltype(edm4eic::CherenkovParticleIDData::radiator),std::shared_ptr<RadiatorAnalysis>> m_radiator_histos;

      // additional objects
      std::shared_ptr<spdlog::logger> m_log;

  };

}
