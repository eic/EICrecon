
//
// Template for this file generated with eicmkplugin.py
//

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
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/CylinderSurface.hpp>
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

class trackmatching_studiesProcessor: public JEventProcessor {
public:
    trackmatching_studiesProcessor() { SetTypeName(NAME_OF_THIS); }

    void Init() override;
//     void InitWithGlobalRootLock() override;
//     void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void Finish() override;
//     void FinishWithGlobalRootLock() override;
    TDirectory *m_dir_main;
    // Declare histogram and tree pointers here. e.g.

    TH2D* hMCEnergyVsEta;

    TH2D* hFEMC_dEta_dPhi;
    TH2D* hEEMC_dEta_dPhi;
    TH2D* hBEMC_dEta_dPhi;
    TH2D* hLFHCAL_dEta_dPhi;
    TH2D* hOHCAL_dEta_dPhi;
    TH2D* hEHCAL_dEta_dPhi;

    TH1D* hECalib_FEMC_matched;
    TH1D* hECalib_EEMC_matched;
    TH1D* hECalib_BEMC_matched;
    TH1D* hECalib_LFHCAL_matched;
    TH1D* hECalib_OHCAL_matched;
    TH1D* hECalib_EHCAL_matched;

    TH1D* hECalib_forward_matched;
    TH1D* hECalib_barrel_matched;
    TH1D* hECalib_backwards_matched;

    int nEventsWithCaloHits = 0;
    std::shared_ptr<spdlog::logger> m_log;
    // Acts::GeometryContext geoContext;

//     /// Tracking propagation algorithm
    eicrecon::TrackPropagation m_propagation_algo;

    std::shared_ptr<const ActsGeometryProvider> m_geo_provider;
    std::shared_ptr<Acts::DiscSurface> m_LFHCAL_prop_surface;
    std::shared_ptr<Acts::DiscSurface> m_FEMC_prop_surface;
    std::shared_ptr<Acts::DiscSurface> m_EEMC_prop_surface;
    std::shared_ptr<Acts::CylinderSurface> m_BEMC_prop_surface;
    std::shared_ptr<Acts::CylinderSurface> m_OHCAL_prop_surface;
    std::shared_ptr<Acts::DiscSurface> m_EHCAL_prop_surface;
    

    std::string nameSimHits         = "LFHCALHits";
    std::string nameRecHits         = "LFHCALRecHits";
    std::string nameClusters        = "LFHCALClusters";
    std::string nameProtoClusters   = "LFHCALIslandProtoClusters";

};
