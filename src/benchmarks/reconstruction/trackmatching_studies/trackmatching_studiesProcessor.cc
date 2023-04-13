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

  hFEMC_dEta_dPhi =
      new TH2D("hFEMC_dEta_dPhi", "; #Delta#eta; #Delta#phi", 200, -0.2, 0.2, 200, -0.2, 0.2);
  hEEMC_dEta_dPhi =
      new TH2D("hEEMC_dEta_dPhi", "; #Delta#eta; #Delta#phi", 200, -0.2, 0.2, 200, -0.2, 0.2);
  hEHCAL_dEta_dPhi =
      new TH2D("hEHCAL_dEta_dPhi", "; #Delta#eta; #Delta#phi", 200, -0.2, 0.2, 200, -0.2, 0.2);
  hLFHCAL_dEta_dPhi =
      new TH2D("hLFHCAL_dEta_dPhi", "; #Delta#eta; #Delta#phi", 200, -0.2, 0.2, 200, -0.2, 0.2);
  hBEMC_dEta_dPhi =
      new TH2D("hBEMC_dEta_dPhi", "; #Delta#eta; #Delta#phi", 200, -0.2, 0.2, 200, -0.2, 0.2);
  hOHCAL_dEta_dPhi =
      new TH2D("hOHCAL_dEta_dPhi", "; #Delta#eta; #Delta#phi", 200, -0.2, 0.2, 200, -0.2, 0.2);
  hFEMC_dEta_dPhi->SetDirectory(m_dir_main);
  hEEMC_dEta_dPhi->SetDirectory(m_dir_main);
  hEHCAL_dEta_dPhi->SetDirectory(m_dir_main);
  hLFHCAL_dEta_dPhi->SetDirectory(m_dir_main);
  hBEMC_dEta_dPhi->SetDirectory(m_dir_main);
  hOHCAL_dEta_dPhi->SetDirectory(m_dir_main);
  
  // possibly vs phi as well 0.01 binning for barrel
  hECalibEtaE_FEMC_matched =
      new TH3D("hECalibEtaE_FEMC_matched", "; E_{rec}/E_{true} ; #eta", 100, 0., 2., 32, -4.0, 4.0, 300, 0., 30.);
  hECalibEtaE_EEMC_matched =
      new TH3D("hECalibEtaE_EEMC_matched", "; E_{rec}/E_{true} ; #eta", 100, 0., 2., 32, -4.0, 4.0, 300, 0., 30.);
  hECalibEtaE_BEMC_matched =
      new TH3D("hECalibEtaE_BEMC_matched", "; E_{rec}/E_{true} ; #eta", 100, 0., 2., 32, -4.0, 4.0, 300, 0., 30.);
  hECalibEtaE_LFHCAL_matched =
      new TH3D("hECalibEtaE_LFHCAL_matched", "; E_{rec}/E_{true} ; #eta", 100, 0., 2., 32, -4.0, 4.0, 300, 0., 30.);
  hECalibEtaE_OHCAL_matched =
      new TH3D("hECalibEtaE_OHCAL_matched", "; E_{rec}/E_{true} ; #eta", 100, 0., 2., 32, -4.0, 4.0, 300, 0., 30.);
  hECalibEtaE_EHCAL_matched =
      new TH3D("hECalibEtaE_EHCAL_matched", "; E_{rec}/E_{true} ; #eta", 100, 0., 2., 32, -4.0, 4.0, 300, 0., 30.);
  hECalibEtaE_EHCAL_matched->SetDirectory(m_dir_main);
  hECalibEtaE_FEMC_matched->SetDirectory(m_dir_main);
  hECalibEtaE_EEMC_matched->SetDirectory(m_dir_main);
  hECalibEtaE_BEMC_matched->SetDirectory(m_dir_main);
  hECalibEtaE_LFHCAL_matched->SetDirectory(m_dir_main);
  hECalibEtaE_OHCAL_matched->SetDirectory(m_dir_main);

  hECalibEtaE_forward_matched =
      new TH3D("hECalibEtaE_forward_matched", "; E_{rec}/E_{true} ; #eta", 100, 0., 2., 32, -4.0, 4.0, 300, 0., 30.);
  hECalibEtaE_barrel_matched =
      new TH3D("hECalibEtaE_barrel_matched", "; E_{rec}/E_{true} ; #eta", 100, 0., 2., 32, -4.0, 4.0, 300, 0., 30.);
  hECalibEtaE_backward_matched =
      new TH3D("hECalibEtaE_backward_matched", "; E_{rec}/E_{true} ; #eta", 100, 0., 2., 32, -4.0, 4.0, 300, 0., 30.);
  hECalibEtaE_forward_matched->SetDirectory(m_dir_main);
  hECalibEtaE_barrel_matched->SetDirectory(m_dir_main);
  hECalibEtaE_backward_matched->SetDirectory(m_dir_main);

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
  const auto BEMC_R     = 785.0 + 50; // 78.5 cm to 140 cm
  const auto BEMC_halfz = 3300.0;     // -330 to 195 cm
  auto BEMC_Trf         = transform * Acts::Translation3(Acts::Vector3(0, 0, 0));
  m_BEMC_prop_surface =
      Acts::Surface::makeShared<Acts::CylinderSurface>(BEMC_Trf, BEMC_R, BEMC_halfz);

  // Create propagation surface for OHCAL
  const auto OHCAL_R     = 1770.0 + 300; // 177 or 190 to 270 cm
  const auto OHCAL_halfz = 6390.0;       //
  auto OHCAL_Trf         = transform * Acts::Translation3(Acts::Vector3(0, 0, 0));
  m_OHCAL_prop_surface =
      Acts::Surface::makeShared<Acts::CylinderSurface>(OHCAL_Trf, OHCAL_R, OHCAL_halfz);

  // Create propagation surface for EHCAL
  const auto EHCAL_Z    = -3300;   //
  const auto EHCAL_MinR = 0.0;     // 19.9431 cm
  const auto EHCAL_MaxR = 2514.44; // 251.444 cm
  auto EHCAL_Bounds     = std::make_shared<Acts::RadialBounds>(EHCAL_MinR, EHCAL_MaxR);
  auto EHCAL_Trf        = transform * Acts::Translation3(Acts::Vector3(0, 0, EHCAL_Z));
  m_EHCAL_prop_surface  = Acts::Surface::makeShared<Acts::DiscSurface>(EHCAL_Trf, EHCAL_Bounds);
}

//******************************************************************************************
// ProcessSequential
//******************************************************************************************
void trackmatching_studiesProcessor::Process(const std::shared_ptr<const JEvent>& event) {
  using namespace std;
  cout << "trackmatching_studiesProcessor::ProcessSequential() start" << endl;
  // ===============================================================================================
  // process MC particles
  // ===============================================================================================
  auto mcParticles = event->Get<edm4hep::MCParticle>("MCParticles");
  double mceta, mcphi, mcp, mcenergy = 0;
  int iMC = 0;
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
  } // end loop over MC particles
  cout << "MC particles (primary): " << iMC << endl;

  auto acts_results = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories");
  // Loop over the trajectories
  for (const auto& traj : acts_results) {
    // Get the entry index for the single trajectory
    // The trajectory entry indices and the multiTrajectory
    const auto& mj        = traj->multiTrajectory();
    const auto& trackTips = traj->tips();
    if (trackTips.empty()) {
      m_log->debug("Empty multiTrajectory.");
      continue;
    }

    // projection to LFHCAL surface
    float highMatchedE_LFHCAL = 0;
    if (mceta > 1) { // acceptance requirement
      edm4eic::TrackPoint* projection_point_LFHCAL = nullptr;
      try {
        // >>> try to propagate to surface <<<
        projection_point_LFHCAL = m_propagation_algo.propagate(traj, m_LFHCAL_prop_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
      }

      if (!projection_point_LFHCAL) {
        m_log->trace("   could not propagate to LFHCAL!");
      } else {

        auto proj_pos    = projection_point_LFHCAL->position;
        auto proj_length = projection_point_LFHCAL->pathlength;
        auto proj_mom    = projection_point_LFHCAL->momentum;
        TVector3 proj_pos_vec(proj_pos.x, proj_pos.y, proj_pos.z);

        // loop over LFHCAL clusters
        int iClF = 0;

        auto lfhcalClustersF = event->Get<edm4eic::Cluster>(nameClusters.data());
        for (auto& cluster : lfhcalClustersF) {
          float clus_eta = (-1.) * std::log(std::tan((float)cluster->getIntrinsicTheta() / 2.));
          float clus_phi = (float)cluster->getIntrinsicPhi();
          float dEta     = proj_pos_vec.Eta() - clus_eta;
          float dPhi     = proj_pos_vec.Phi() - clus_phi;
          hLFHCAL_dEta_dPhi->Fill(dEta, dPhi);

          if (dEta < 0.1 && dPhi < 0.1) {
            hECalibEtaE_LFHCAL_matched->Fill(cluster->getEnergy() / mcenergy, mceta, mcenergy);
            if (cluster->getEnergy() > highMatchedE_LFHCAL) {
              highMatchedE_LFHCAL = cluster->getEnergy();
            }
          }

          iClF++;
        }
        cout << "LFHCAL clusters: " << iClF << endl;
      } // end projection to LFHCAL surface
    }   // end LFHCAL acceptance requirement

    // projection to FEMC surface
    float highMatchedE_FEMC = 0;
    if (mceta > 1) { // acceptance requirement
      edm4eic::TrackPoint* projection_point_FEMC = nullptr;
      try {
        // >>> try to propagate to surface <<<
        projection_point_FEMC = m_propagation_algo.propagate(traj, m_FEMC_prop_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
      }

      if (!projection_point_FEMC) {
        m_log->trace("   could not propagate to FEMC!");
      } else {

        auto proj_pos    = projection_point_FEMC->position;
        auto proj_length = projection_point_FEMC->pathlength;
        auto proj_mom    = projection_point_FEMC->momentum;
        TVector3 proj_pos_vec(proj_pos.x, proj_pos.y, proj_pos.z);

        // loop over FEMC clusters
        int iECl = 0;

        auto FEMC_Clusters = event->Get<edm4eic::Cluster>("EcalEndcapPClusters");
        for (auto& cluster : FEMC_Clusters) {
          float clus_eta = (-1.) * std::log(std::tan((float)cluster->getIntrinsicTheta() / 2.));
          float clus_phi = (float)cluster->getIntrinsicPhi();
          float dEta     = proj_pos_vec.Eta() - clus_eta;
          float dPhi     = proj_pos_vec.Phi() - clus_phi;
          hFEMC_dEta_dPhi->Fill(dEta, dPhi);

          if (dEta < 0.1 && dPhi < 0.1) {
            hECalibEtaE_FEMC_matched->Fill(cluster->getEnergy() / mcenergy, mceta, mcenergy);
            if (cluster->getEnergy() > highMatchedE_FEMC) {
              highMatchedE_FEMC = cluster->getEnergy();
            }
          }
          iECl++;
        }
        cout << "FEMC clusters: " << iECl << endl;
      } // end FEMC projection
    }   // end FEMC acceptance requirement

    // projection to EEMC surface
    float highMatchedE_EEMC = 0;
    if (mceta < 1) { // acceptance requirement
      edm4eic::TrackPoint* projection_point_EEMC = nullptr;
      try {
        // >>> try to propagate to surface <<<
        projection_point_EEMC = m_propagation_algo.propagate(traj, m_EEMC_prop_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
      }

      if (!projection_point_EEMC) {
        m_log->trace("   could not propagate to EEMC!");
      } else {

        auto proj_pos    = projection_point_EEMC->position;
        auto proj_length = projection_point_EEMC->pathlength;
        auto proj_mom    = projection_point_EEMC->momentum;
        TVector3 proj_pos_vec(proj_pos.x, proj_pos.y, proj_pos.z);

        int iECl = 0;

        auto EEMC_Clusters = event->Get<edm4eic::Cluster>("EcalEndcapNClusters");
        for (auto& cluster : EEMC_Clusters) {
          float clus_eta = (-1.) * std::log(std::tan((float)cluster->getIntrinsicTheta() / 2.));
          float clus_phi = (float)cluster->getIntrinsicPhi();
          float dEta     = proj_pos_vec.Eta() - clus_eta;
          float dPhi     = proj_pos_vec.Phi() - clus_phi;
          hEEMC_dEta_dPhi->Fill(dEta, dPhi);

          if (dEta < 0.1 && dPhi < 0.1) {
            hECalibEtaE_EEMC_matched->Fill(cluster->getEnergy() / mcenergy, mceta, mcenergy);
            if (cluster->getEnergy() > highMatchedE_EEMC) {
              highMatchedE_EEMC = cluster->getEnergy();
            }
          }
          iECl++;
        }
        cout << "EEMC clusters: " << iECl << endl;
      } // end of EEMC projection
    }   // end of EEMC acceptance requirement

    // projection to BEMC surface
    float highMatchedE_BEMC = 0;
    if (abs(mceta) < 1.5) { // acceptance requirement
      edm4eic::TrackPoint* projection_point_BEMC = nullptr;
      try {
        // >>> try to propagate to surface <<<
        projection_point_BEMC = m_propagation_algo.propagate(traj, m_BEMC_prop_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
      }

      if (!projection_point_BEMC) {
        m_log->trace("   could not propagate to BEMC!");
      } else {

        auto proj_pos    = projection_point_BEMC->position;
        auto proj_length = projection_point_BEMC->pathlength;
        auto proj_mom    = projection_point_BEMC->momentum;
        TVector3 proj_pos_vec(proj_pos.x, proj_pos.y, proj_pos.z);

        // loop over BEMC clusters
        int iECl = 0;

        auto BEMC_Clusters = event->Get<edm4eic::Cluster>("EcalBarrelSciGlassClusters");
        for (auto& cluster : BEMC_Clusters) {
          float clus_eta = (-1.) * std::log(std::tan((float)cluster->getIntrinsicTheta() / 2.));
          float clus_phi = (float)cluster->getIntrinsicPhi();
          float dEta     = proj_pos_vec.Eta() - clus_eta;
          float dPhi     = proj_pos_vec.Phi() - clus_phi;
          hBEMC_dEta_dPhi->Fill(dEta, dPhi);

          if (dEta < 0.1 && dPhi < 0.1) {
            hECalibEtaE_BEMC_matched->Fill(cluster->getEnergy() / mcenergy, mceta, mcenergy);
            if (cluster->getEnergy() > highMatchedE_BEMC) {
              highMatchedE_BEMC = cluster->getEnergy();
            }
          }
          iECl++;
        }
        cout << "BEMC clusters: " << iECl << endl;
      } // end of BEMC projection
    }   // end of BEMC acceptance requirement

    // projection to OHCAL surface
    float highMatchedE_OHCAL = 0;
    if (abs(mceta) < 1.5) { // acceptance requirement
      edm4eic::TrackPoint* projection_point_OHCAL = nullptr;
      try {
        // >>> try to propagate to surface <<<
        projection_point_OHCAL = m_propagation_algo.propagate(traj, m_OHCAL_prop_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
      }

      if (!projection_point_OHCAL) {
        m_log->trace("   could not propagate to OHCAL!");
      } else {

        auto proj_pos    = projection_point_OHCAL->position;
        auto proj_length = projection_point_OHCAL->pathlength;
        auto proj_mom    = projection_point_OHCAL->momentum;
        TVector3 proj_pos_vec(proj_pos.x, proj_pos.y, proj_pos.z);

        // loop over OHCAL clusters
        int iECl            = 0;
        auto OHCAL_Clusters = event->Get<edm4eic::Cluster>("HcalBarrelClusters");
        for (auto& cluster : OHCAL_Clusters) {
          float clus_eta = (-1.) * std::log(std::tan((float)cluster->getIntrinsicTheta() / 2.));
          float clus_phi = (float)cluster->getIntrinsicPhi();
          float dEta     = proj_pos_vec.Eta() - clus_eta;
          float dPhi     = proj_pos_vec.Phi() - clus_phi;
          hOHCAL_dEta_dPhi->Fill(dEta, dPhi);

          if (dEta < 0.1 && dPhi < 0.1) {
            hECalibEtaE_OHCAL_matched->Fill(cluster->getEnergy() / mcenergy, mceta, mcenergy);
            if (cluster->getEnergy() > highMatchedE_OHCAL) {
              highMatchedE_OHCAL = cluster->getEnergy();
            }
          }
          iECl++;
        }
        cout << "OHCAL clusters: " << iECl << endl;
      } // end of OHCAL projection
    }   // end of OHCAL acceptance requirement

    float highMatchedE_EHCAL = 0;
    // projection to EHCAL surface
    if (mceta < -1.) { // acceptance requirement
      edm4eic::TrackPoint* projection_point_EHCAL = nullptr;
      try {
        // >>> try to propagate to surface <<<
        projection_point_EHCAL = m_propagation_algo.propagate(traj, m_EHCAL_prop_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
      }

      if (!projection_point_EHCAL) {
        m_log->trace("   could not propagate to EHCAL!");
      } else {

        auto proj_pos    = projection_point_EHCAL->position;
        auto proj_length = projection_point_EHCAL->pathlength;
        auto proj_mom    = projection_point_EHCAL->momentum;
        TVector3 proj_pos_vec(proj_pos.x, proj_pos.y, proj_pos.z);

        // loop over EHCAL clusters
        int iECl = 0;

        auto EHCAL_Clusters = event->Get<edm4eic::Cluster>("EcalEndcapNClusters");
        for (auto& cluster : EHCAL_Clusters) {
          float clus_eta = (-1.) * std::log(std::tan((float)cluster->getIntrinsicTheta() / 2.));
          float clus_phi = (float)cluster->getIntrinsicPhi();
          float dEta     = proj_pos_vec.Eta() - clus_eta;
          float dPhi     = proj_pos_vec.Phi() - clus_phi;
          hEHCAL_dEta_dPhi->Fill(dEta, dPhi);

          if (dEta < 0.1 && dPhi < 0.1) {
            hECalibEtaE_EHCAL_matched->Fill(cluster->getEnergy() / mcenergy, mceta, mcenergy);
            if (cluster->getEnergy() > highMatchedE_EHCAL) {
              highMatchedE_EHCAL = cluster->getEnergy();
            }
          }
          iECl++;
        } // end loop over EHCAL clusters
        cout << "EHCAL clusters: " << iECl << endl;
      } // end if projection to EHCAL surface
    }   // end projection to EHCAL surface

    if (highMatchedE_OHCAL > 0 && highMatchedE_BEMC > 0) {
      hECalibEtaE_barrel_matched->Fill((highMatchedE_OHCAL + highMatchedE_BEMC) / mcenergy, mceta, mcenergy);
    }
    if (highMatchedE_EEMC > 0 && highMatchedE_EHCAL > 0) {
      hECalibEtaE_backward_matched->Fill((highMatchedE_EEMC + highMatchedE_EHCAL) / mcenergy, mceta, mcenergy);
    }
    if (highMatchedE_FEMC > 0 && highMatchedE_LFHCAL > 0) {
      hECalibEtaE_forward_matched->Fill((highMatchedE_FEMC + highMatchedE_LFHCAL) / mcenergy, mceta, mcenergy);
    }
  } // end loop over tracks
} // end process

//******************************************************************************************
// Finish
//******************************************************************************************
void trackmatching_studiesProcessor::Finish() {
  std::cout << "------> " << nEventsWithCaloHits << " with calo info present" << std::endl;
  // Do any final calculations here.
}
