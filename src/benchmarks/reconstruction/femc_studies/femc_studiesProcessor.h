// Copyright 2023, Friederike Bock
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//  Sections Copyright (C) 2023 Friederike Bock
//  under SPDX-License-Identifier: LGPL-3.0-or-later

#include <JANA/JEventProcessor.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TFile.h>
#include <TTree.h>

// Include appropirate class headers. e.g.
#include <edm4hep/SimCalorimeterHit.h>
// #include <detectors/BEMC/BEMCRawCalorimeterHit.h>

#include "Acts/Utilities/Logger.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include <Acts/Surfaces/DiscSurface.hpp>
#include "Acts/Definitions/Common.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp"
#include <Acts/Material/IMaterialDecorator.hpp>
// #include <extensions/spdlog/SpdlogMixin.h>
#include <edm4hep//MCParticle.h>
#include <edm4hep/SimTrackerHit.h>
#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/ProtoCluster.h>

#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <algorithms/tracking/ActsGeometryProvider.h>
#include <services/geometry/acts/ACTSGeo_service.h>
#include <algorithms/tracking/TrackPropagation.h>
#include <spdlog/spdlog.h>

class femc_studiesProcessor: public JEventProcessor {
public:
    femc_studiesProcessor() { SetTypeName(NAME_OF_THIS); }

    void Init() override;
//     void InitWithGlobalRootLock() override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
//     void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
//     void FinishWithGlobalRootLock() override;
    void Finish() override;
    TDirectory *m_dir_main;
    // Declare histogram and tree pointers here. e.g.
    TH2D* hMCEnergyVsEta;
    TH3D* hClusterEcalib_E_eta;
    TH3D* hClusterNCells_E_eta;
    TH3D* hClusterEcalib_E_phi;
    TH2D* hPosCaloHitsXY;
    TH3D* hClusterESimcalib_E_eta;
    TH3D* hClusterSimNCells_E_eta;
    TH3D* hClusterESimcalib_E_phi;
    TH2D* hCellESim_layerX;
    TH2D* hCellESim_layerY;
    TH2D* hCellTSim_layerX;
    TH2D* hPosCaloSimHitsXY;
    TH3D* hRecClusterEcalib_E_eta;
    TH3D* hRecNClusters_E_eta;
    TH3D* hRecClusterEcalib_Ehigh_eta;
    TH3D* hRecClusterNCells_Ehigh_eta;
    TH3D* hRecFClusterEcalib_E_eta;
    TH3D* hRecFNClusters_E_eta;
    TH3D* hRecFClusterEcalib_Ehigh_eta;
    TH3D* hRecFClusterNCells_Ehigh_eta;
    TH2D* hSamplingFractionEta;

    bool enableTree       = false;
    TTree* event_tree;
    const int maxNTowers  = 65000;
    int     t_fEMC_towers_N;
    short*  t_fEMC_towers_cellIDx;
    short*  t_fEMC_towers_cellIDy;
    short*  t_fEMC_towers_clusterIDA;
    short*  t_fEMC_towers_clusterIDB;
    float*  t_fEMC_towers_cellE;
    float*  t_fEMC_towers_cellT;
    int*    t_fEMC_towers_cellTrueID;

    bool enableTreeCluster  = false;
    TTree* cluster_tree;
    const int maxNCluster   = 50;
    const int maxNMC        = 50;
    int     t_mc_N;
    float*  t_mc_E;
    float*  t_mc_Phi;
    float*  t_mc_Eta;
    int     t_fEMC_clusters_N;
    float*  t_fEMC_cluster_E;
    int*    t_fEMC_cluster_NCells;
    float*  t_fEMC_cluster_Phi;
    float*  t_fEMC_cluster_Eta;

    int nEventsWithCaloHits = 0;
    std::shared_ptr<spdlog::logger> m_log;
    dd4hep::BitFieldCoder* m_decoder;
    std::string nameSimHits         = "EcalEndcapPHits";
    std::string nameRecHits         = "EcalEndcapPRecHits";
    std::string nameClusters        = "EcalEndcapPClusters";
    std::string nameProtoClusters   = "EcalEndcapPIslandProtoClusters";
    short iLx;
    short iLy;

};
