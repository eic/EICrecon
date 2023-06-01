// Copyright 2023, Friederike Bock
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//  Sections Copyright (C) 2023 Friederike Bock
//  under SPDX-License-Identifier: LGPL-3.0-or-later

#include "femc_studiesProcessor.h"
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

#include "benchmarks/reconstruction/lfhcal_studies/clusterizer_MA.h"
// #include <extensions/spdlog/SpdlogMixin.h>

// The following just makes this a JANA plugin
extern "C" {
  void InitPlugin(JApplication* app) {
    InitJANAPlugin(app);
    app->Add(new femc_studiesProcessor());
  }
}


//******************************************************************************************
// InitWithGlobalRootLock
//******************************************************************************************
void femc_studiesProcessor::Init() {
  std::string plugin_name = ("femc_studies");

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

  // Get TDirectory for histograms root file
  auto globalRootLock = app->GetService<JGlobalRootLock>();
  globalRootLock->acquire_write_lock();
  auto file = root_file_service->GetHistFile();
  globalRootLock->release_lock();

  // ===============================================================================================
  // Create a directory for this plugin. And subdirectories for series of histograms
  // ===============================================================================================
  m_dir_main = file->mkdir(plugin_name.c_str());

  // ===============================================================================================
  // Simulations hists
  // ===============================================================================================
  hMCEnergyVsEta        = new TH2D("hMCEnergyVsEta", "; E (GeV); #eta", 1500, 0., 150., 500, 0, 5);
  hMCEnergyVsEta->SetDirectory(m_dir_main);

  // ===============================================================================================
  // Sum cell clusters rec histos
  // ===============================================================================================
  hClusterEcalib_E_eta  = new TH3D("hClusterEcalib_E_eta", "; E_{MC} (GeV); E_{rec,rec hit}/E_{MC}; #eta",
                                    1500, 0., 150.0, 200, 0., 2.0, 50, 0, 5);
  hClusterNCells_E_eta  = new TH3D("hClusterNCells_E_eta", "; E_{MC} (GeV); N_{cells}; #eta",
                                    1500, 0., 150.0, 500, -0.5, 499.5, 50, 0, 5);
  hClusterEcalib_E_phi  = new TH3D("hClusterEcalib_E_phi", "; E_{MC} (GeV); E_{rec,rec hit}/E_{MC}; #varphi (rad)",
                                    1500, 0., 150.0, 200, 0., 2.0, 360 , -TMath::Pi(), TMath::Pi());
  hPosCaloHitsXY        = new TH2D("hPosCaloHitsXY", "; X (cm); Y (cm)", 400, -400., 400., 400, -400., 400.);
  hClusterEcalib_E_eta->SetDirectory(m_dir_main);
  hClusterNCells_E_eta->SetDirectory(m_dir_main);
  hClusterEcalib_E_phi->SetDirectory(m_dir_main);
  hPosCaloHitsXY->SetDirectory(m_dir_main);

  // ===============================================================================================
  // Sum cell clusters sim histos
  // ===============================================================================================
  hClusterESimcalib_E_eta = new TH3D("hClusterESimcalib_E_eta", "; E_{MC} (GeV); E_{rec,sim hit}/E_{MC}; #eta" ,
                                      1500, 0., 150.0, 200, 0., 2.0, 50, 0, 5);
  hClusterSimNCells_E_eta = new TH3D("hClusterSimNCells_E_eta", "; E_{MC} (GeV); N_{cells, sim}; #eta",
                                      1500, 0., 150.0, 500, -0.5, 499.5, 50, 0, 5);
  hClusterESimcalib_E_phi = new TH3D("hClusterESimcalib_E_phi", "; E_{MC} (GeV); E_{rec,sim hit}/E_{MC}; #varphi (rad)" ,
                                      1500, 0., 150.0, 200, 0., 2.0, 360 , -TMath::Pi(), TMath::Pi());
  hCellESim_layerX        = new TH2D("hCellESim_layerX", "; #cell ID X; E_{rec,sim hit} (GeV)" , 500, -0.5, 499.5, 5000, 0, 1);
  hCellESim_layerY        = new TH2D("hCellESim_layerY", "; #cell ID Y; E_{rec,sim hit} (GeV)" , 500, -0.5, 499.5, 5000, 0, 1);
  hCellTSim_layerX        = new TH2D("hCellTSim_layerX", "; #cell ID X; t_{rec,sim hit} (GeV)" , 500, -0.5, 499.5, 5000, 0, 1000);
  hPosCaloSimHitsXY       = new TH2D("hPosCaloSimHitsXY", "; X (cm); Y (cm)", 400, -400., 400., 400, -400., 400.);
  hClusterESimcalib_E_eta->SetDirectory(m_dir_main);
  hClusterSimNCells_E_eta->SetDirectory(m_dir_main);
  hClusterESimcalib_E_phi->SetDirectory(m_dir_main);
  hCellESim_layerX->SetDirectory(m_dir_main);
  hCellESim_layerY->SetDirectory(m_dir_main);
  hCellTSim_layerX->SetDirectory(m_dir_main);
  hPosCaloSimHitsXY->SetDirectory(m_dir_main);

  // ===============================================================================================
  // rec cluster MA clusters histos
  // ===============================================================================================
  hRecClusterEcalib_E_eta     = new TH3D("hRecClusterEcalib_E_eta", "; E_{MC} (GeV); E_{rec,rec clus}/E_{MC}; #eta",
                                          1500, 0., 150.0, 200, 0., 2.0, 50, 0, 5);
  hRecNClusters_E_eta         = new TH3D("hRecNClusters_E_eta", "; E_{MC} (GeV); N_{rec cl.}; #eta",
                                          1500, 0., 150.0, 10, -0.5, 9.5, 50, 0, 5);
  // rec cluster highest
  hRecClusterEcalib_Ehigh_eta = new TH3D("hRecClusterEcalib_Ehigh_eta", "; E_{MC} (GeV); E_{rec,rec clus high.}/E_{MC}; #eta",
                                          1500, 0., 150.0, 200, 0., 2.0, 50, 0, 5);
  hRecClusterNCells_Ehigh_eta = new TH3D("hRecClusterNCells_Ehigh_eta", "; E_{MC} (GeV); N_{cells, rec cl., high.}; #eta",
                                          1500, 0., 150.0, 500, -0.5, 499.5, 50, 0, 5);
  hRecClusterEcalib_E_eta->SetDirectory(m_dir_main);
  hRecNClusters_E_eta->SetDirectory(m_dir_main);
  hRecClusterEcalib_Ehigh_eta->SetDirectory(m_dir_main);
  hRecClusterNCells_Ehigh_eta->SetDirectory(m_dir_main);

  // ===============================================================================================
  // rec cluster framework Island clusters histos
  // ===============================================================================================
  hRecFClusterEcalib_E_eta      = new TH3D("hRecFClusterEcalib_E_eta", "; E_{MC} (GeV); E_{rec,fram clus}/E_{MC}; #eta",
                                            1500, 0., 150.0, 200, 0., 2.0, 50, 0, 5);
  hRecFNClusters_E_eta          = new TH3D("hRecFNClusters_E_eta", "; E_{MC} (GeV); N_{rec f. cl.}; #eta",
                                            1500, 0., 150.0, 10, -0.5, 9.5, 50, 0, 5);
  // rec cluster framework highest
  hRecFClusterEcalib_Ehigh_eta  = new TH3D("hRecFClusterEcalib_Ehigh_eta", "; E_{MC} (GeV); E_{rec,fram clus high.}/E_{MC}; #eta",
                                            1500, 0., 150.0, 200, 0., 2.0, 50, 0, 5);
  hRecFClusterNCells_Ehigh_eta  = new TH3D("hRecFClusterNCells_Ehigh_eta", "; E_{MC} (GeV); N_{cells, rec f. cl., high.}; #eta",
                                            1500, 0., 150.0, 500, -0.5, 499.5, 50, 0, 5);
  hRecFClusterEcalib_E_eta->SetDirectory(m_dir_main);
  hRecFNClusters_E_eta->SetDirectory(m_dir_main);
  hRecFClusterEcalib_Ehigh_eta->SetDirectory(m_dir_main);
  hRecFClusterNCells_Ehigh_eta->SetDirectory(m_dir_main);

  // ===============================================================================================
  // Sampling fraction
  // ===============================================================================================
  hSamplingFractionEta    = new TH2D("hSamplingFractionEta", "; #eta; f", 400, 1., 5., 500, 0., 0.2);
  hSamplingFractionEta->SetDirectory(m_dir_main);

  // ===============================================================================================
  // Tree for clusterizer studies
  // ===============================================================================================
  if (enableTree){
    event_tree                = new TTree("event_tree", "event_tree");
    event_tree->SetDirectory(m_dir_main);

    t_fEMC_towers_cellE       = new float[maxNTowers];
    t_fEMC_towers_cellT       = new float[maxNTowers];
    t_fEMC_towers_cellIDx     = new short[maxNTowers];
    t_fEMC_towers_cellIDy     = new short[maxNTowers];
    t_fEMC_towers_clusterIDA  = new short[maxNTowers];
    t_fEMC_towers_clusterIDB  = new short[maxNTowers];
    t_fEMC_towers_cellTrueID  = new int[maxNTowers];

    // towers FEMC
    event_tree->Branch("tower_FEMC_N", &t_fEMC_towers_N, "tower_FEMC_N/I");
    event_tree->Branch("tower_FEMC_E", t_fEMC_towers_cellE, "tower_FEMC_E[tower_FEMC_N]/F");
    event_tree->Branch("tower_FEMC_T", t_fEMC_towers_cellT, "tower_FEMC_T[tower_FEMC_N]/F");
    event_tree->Branch("tower_FEMC_ix", t_fEMC_towers_cellIDx, "tower_FEMC_ix[tower_FEMC_N]/S");
    event_tree->Branch("tower_FEMC_iy", t_fEMC_towers_cellIDy, "tower_FEMC_iy[tower_FEMC_N]/S");
    event_tree->Branch("tower_FEMC_clusIDA", t_fEMC_towers_clusterIDA, "tower_FEMC_clusIDA[tower_FEMC_N]/S");
    event_tree->Branch("tower_FEMC_clusIDB", t_fEMC_towers_clusterIDB, "tower_FEMC_clusIDB[tower_FEMC_N]/S");
    event_tree->Branch("tower_FEMC_trueID", t_fEMC_towers_cellTrueID, "tower_FEMC_trueID[tower_FEMC_N]/I");
  }

  // ===============================================================================================
  // Tree for cluster studies
  // ===============================================================================================
  if (enableTreeCluster){
    cluster_tree          = new TTree("cluster_tree", "cluster_tree");
    cluster_tree->SetDirectory(m_dir_main);

    t_mc_E                  = new float[maxNMC];
    t_mc_Phi                = new float[maxNMC];
    t_mc_Eta                = new float[maxNMC];
    t_fEMC_cluster_E      = new float[maxNCluster];
    t_fEMC_cluster_NCells = new int[maxNCluster];
    t_fEMC_cluster_Phi    = new float[maxNCluster];
    t_fEMC_cluster_Eta    = new float[maxNCluster];

    // MC particles
    cluster_tree->Branch("mc_N", &t_mc_N, "mc_N/I");
    cluster_tree->Branch("mc_E", t_mc_E, "mc_E[mc_N]/F");
    cluster_tree->Branch("mc_Phi", t_mc_Phi, "mc_Phi[mc_N]/F");
    cluster_tree->Branch("mc_Eta", t_mc_Eta, "mc_Eta[mc_N]/F");
    // clusters FECal
    cluster_tree->Branch("cluster_FEMC_N", &t_fEMC_clusters_N, "cluster_FEMC_N/I");
    cluster_tree->Branch("cluster_FEMC_E", t_fEMC_cluster_E, "cluster_FEMC_E[cluster_FEMC_N]/F");
    cluster_tree->Branch("cluster_FEMC_Ncells", t_fEMC_cluster_NCells, "cluster_FEMC_Ncells[cluster_FEMC_N]/I");
    cluster_tree->Branch("cluster_FEMC_Eta", t_fEMC_cluster_Eta, "cluster_FEMC_Eta[cluster_FEMC_N]/F");
    cluster_tree->Branch("cluster_FEMC_Phi", t_fEMC_cluster_Phi, "cluster_FEMC_Phi[cluster_FEMC_N]/F");
  }

  std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << std::endl;
  dd4hep::Detector& detector = dd4hep::Detector::getInstance();
  dd4hep::rec::CellIDPositionConverter cellid_converter(detector);
  std::cout << "--------------------------\nID specification:\n";
  try {
    m_decoder         = detector.readout("EcalEndcapPHits").idSpec().decoder();
    std::cout <<"1st: "<< m_decoder << std::endl;
    std::cout << "full list: " << " " << m_decoder->fieldDescription() << std::endl;
  } catch (...) {
      std::cout <<"2nd: "  << m_decoder << std::endl;
      m_log->error("readoutClass not in the output");
      throw std::runtime_error("readoutClass not in the output.");
  }

}

//******************************************************************************************
// ProcessSequential
//******************************************************************************************
void femc_studiesProcessor::Process(const std::shared_ptr<const JEvent>& event) {
// void femc_studiesProcessor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {
  using namespace std;

  // ===============================================================================================
  // process MC particles
  // ===============================================================================================
  auto mcParticles = event -> Get<edm4hep::MCParticle>("MCParticles");
  double mceta    = 0;
  double mcphi    = 0;
  double mcp      = 0;
  double mcenergy = 0;
  int iMC         = 0;
  for (auto mcparticle : mcParticles) {
    if (mcparticle->getGeneratorStatus() != 1)
      continue;
    auto& mom = mcparticle->getMomentum();
    // get particle energy
    mcenergy = mcparticle->getEnergy();
    //determine mceta from momentum
    mceta = -log(tan(atan2(sqrt(mom.x * mom.x + mom.y * mom.y), mom.z) / 2.));
    // determine mcphi from momentum
    mcphi = atan2(mom.y, mom.x);
    // determine mc momentum
    mcp = sqrt(mom.x * mom.x + mom.y * mom.y + mom.z * mom.z);
    m_log->trace("MC particle:{} \t {} \t {} \t totmom: {} phi {} eta {}", mom.x, mom.y, mom.z, mcp, mcphi, mceta);
    hMCEnergyVsEta->Fill(mcp,mceta);

    if (enableTreeCluster){
      if (iMC < maxNMC){
        t_mc_E[iMC]   = mcenergy;
        t_mc_Phi[iMC] = mcphi;
        t_mc_Eta[iMC] = mceta;
      }
    }
    iMC++;
  }
  if (enableTreeCluster) t_mc_N = iMC;
  // ===============================================================================================
  // process sim hits
  // ===============================================================================================
  std::vector<towersStrct> input_tower_sim;
  int nCaloHitsSim = 0;
  float sumActiveCaloEnergy = 0;
  float sumPassiveCaloEnergy = 0;
  auto simHits = event -> Get<edm4hep::SimCalorimeterHit>(nameSimHits.data());
  for (auto caloHit : simHits) {
    float x         = caloHit->getPosition().x / 10.;
    float y         = caloHit->getPosition().y / 10.;
    float z         = caloHit->getPosition().z / 10.;
    uint64_t cellID = caloHit->getCellID();
    float energy    = caloHit->getEnergy();
    double time = std::numeric_limits<double>::max();
    for (const auto& c : caloHit->getContributions()) {
        if (c.getTime() <= time) {
            time = c.getTime();
        }
    }

    auto detector_layer_x = floor((x+246)/2.5);
    auto detector_layer_y = floor((y+246)/2.5);
    auto detector_passive = 0;
    if(detector_passive == 0) {
      sumActiveCaloEnergy += energy;
    } else {
      sumPassiveCaloEnergy += energy;
    }

    if (detector_passive > 0) continue;
    // calc cell IDs
    int cellIDx = detector_layer_x;
    int cellIDy = detector_layer_y;
    int cellIDz = 0;
    nCaloHitsSim++;

    hPosCaloSimHitsXY->Fill(x, y);
    hCellESim_layerX->Fill(cellIDx, energy);
    hCellESim_layerY->Fill(cellIDy, energy);
    hCellTSim_layerX->Fill(cellIDx, time);

    //loop over input_tower_sim and find if there is already a tower with the same cellID
    bool found = false;
    for (auto& tower : input_tower_sim) {
      if (tower.cellID == cellID) {
        tower.energy += energy;
        found = true;
        break;
      }
    }
    if (!found) {
      towersStrct tempstructT;
      tempstructT.energy        = energy;
      tempstructT.time          = time;
      tempstructT.posx          = x;
      tempstructT.posy          = y;
      tempstructT.posz          = z;
      tempstructT.cellID        = cellID;
      tempstructT.cellIDx       = cellIDx;
      tempstructT.cellIDy       = cellIDy;
      tempstructT.cellIDz       = cellIDz;
      tempstructT.tower_trueID  = 0; //TODO how to get trueID?
      input_tower_sim.push_back(tempstructT);
    }
  }

  // ===============================================================================================
  // read rec hits & fill structs
  // ===============================================================================================
  auto recHits = event -> Get<edm4eic::CalorimeterHit>(nameRecHits.data());
  int nCaloHitsRec = 0;
  std::vector<towersStrct> input_tower_rec;
  std::vector<towersStrct> input_tower_recSav;
  // process rec hits
  for (auto caloHit : recHits) {
    float x         = caloHit->getPosition().x / 10.;
    float y         = caloHit->getPosition().y / 10.;
    float z         = caloHit->getPosition().z / 10.;
    uint64_t cellID = caloHit->getCellID();
    float energy    = caloHit->getEnergy();
    float time      = caloHit->getTime();

    auto detector_passive = 0;
    auto detector_layer_x = floor((x+246)/2.5);
    auto detector_layer_y = floor((y+246)/2.5);
    if (detector_passive > 0) continue;

    // calc cell IDs
    int cellIDx = detector_layer_x;
    int cellIDy = detector_layer_y;
    int cellIDz = 0;

    hPosCaloHitsXY->Fill(x, y);
    nCaloHitsRec++;

    //loop over input_tower_rec and find if there is already a tower with the same cellID
    bool found = false;
    for (auto& tower : input_tower_rec) {
      if (tower.cellID == cellID) {
        tower.energy += energy;
        found = true;
        break;
      }
    }
    if (!found) {
      towersStrct tempstructT;
      tempstructT.energy        = energy;
      tempstructT.time          = time;
      tempstructT.posx          = x;
      tempstructT.posy          = y;
      tempstructT.posz          = z;
      tempstructT.cellID        = cellID;
      tempstructT.cellIDx       = cellIDx;
      tempstructT.cellIDy       = cellIDy;
      tempstructT.tower_trueID  = 0; //TODO how to get trueID?
      input_tower_rec.push_back(tempstructT);
      input_tower_recSav.push_back(tempstructT);
    }
  }
  m_log->trace("FEMC mod: nCaloHits sim  {}\t rec {}", nCaloHitsSim, nCaloHitsRec);
  if (nCaloHitsRec > 0) nEventsWithCaloHits++;

  // ===============================================================================================
  // sort tower arrays
  // ===============================================================================================
  hSamplingFractionEta->Fill(mceta, sumActiveCaloEnergy / (sumActiveCaloEnergy+sumPassiveCaloEnergy));
  std::sort(input_tower_rec.begin(), input_tower_rec.end(), &acompare);
  std::sort(input_tower_recSav.begin(), input_tower_recSav.end(), &acompare);
  std::sort(input_tower_sim.begin(), input_tower_sim.end(), &acompare);

  // ===============================================================================================
  // calculated summed hit energy for rec and sim hits
  // ===============================================================================================

  // rec hits
  double tot_energyRecHit = 0;
  for (auto& tower : input_tower_rec) {
    tower.energy = tower.energy; // calibrate
    tot_energyRecHit += tower.energy;
  }

  double samplingFraction = 1.;
  // sim hits
  double tot_energySimHit = 0;
  for (auto& tower : input_tower_sim) {
    tower.energy = tower.energy/samplingFraction; // calibrate
    tot_energySimHit += tower.energy;
  }
  m_log->trace("Mc E: {} \t eta: {} \t sim E rec: {}\t rec E rec: {}", mcenergy, mceta, tot_energySimHit, tot_energyRecHit);

  // ===============================================================================================
  // Fill summed hits histos
  // ===============================================================================================
  // rec hits
  hClusterNCells_E_eta->Fill(mcenergy, nCaloHitsRec, mceta);
  hClusterEcalib_E_eta->Fill(mcenergy, tot_energyRecHit/mcenergy, mceta);
  hClusterEcalib_E_phi->Fill(mcenergy, tot_energyRecHit/mcenergy, mcphi);
  // sim hits
  hClusterSimNCells_E_eta->Fill(mcenergy, nCaloHitsSim, mceta);
  hClusterESimcalib_E_eta->Fill(mcenergy, tot_energySimHit/mcenergy, mceta);
  hClusterESimcalib_E_phi->Fill(mcenergy, tot_energySimHit/mcenergy, mcphi);

  // ===============================================================================================
  // MA clusterization
  // ===============================================================================================
  int removedCells  = 0;
  float minAggE     = 0.001;
  float seedE       = 0.20;

  if (input_tower_rec.size()> 0){

    // clean up rec array for clusterization
    while (input_tower_rec.at(input_tower_rec.size()-1).energy < minAggE ){
      input_tower_rec.pop_back();
      removedCells++;
    }
    m_log->trace("removed {} with E < {} GeV", removedCells, minAggE);

    int nclusters = 0;
    // vector of clusters
    std::vector<clustersStrct> clusters_calo;
    // vector of towers within the currently found cluster
    std::vector<towersStrct> cluster_towers;
    while (!input_tower_rec.empty() ) {
      cluster_towers.clear();
      clustersStrct tempstructC;
      // always start with highest energetic tower
      if(input_tower_rec.at(0).energy > seedE){
        m_log->trace("seed: {}\t {} \t {}", input_tower_rec.at(0).energy, input_tower_rec.at(0).cellIDx, input_tower_rec.at(0).cellIDy );
        tempstructC = findMACluster(seedE, minAggE, input_tower_rec, cluster_towers, 0.1);

        // determine remaining cluster properties from its towers
        float* showershape_eta_phi = CalculateM02andWeightedPosition(cluster_towers, tempstructC.cluster_E, 4.5);
        tempstructC.cluster_M02 = showershape_eta_phi[0];
        tempstructC.cluster_M20 = showershape_eta_phi[1];
        tempstructC.cluster_Eta = showershape_eta_phi[2];
        tempstructC.cluster_Phi = showershape_eta_phi[3];
        tempstructC.cluster_X = showershape_eta_phi[4];
        tempstructC.cluster_Y = showershape_eta_phi[5];
        tempstructC.cluster_Z = showershape_eta_phi[6];
        tempstructC.cluster_towers = cluster_towers;
        m_log->trace("---------> \t {} \tcluster with E = {} \tEta: {} \tPhi: {} \tX: {} \tY: {} \tZ: {} \tntowers: {} \ttrueID: {}", nclusters, tempstructC.cluster_E, tempstructC.cluster_Eta, tempstructC.cluster_Phi, tempstructC.cluster_X, tempstructC.cluster_Y, tempstructC.cluster_Z, tempstructC.cluster_NTowers, tempstructC.cluster_trueID );
        clusters_calo.push_back(tempstructC);

        nclusters++;
      } else {
        m_log->trace("remaining: {} largest: {} \t {}  \t {}  \t {}", (int)input_tower_rec.size(), input_tower_rec.at(0).energy, input_tower_rec.at(0).cellIDx, input_tower_rec.at(0).cellIDy,  input_tower_rec.at(0).cellIDz);
        input_tower_rec.clear();
      }
    }

    // -----------------------------------------------------------------------------------------------
    // --------------------------- Fill LFHCal MA clusters in tree and hists -------------------------
    // -----------------------------------------------------------------------------------------------
    std::sort(clusters_calo.begin(), clusters_calo.end(), &acompareCl);
    m_log->info("-----> found {} clusters" , clusters_calo.size());
    hRecNClusters_E_eta->Fill(mcenergy, clusters_calo.size(), mceta);
    int iCl = 0;
    for (auto& cluster : clusters_calo) {
      if (iCl < maxNCluster && enableTreeCluster){
        t_fEMC_cluster_E[iCl]       = (float)cluster.cluster_E;
        t_fEMC_cluster_NCells[iCl]  = (int)cluster.cluster_NTowers;
        t_fEMC_cluster_Eta[iCl]     = (float)cluster.cluster_Eta;
        t_fEMC_cluster_Phi[iCl]     = (float)cluster.cluster_Phi;
      }
      hRecClusterEcalib_E_eta->Fill(mcenergy, cluster.cluster_E/mcenergy, mceta);
      for (int iCell = 0; iCell < (int)cluster.cluster_towers.size(); iCell++){
        int pSav = 0;
        while(cluster.cluster_towers.at(iCell).cellID !=  input_tower_recSav.at(pSav).cellID && pSav < (int)input_tower_recSav.size() ) pSav++;
        if (cluster.cluster_towers.at(iCell).cellID == input_tower_recSav.at(pSav).cellID)
          input_tower_recSav.at(pSav).tower_clusterIDA = iCl;
      }

      if (iCl == 0){
        hRecClusterEcalib_Ehigh_eta->Fill(mcenergy, cluster.cluster_E/mcenergy, mceta);
        hRecClusterNCells_Ehigh_eta->Fill(mcenergy, cluster.cluster_NTowers, mceta);
      }
      iCl++;
      m_log->trace("MA cluster {}:\t {} \t {}", iCl, cluster.cluster_E, cluster.cluster_NTowers);
    }
    if (iCl < maxNCluster && enableTreeCluster) t_fEMC_clusters_N = (int)iCl;

    clusters_calo.clear();
  } else {
    hRecNClusters_E_eta->Fill(mcenergy, 0., mceta);
    if (enableTreeCluster) t_fEMC_clusters_N = 0;
  }

  // ===============================================================================================
  // ------------------------------- Fill LFHCAl Island clusters in hists --------------------------
  // ===============================================================================================
  int iClF          = 0;
  float highestEFr  = 0;
  int iClFHigh      = 0;

  auto fecalClustersF = event -> Get<edm4eic::Cluster>(nameClusters.data());
  for (auto& cluster : fecalClustersF) {
    if (cluster->getEnergy() > highestEFr){
      iClFHigh    = iClF;
      highestEFr  = cluster->getEnergy();
    }
    hRecFClusterEcalib_E_eta->Fill(mcenergy, cluster->getEnergy()/mcenergy, mceta);
    m_log->trace("Island cluster {}:\t {} \t {}", iClF, cluster->getEnergy(), cluster->getNhits());

    for (auto& hit: cluster->getHits()){
      int pSav = 0;
      while(hit.getCellID() !=  input_tower_recSav.at(pSav).cellID && pSav < (int)input_tower_recSav.size() ) pSav++;
      if (hit.getCellID() == input_tower_recSav.at(pSav).cellID)
        input_tower_recSav.at(pSav).tower_clusterIDB = iClF;
    }
    iClF++;
  }
  hRecFNClusters_E_eta->Fill(mcenergy, iClF, mceta);
  // fill hists for highest Island cluster
  iClF          = 0;
  for (auto& cluster : fecalClustersF) {
    if (iClF == iClFHigh){
      hRecFClusterEcalib_Ehigh_eta->Fill(mcenergy, cluster->getEnergy()/mcenergy, mceta);
      hRecFClusterNCells_Ehigh_eta->Fill(mcenergy, cluster->getNhits(), mceta);
    }
    iClF++;
  }

  // ===============================================================================================
  // Write clusterizer tree & clean-up variables
  // ===============================================================================================
  if (enableTree){
    t_fEMC_towers_N = (int)input_tower_recSav.size();
    for (int iCell = 0; iCell < (int)input_tower_recSav.size(); iCell++){
      m_log->trace("{} \t {} \t {} \t {} \t {} \t {}", input_tower_recSav.at(iCell).cellIDx, input_tower_recSav.at(iCell).cellIDy , input_tower_recSav.at(iCell).energy, input_tower_recSav.at(iCell).tower_clusterIDA, input_tower_recSav.at(iCell).tower_clusterIDB  );

      t_fEMC_towers_cellE[iCell]      = (float)input_tower_recSav.at(iCell).energy;
      t_fEMC_towers_cellT[iCell]      = (float)input_tower_recSav.at(iCell).time;
      t_fEMC_towers_cellIDx[iCell]    = (short)input_tower_recSav.at(iCell).cellIDx;
      t_fEMC_towers_cellIDy[iCell]    = (short)input_tower_recSav.at(iCell).cellIDy;
      t_fEMC_towers_clusterIDA[iCell] = (short)input_tower_recSav.at(iCell).tower_clusterIDA;
      t_fEMC_towers_clusterIDB[iCell] = (short)input_tower_recSav.at(iCell).tower_clusterIDB;
      t_fEMC_towers_cellTrueID[iCell] = (int)input_tower_recSav.at(iCell).tower_trueID;
    }

    event_tree->Fill();

    t_fEMC_towers_N = 0;
    for (Int_t itow = 0; itow < maxNTowers; itow++){
      t_fEMC_towers_cellE[itow]       = 0;
      t_fEMC_towers_cellT[itow]       = 0;
      t_fEMC_towers_cellIDx[itow]     = 0;
      t_fEMC_towers_cellIDy[itow]     = 0;
      t_fEMC_towers_clusterIDA[itow]  = 0;
      t_fEMC_towers_clusterIDB[itow]  = 0;
      t_fEMC_towers_cellTrueID[itow]  = 0;
    }
  }

  // ===============================================================================================
  // Write cluster tree & clean-up variables
  // ===============================================================================================
  if (enableTreeCluster){
    cluster_tree->Fill();

    t_mc_N                = 0;
    t_fEMC_clusters_N   = 0;
    t_fEMC_clusters_N    = 0;
    for (Int_t iMC = 0; iMC < maxNMC; iMC++){
      t_mc_E[iMC]                   = 0;
      t_mc_Phi[iMC]                 = 0;
      t_mc_Eta[iMC]                 = 0;
    }
    for (Int_t iCl = 0; iCl < maxNCluster; iCl++){
      t_fEMC_cluster_E[iCl]       = 0;
      t_fEMC_cluster_NCells[iCl]  = 0;
      t_fEMC_cluster_Eta[iCl]     = 0;
      t_fEMC_cluster_Phi[iCl]     = 0;
    }
  }

}


//******************************************************************************************
// FinishWithGlobalRootLock
//******************************************************************************************
void femc_studiesProcessor::Finish() {
  std::cout << "------> FEMC " << nEventsWithCaloHits << " with calo info present"<< std::endl;
  if (enableTreeCluster) cluster_tree->Write();
  // Do any final calculations here.
}
