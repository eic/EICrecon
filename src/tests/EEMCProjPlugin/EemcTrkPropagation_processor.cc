
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>

#include "EemcTrkPropagation_processor.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>

#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include <services/rootfile/RootFile_service.h>
#include <services/geometry/acts/ACTSGeo_service.h>

#include <edm4hep/MCParticle.h>

//------------------
// (Constructor)
//------------------
EemcTrkPropagation_processor::EemcTrkPropagation_processor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void EemcTrkPropagation_processor::Init()
{
    std::string plugin_name=("Eemc_TrkPropagation");

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

    // Create ECal surface that will be used for propagation
    auto transform = Acts::Transform3::Identity();

    // make a reference disk to mimic electron-endcap ECal
    const auto ecalEndcapNZ = -1840.025;
    const auto ecalEndcapNMinR = 50;
    const auto ecalEndcapNMaxR = 950;
    auto ecalEndcapNBounds = std::make_shared<Acts::RadialBounds>(ecalEndcapNMinR, ecalEndcapNMaxR);
    auto ecalEndcapNTrf = transform * Acts::Translation3(Acts::Vector3(0, 0, ecalEndcapNZ));
    m_ecal_surface = Acts::Surface::makeShared<Acts::DiscSurface>(ecalEndcapNTrf, ecalEndcapNBounds);
}


//------------------
// Process
//------------------
// This function is called every event
void EemcTrkPropagation_processor::Process(const std::shared_ptr<const JEvent>& event)
{
    m_log->trace("EemcTrkPropagation_processor event");

    //Generated particles (only one particle generated)
    double mceta = 0; //particle eta
    double mcphi = 0; //particle phi
    double mcp = 0; //total momentum
    double mce = 0; //energy
    int num_primary = 0; //Number of primary particles

    auto mcParticles = event->Get<edm4hep::MCParticle>("MCParticles");

    for( size_t iParticle=0;iParticle<mcParticles.size(); iParticle++ ){
	
	auto mcparticle = mcParticles[iParticle];
        if(mcparticle->getGeneratorStatus() != 1) continue;
        auto& mom = mcparticle->getMomentum();
        
        mceta = -log(tan(atan2(sqrt(mom.x*mom.x+mom.y*mom.y),mom.z)/2.));
        mcphi = atan2(mom.y, mom.x);
	mcp = sqrt(mom.x*mom.x+mom.y*mom.y+mom.z*mom.z);
        mce = sqrt(mcp*mcp + mcparticle->getMass()*mcparticle->getMass());	

	num_primary++;

    }

    //Reconstructed track momentum
    double recp = 0; //total momentum
    int num_rec = 0; //Number of reconstructed particles

    auto RecParticles = event->Get<edm4eic::ReconstructedParticle>("ReconstructedParticles"); 
      
    for( size_t iParticle=0;iParticle<RecParticles.size(); iParticle++ ){

        auto Recparticle = RecParticles[iParticle];
        auto& mom = Recparticle->getMomentum();
        recp = sqrt(mom.x*mom.x+mom.y*mom.y+mom.z*mom.z);
        num_rec++;
    }
    

    //EEMC Cluster energy and cluster position (to compare to track projection)
    double cluse = 0; //Energy of largest cluster
    double num_clus = 0; //Number of clusters
    double clus_x = 0; //Cluster position x
    double clus_y = 0; //Cluster position y
    double clus_z = 0; //Cluster position z

    auto EEMCClusters = event->Get<edm4eic::Cluster>("EcalEndcapNClusters");

    for( size_t iClus=0;iClus<EEMCClusters.size(); iClus++ ){

        auto cluster = EEMCClusters[iClus];
        cluse = cluster->getEnergy();
        auto& clus_pos = cluster->getPosition();
	clus_x = clus_pos.x; clus_y = clus_pos.y; clus_z = clus_pos.z;
        num_clus++;
    }

    //Print generated info to log
    m_log->trace("Number of primary generated particles:");
    m_log->trace("{:>10}",num_primary);
    m_log->trace("Generated particle eta, p, E:");
    m_log->trace("{:>10.2f} {:>10.2f} {:>10.2f}", mceta, mcp, mce);

    m_log->trace("Number of reconstructed tracks:");
    m_log->trace("{:>10}",num_rec);
    m_log->trace("Reconstructed track p:");
    m_log->trace("{:>10.2f}", recp);

    m_log->trace("Number of EEMC clusters:");
    m_log->trace("{:>10}",num_clus);
    m_log->trace("Cluster E, x, y, z:");
    m_log->trace("{:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", cluse, clus_x, clus_y, clus_z);

    // Get trajectories from tracking
    auto trajectories = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories");

    // Iterate over trajectories
    m_log->debug("Propagating through {} trajectories", trajectories.size());
    for (size_t traj_index = 0; traj_index < trajectories.size(); traj_index++) {
        auto &trajectory = trajectories[traj_index];
        m_log->trace(" -- trajectory {} --", traj_index);

        edm4eic::TrackPoint* projection_point;
        try {
            // >>> try to propagate to surface <<<
            projection_point = m_propagation_algo.propagate(trajectory, m_ecal_surface);
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
}


//------------------
// Finish
//------------------
void EemcTrkPropagation_processor::Finish()
{
	m_log->trace("EemcTrkPropagation_processor finished\n");

}

