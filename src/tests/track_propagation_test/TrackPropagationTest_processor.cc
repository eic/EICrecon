
#include "TrackPropagationTest_processor.h"

#include "extensions/spdlog/SpdlogExtensions.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <fmt/core.h>

#include <TDirectory.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <Math/LorentzVector.h>
#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>

#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/MCParticle.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/ReconstructedParticle.h>

#include <algorithms/tracking/TrackerSourceLinkerResult.h>
#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include <algorithms/tracking/JugTrack/Track.hpp>
#include <services/rootfile/RootFile_service.h>
#include <services/geometry/acts/ACTSGeo_service.h>

using namespace fmt;

//------------------
// OccupancyAnalysis (Constructor)
//------------------
TrackPropagationTest_processor::TrackPropagationTest_processor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void TrackPropagationTest_processor::Init()
{
    std::string plugin_name=("track_propagation_test");

    // Get JANA application
    auto app = GetApplication();

    // Ask service locator a file to write histograms to
    auto root_file_service = app->GetService<RootFile_service>();

    // Get TDirectory for histograms root file
    auto globalRootLock = app->GetService<JGlobalRootLock>();
    globalRootLock->acquire_write_lock();
    auto file = root_file_service->GetHistFile();
    globalRootLock->release_lock();

    // Create a directory for this plugin. And subdirectories for series of histograms
    m_dir_main = file->mkdir(plugin_name.c_str());

    // Get log level from user parameter or default
    InitLogger(plugin_name);

    auto acts_service = GetApplication()->GetService<ACTSGeo_service>();

    m_propagation_algo.init(acts_service->actsGeoProvider(), logger());
}


//------------------
// Process
//------------------
// This function is called every event
void TrackPropagationTest_processor::Process(const std::shared_ptr<const JEvent>& event)
{
    m_log->trace("TrackPropagationTest_processor event");

    auto trk_result = event->GetSingle<ParticlesFromTrackFitResult>("CentralTrackingParticles");

    // Get trajectories from tracking
    auto trajectories = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories");

    std::vector<edm4eic::TrackSegment*> tracks;
    try {
        tracks = m_propagation_algo.execute(trajectories);
    }
    catch(std::exception &e) {
        m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
    }

    // Now go through reconstructed tracks points
    logger()->trace("Going over tracks:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[length]");
    for( auto track_segment : tracks ){
        logger()->trace(" Track trajectory");

        for(auto point: track_segment->getPoints()) {
            auto &pos = point.position;
            m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", pos.x, pos.y, pos.z, point.pathlength);
        }
    }
}


//------------------
// Finish
//------------------
void TrackPropagationTest_processor::Finish()
{
//    m_log->trace("TrackPropagationTest_processor finished\n");

}

