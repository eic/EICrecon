
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEventProcessorSequentialRoot.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TFile.h>

// Include appropirate class headers. e.g.
// #include <edm4hep/SimCalorimeterHit.h>
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
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/TrackSegment.h>
#include <edm4eic/TrackerHit.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <algorithms/tracking/ActsGeometryProvider.h>
#include <services/geometry/acts/ACTSGeo_service.h>
#include <algorithms/tracking/TrackPropagation.h>
#include <spdlog/spdlog.h>

class tracking_studiesProcessor: public JEventProcessorSequentialRoot {
private:

    // Data objects we will need from JANA e.g.
    // PrefetchT<edm4hep::SimCalorimeterHit> rawhits   = {this, "EcalBarrelHits"};
    // PrefetchT<BEMCRawCalorimeterHit>      digihits  = {this};
    PrefetchT<edm4hep::MCParticle>  mcParticles   = {this, "MCParticles" };
    PrefetchT<edm4eic::TrackSegment> trackSegments = {this, "CentralTrackSegments"};
    PrefetchT<edm4eic::TrackerHit> barrelHits = {this, "TOFBarrelRecHit"};
    PrefetchT<edm4eic::TrackerHit> endcapHits = {this, "TOFEndcapRecHits"};

    // Declare histogram and tree pointers here. e.g.
    // TH2D* hEdigi = nullptr ;

public:
    tracking_studiesProcessor() { SetTypeName(NAME_OF_THIS); }
    
    void InitWithGlobalRootLock() override;
    void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
    void FinishWithGlobalRootLock() override;
    TDirectory *m_dir_main;
    TH1D* hPosTestX;
    TH3D* nHitsTrackVsEtaVsP;
    int nTracksFilled = 0;
    int nEventsFilled = 0;
    TH3D* nHitsEventVsEtaVsP;
    std::shared_ptr<spdlog::logger> m_log;
    // Acts::GeometryContext geoContext;

    /// Tracking propagation algorithm
    eicrecon::TrackPropagation m_propagation_algo;

    std::shared_ptr<const ActsGeometryProvider> m_geo_provider;
    std::shared_ptr<Acts::DiscSurface> m_mRICH_center_surface;
    std::shared_ptr<Acts::DiscSurface> m_dRICH_center_surface;

};
