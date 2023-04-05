
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>

#include "TrackSeedingTest_processor.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>

#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include <services/rootfile/RootFile_service.h>
#include <services/geometry/acts/ACTSGeo_service.h>




//------------------
// OccupancyAnalysis (Constructor)
//------------------
TrackSeedingTest_processor::TrackSeedingTest_processor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void TrackSeedingTest_processor::Init()
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

    // Create HCal surface that will be used for propagation
    auto transform = Acts::Transform3::Identity();

    // make a reference disk to mimic electron-endcap HCal
    const auto hcalEndcapNZ = -3322.;
    const auto hcalEndcapNMinR = 83.01;
    const auto hcalEndcapNMaxR = 950;
    auto hcalEndcapNBounds = std::make_shared<Acts::RadialBounds>(hcalEndcapNMinR, hcalEndcapNMaxR);
    auto hcalEndcapNTrf = transform * Acts::Translation3(Acts::Vector3(0, 0, hcalEndcapNZ));
    m_hcal_surface = Acts::Surface::makeShared<Acts::DiscSurface>(hcalEndcapNTrf, hcalEndcapNBounds);
}


//------------------
// Process
//------------------
// This function is called every event
void TrackSeedingTest_processor::Process(const std::shared_ptr<const JEvent>& event)
{
    m_log->trace("TrackSeedingTest_processor event");

    // Get trajectories from tracking
    auto trajectories = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories");


    auto clusters = event->Get<edm4eic::Cluster>("HcalEndcapNClusters");

    // Iterate over trajectories
    m_log->debug("Propagating through {} trajectories", trajectories.size());
    for (size_t traj_index = 0; traj_index < trajectories.size(); traj_index++) {
        auto &trajectory = trajectories[traj_index];
        m_log->trace(" -- trajectory {} --", traj_index);

        edm4eic::TrackPoint* projection_point;
        try {
            // >>> try to propagate to surface <<<
            projection_point = m_propagation_algo.propagate(trajectory, m_hcal_surface);
        }
        catch(std::exception &e) {
            m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
        }

        if(!projection_point) {
            m_log->trace("   could not propagate!", traj_index);
            continue;
        }

        // Now go through reconstructed tracks points
        auto pos = projection_point->position;
        auto length =  projection_point->pathlength;
        m_log->trace("   {:>10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", traj_index, pos.x, pos.y, pos.z, length);
    }

    m_log->trace("Now points created in a factory");
    auto proj_from_factory = event->Get<edm4eic::TrackPoint>("HcalEndcapNProjections");
    for(auto point: proj_from_factory) {
        // Now go through reconstructed tracks points
        auto pos = point->position;
        auto length =  point->pathlength;
        m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", pos.x, pos.y, pos.z, length);
    }
}


//------------------
// Finish
//------------------
void TrackSeedingTest_processor::Finish()
{
//    m_log->trace("TrackSeedingTest_processor finished\n");

}
