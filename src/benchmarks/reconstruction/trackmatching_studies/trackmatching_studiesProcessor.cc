#include "trackmatching_studiesProcessor.h"
#include "algorithms/tracking/JugTrack/TrackingResultTrajectory.hpp"
#include "edm4eic/vector_utils.h"
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <services/rootfile/RootFile_service.h>
#include <spdlog/spdlog.h>

#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <services/log/Log_service.h>
#include <spdlog/fmt/ostr.h>

#include "DD4hep/DetElement.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Objects.h"
#include "DDG4/Geant4Data.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/Surface.h"
#include "DDRec/SurfaceManager.h"
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include "TCanvas.h"
#include "TChain.h"
#include "TVector3.h"

#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp"

#include "clusterizer_MA.h"
// #include <extensions/spdlog/SpdlogMixin.h>

// The following just makes this a JANA plugin
extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  app->Add(new trackmatching_studiesProcessor());
}
}

//******************************************************************************************
// InitWithGlobalRootLock
//******************************************************************************************
// void trackmatching_studiesProcessor::InitWithGlobalRootLock() {
void trackmatching_studiesProcessor::Init() {
  std::string plugin_name = ("trackmatching_studies");

  // InitLogger(plugin_name);
  // ===============================================================================================
  // Get JANA application and seup general variables
  // ===============================================================================================
  auto app          = GetApplication();
  auto acts_service = GetApplication()->GetService<ACTSGeo_service>();

  std::string log_level_str = "info";
  m_log                     = app->GetService<Log_service>()->logger(plugin_name);
  app->SetDefaultParameter(plugin_name + ":LogLevel", log_level_str,
                           "LogLevel: trace, debug, info, warn, err, critical, off");
  m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

  // Ask service locator a file to write histograms to
  auto root_file_service = app->GetService<RootFile_service>();
  m_propagation_algo.init(acts_service->actsGeoProvider(), m_log);

  // Get TDirectory for histograms root file
  auto globalRootLock = app->GetService<JGlobalRootLock>();
  globalRootLock->acquire_write_lock();
  auto file = root_file_service->GetHistFile();
  globalRootLock->release_lock();

  std::cout << "trackmatching_studiesProcessor::Init() start" << std::endl;
  // ===============================================================================================
  // Create a directory for this plugin. And subdirectories for series of histograms
  // ===============================================================================================
  m_dir_main = file->mkdir(plugin_name.c_str());

  // ===============================================================================================
  // Simulations hists
  // ===============================================================================================
  hMCEnergyVsEta = new TH2D("hMCEnergyVsEta", "; E (GeV); #eta", 1500, 0., 150., 500, 0, 5);
  hMCEnergyVsEta->SetDirectory(m_dir_main);

  std::cout << "trackmatching_studiesProcessor::Init() done" << std::endl;

  auto transform = Acts::Transform3::Identity();

  // Create propagation surface for LFHCAL
  const auto LFHCAL_Z    = 3596 + 100.; // make it at a certain depth (avg. depth of clusters)
  const auto LFHCAL_MinR = 0.0;
  const auto LFHCAL_MaxR = 3000.0;
  auto LFHCAL_Bounds     = std::make_shared<Acts::RadialBounds>(LFHCAL_MinR, LFHCAL_MaxR);
  auto LFHCAL_Trf        = transform * Acts::Translation3(Acts::Vector3(0, 0, LFHCAL_Z));
  m_LFHCAL_prop_surface  = Acts::Surface::makeShared<Acts::DiscSurface>(LFHCAL_Trf, LFHCAL_Bounds);

  // Create propagation surface for FEMC
  const auto FEMC_Z    = 3296 + 100.; // make it at a certain depth (avg. depth of clusters)
  const auto FEMC_MinR = 0.0;         // 20 cm
  const auto FEMC_MaxR = 2460.;       // 2.46 m
  auto FEMC_Bounds     = std::make_shared<Acts::RadialBounds>(FEMC_MinR, FEMC_MaxR);
  auto FEMC_Trf        = transform * Acts::Translation3(Acts::Vector3(0, 0, FEMC_Z));
  m_FEMC_prop_surface  = Acts::Surface::makeShared<Acts::DiscSurface>(FEMC_Trf, FEMC_Bounds);

  // Create propagation surface for EEMC
  const auto EEMC_Z    = -1840 - 100.; // make it at a certain depth (avg. depth of clusters)
  const auto EEMC_MinR = 0.0;          // 9 cm
  const auto EEMC_MaxR = 641.0;        // 64.1 cm
  auto EEMC_Bounds     = std::make_shared<Acts::RadialBounds>(EEMC_MinR, EEMC_MaxR);
  auto EEMC_Trf        = transform * Acts::Translation3(Acts::Vector3(0, 0, EEMC_Z));
  m_EEMC_prop_surface  = Acts::Surface::makeShared<Acts::DiscSurface>(EEMC_Trf, EEMC_Bounds);

  // Create propagation surface for BEMC
  const auto BEMC_R = 785.0;
  +50;                            // 78.5 cm to 140 cm
  const auto BEMC_halfz = 3300.0; // -330 to 195 cm
  auto BEMC_Trf         = transform * Acts::Translation3(Acts::Vector3(0, 0, 0));
  m_BEMC_prop_surface =
      Acts::Surface::makeShared<Acts::CylinderSurface>(BEMC_Trf, BEMC_R, BEMC_halfz);
}

//******************************************************************************************
// ProcessSequential
//******************************************************************************************
void trackmatching_studiesProcessor::Process(const std::shared_ptr<const JEvent>& event) {
  int verbose = 0;
  using namespace std;
  cout << "trackmatching_studiesProcessor::ProcessSequential() start" << endl;
  // ===============================================================================================
  // process MC particles
  // ===============================================================================================
  auto mcParticles = event->Get<edm4hep::MCParticle>("MCParticles");
  double mceta     = 0;
  double mcphi     = 0;
  double mcp       = 0;
  double mcenergy  = 0;
  int iMC          = 0;
  cout << "MC particles: " << mcParticles.size() << endl;
  for (auto mcparticle : mcParticles) {
    if (mcparticle->getGeneratorStatus() != 1)
      continue;
    auto& mom = mcparticle->getMomentum();
    // get particle energy
    mcenergy = mcparticle->getEnergy();
    // determine mceta from momentum
    mceta = -log(tan(atan2(sqrt(mom.x * mom.x + mom.y * mom.y), mom.z) / 2.));
    // determine mcphi from momentum
    mcphi = atan2(mom.y, mom.x);
    // determine mc momentum
    mcp = sqrt(mom.x * mom.x + mom.y * mom.y + mom.z * mom.z);
    hMCEnergyVsEta->Fill(mcp, mceta);

    iMC++;
  }
  // if (enableTreeCluster) t_mc_N = iMC;
  cout << "MC particles: " << iMC << endl;

  auto acts_results = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories");
  cout << "acts_results: " << acts_results.size() << endl;
  // Loop over the trajectories
  for (const auto& traj : acts_results) {
    // Get the entry index for the single trajectory
    // The trajectory entry indices and the multiTrajectory
    const auto& mj        = traj->multiTrajectory();
    const auto& trackTips = traj->tips();
    if (trackTips.empty()) {
      // m_log->debug("Empty multiTrajectory.");
      continue;
    }

    // projection to LFHCAL surface
    {
      edm4eic::TrackPoint* projection_point_LFHCAL = nullptr;
      try {
        // >>> try to propagate to surface <<<
        projection_point_LFHCAL = m_propagation_algo.propagate(traj, m_LFHCAL_prop_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
      }

      if (!projection_point_LFHCAL) {
        m_log->trace("   could not propagate to LFHCAL!");
        if (verbose)
          cout << "   could not propagate to LFHCAL!" << endl;
      } else {

        auto proj_pos    = projection_point_LFHCAL->position;
        auto proj_length = projection_point_LFHCAL->pathlength;
        auto proj_mom    = projection_point_LFHCAL->momentum;
        TVector3 proj_mom_vec(proj_mom.x, proj_mom.y, proj_mom.z);
        if (verbose)
          cout << "\tLFHCAL_projection pos: " << proj_pos.x << " " << proj_pos.y << " "
               << proj_pos.z << " " << proj_length << endl;

        // loop over LFHCAL clusters
        int iClF         = 0;
        float highestEFr = 0;
        int iClFHigh     = 0;

        auto lfhcalClustersF = event->Get<edm4eic::Cluster>(nameClusters.data());
        for (auto& cluster : lfhcalClustersF) {
          if (cluster->getEnergy() > highestEFr) {
            iClFHigh   = iClF;
            highestEFr = cluster->getEnergy();
          }
          // hRecFClusterEcalib_E_eta->Fill(mcenergy, cluster->getEnergy()/mcenergy, mceta);

          //     std::cout << "Island cluster "<< iClF << ":\t" << cluster->getEnergy()  << "\t"<<
          //     cluster->getNhits()  << std::endl; for (auto& protocluster :
          //     lfhcalProtoClustersF()) { for (auto& hit: cluster->getHits()){
          // //       for (int iCell = 0;  iCell < (int)cluster->getHits().size(); iCell++){
          //       int pSav = 0;
          //       while(hit.getCellID() !=  input_tower_recSav.at(pSav).cellID && pSav <
          //       (int)input_tower_recSav.size() ) pSav++; if (hit.getCellID() ==
          //       input_tower_recSav.at(pSav).cellID)
          //         input_tower_recSav.at(pSav).tower_clusterIDB = iClF;
          //     }
          //     }
          iClF++;
        }
        cout << "LFHCAL clusters: " << iClF << endl;
      }
    }

    // projection to FEMC surface
    {
      edm4eic::TrackPoint* projection_point_FEMC = nullptr;
      try {
        // >>> try to propagate to surface <<<
        projection_point_FEMC = m_propagation_algo.propagate(traj, m_FEMC_prop_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
      }

      if (!projection_point_FEMC) {
        m_log->trace("   could not propagate to FEMC!");
        if (verbose)
          cout << "   could not propagate to FEMC!" << endl;
      } else {

        auto proj_pos    = projection_point_FEMC->position;
        auto proj_length = projection_point_FEMC->pathlength;
        auto proj_mom    = projection_point_FEMC->momentum;
        TVector3 proj_mom_vec(proj_mom.x, proj_mom.y, proj_mom.z);
        if (verbose)
          cout << "\tFEMC_projection pos: " << proj_pos.x << " " << proj_pos.y << " " << proj_pos.z
               << " " << proj_length << endl;

        // loop over FEMC clusters
        int iECl           = 0;
        float highestEEmCl = 0;
        int iEClHigh       = 0;

        auto FEMC_Clusters = event->Get<edm4eic::Cluster>("EcalEndcapPClusters");
        //   cout << "FEMC_Clusters: " << FEMC_Clusters.size() << endl;
        for (auto& cluster : FEMC_Clusters) {
          // if (iECl < maxNCluster && enableTreeCluster){
          //     t_fECal_cluster_E[iECl]       = (float)cluster->getEnergy();
          //     t_fECal_cluster_NCells[iECl]  = (int)cluster->getNhits();
          //     t_fECal_cluster_Eta[iECl]     = (-1.) *
          //     std::log(std::atan((float)cluster->getIntrinsicTheta() / 2.));
          //     t_fECal_cluster_Phi[iECl] = (float)cluster->getIntrinsicPhi();
          // }

          if (cluster->getEnergy() > highestEEmCl) {
            iEClHigh     = iECl;
            highestEEmCl = cluster->getEnergy();
          }
          // hRecFEmClusterEcalib_E_eta->Fill(mcenergy, cluster->getEnergy()/mcenergy, mceta);
          iECl++;
        }
        // hRecFEmNClusters_E_eta->Fill(mcenergy, iECl, mceta);
        cout << "FEMC clusters: " << iECl << endl;
      }
    }
    // projection to EEMC surface
    {
      edm4eic::TrackPoint* projection_point_EEMC = nullptr;
      try {
        // >>> try to propagate to surface <<<
        projection_point_EEMC = m_propagation_algo.propagate(traj, m_EEMC_prop_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
      }

      if (!projection_point_EEMC) {
        m_log->trace("   could not propagate to EEMC!");
        if (verbose)
          cout << "   could not propagate to EEMC!" << endl;
      } else {

        auto proj_pos    = projection_point_EEMC->position;
        auto proj_length = projection_point_EEMC->pathlength;
        auto proj_mom    = projection_point_EEMC->momentum;
        TVector3 proj_mom_vec(proj_mom.x, proj_mom.y, proj_mom.z);
        if (verbose)
          cout << "\tEEMC_projection pos: " << proj_pos.x << " " << proj_pos.y << " " << proj_pos.z
               << " " << proj_length << endl;

        // loop over EEMC clusters
        int iECl           = 0;
        float highestEEmCl = 0;
        int iEClHigh       = 0;

        auto EEMC_Clusters = event->Get<edm4eic::Cluster>("EcalEndcapNClusters");
        //   cout << "EEMC_Clusters: " << EEMC_Clusters.size() << endl;
        for (auto& cluster : EEMC_Clusters) {
          // if (iECl < maxNCluster && enableTreeCluster){
          //     t_fECal_cluster_E[iECl]       = (float)cluster->getEnergy();
          //     t_fECal_cluster_NCells[iECl]  = (int)cluster->getNhits();
          //     t_fECal_cluster_Eta[iECl]     = (-1.) *
          //     std::log(std::atan((float)cluster->getIntrinsicTheta() / 2.));
          //     t_fECal_cluster_Phi[iECl] = (float)cluster->getIntrinsicPhi();
          // }

          if (cluster->getEnergy() > highestEEmCl) {
            iEClHigh     = iECl;
            highestEEmCl = cluster->getEnergy();
          }
          // hRecEEMClusterEcalib_E_eta->Fill(mcenergy, cluster->getEnergy()/mcenergy, mceta);
          iECl++;
        }
        // hRecFEmNClusters_E_eta->Fill(mcenergy, iECl, mceta);
        cout << "EEMC clusters: " << iECl << endl;
      }
    }

    // projection to BEMC surface
    {
      edm4eic::TrackPoint* projection_point_BEMC = nullptr;
      try {
        // >>> try to propagate to surface <<<
        projection_point_BEMC = m_propagation_algo.propagate(traj, m_BEMC_prop_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
      }

      if (!projection_point_BEMC) {
        m_log->trace("   could not propagate to BEMC!");
        if (verbose)
          cout << "   could not propagate to BEMC!" << endl;
      } else {

        auto proj_pos    = projection_point_BEMC->position;
        auto proj_length = projection_point_BEMC->pathlength;
        auto proj_mom    = projection_point_BEMC->momentum;
        TVector3 proj_mom_vec(proj_mom.x, proj_mom.y, proj_mom.z);
        if (verbose)
          cout << "\tBEMC_projection pos: " << proj_pos.x << " " << proj_pos.y << " " << proj_pos.z
               << " " << proj_length << endl;

        // loop over BEMC clusters
        int iECl           = 0;
        float highestBEMCl = 0;
        int iEClHigh       = 0;

        auto BEMC_Clusters = event->Get<edm4eic::Cluster>("EcalBarrelSciGlassClusters");
        //   cout << "BEMC_Clusters: " << BEMC_Clusters.size() << endl;
        for (auto& cluster : BEMC_Clusters) {
          // if (iECl < maxNCluster && enableTreeCluster){
          //     t_fECal_cluster_E[iECl]       = (float)cluster->getEnergy();
          //     t_fECal_cluster_NCells[iECl]  = (int)cluster->getNhits();
          //     t_fECal_cluster_Eta[iECl]     = (-1.) *
          //     std::log(std::atan((float)cluster->getIntrinsicTheta() / 2.));
          //     t_fECal_cluster_Phi[iECl] = (float)cluster->getIntrinsicPhi();
          // }

          if (cluster->getEnergy() > highestBEMCl) {
            iEClHigh     = iECl;
            highestBEMCl = cluster->getEnergy();
          }
          // hRecBEMClusterEcalib_E_eta->Fill(mcenergy, cluster->getEnergy()/mcenergy, mceta);
          iECl++;
        }
        // hRecFEmNClusters_E_eta->Fill(mcenergy, iECl, mceta);
        cout << "BEMC clusters: " << iECl << endl;
      }
    }
  }
}

//******************************************************************************************
// Finish
//******************************************************************************************
void trackmatching_studiesProcessor::Finish() {
  std::cout << "------> " << nEventsWithCaloHits << " with calo info present" << std::endl;
  // Do any final calculations here.
}
