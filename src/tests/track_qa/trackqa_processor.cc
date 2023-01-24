#include "trackqa_processor.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>

#include <algorithms/tracking/JugTrack/TrackingResultTrajectory.hpp>
#include <algorithms/tracking/ParticlesFromTrackFitResult.h>

#include <services/rootfile/RootFile_service.h>
#include <services/geometry/acts/ACTSGeo_service.h>

#include <Acts/EventData/MultiTrajectory.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>

#include <edm4hep/MCParticle.h>

//------------------
// (Constructor)
//------------------
trackqa_processor::trackqa_processor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void trackqa_processor::Init()
{
    std::string plugin_name=("track_qa");

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

    //Define histograms

    // Get log level from user parameter or default
    InitLogger(plugin_name);

    auto acts_service = app->GetService<ACTSGeo_service>();
    m_geo_provider = acts_service->actsGeoProvider();

}


//------------------
// Process
//------------------
// This function is called every event
void trackqa_processor::Process(const std::shared_ptr<const JEvent>& event)
{
    m_log->trace("");
    m_log->trace("trackqa_processor event");

    //Generated particles (only one particle generated)
    double mceta = 0; //particle eta
    double mcphi = 0; //particle phi
    double mcp = 0; //total momentum
    double mce = 0; //energy
    int mcid = 0; //PDG id
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

        mcid = mcparticle->getPDG();

	    num_primary++;
    }

    //Print generated info to log
    m_log->trace("-------------------------");
    m_log->trace("Number of primary generated particles:");
    m_log->trace("{:>10}",num_primary);
    m_log->trace("Generated particle id, eta, p, E:");
    m_log->trace("{:>10} {:>10.2f} {:>10.2f} {:>10.2f}", mcid, mceta, mcp, mce);

    //Tracker hits
    std::vector<std::string> m_data_names = {
        "SiBarrelTrackerRecHits",         // Barrel Tracker
        "SiBarrelVertexRecHits",          // Vertex
        "SiEndcapTrackerRecHits",         // End Cap tracker
        "MPGDBarrelRecHits",              // MPGD
        "MPGDDIRCRecHits",                // MPGD DIRC
        "TOFBarrelRecHit",                // Barrel TOF
        "TOFEndcapRecHits"                // End Cap TOF
    };

    m_log->trace("-------------------------");
    int nHitsallTrackers = 0;
    
    for(size_t name_index = 0; name_index < m_data_names.size(); name_index++ ) {
        auto data_name = m_data_names[name_index];
        auto hits = event->Get<edm4eic::TrackerHit>(data_name);
        m_log->trace("Detector {} has {} digitized hits.",data_name,hits.size());
        
        int nHits_detector = 0;
        for(auto hit: hits) {
            auto x = hit->getPosition().x;
            auto y = hit->getPosition().y;
            auto z = hit->getPosition().z;
            auto r = sqrt(x*x + y*y);
            auto etahit = -log(tan(atan2(r,z)/2.));

            nHits_detector++;
            m_log->trace("For digitized hit number {}:",nHits_detector);
            m_log->trace("Hit x, y, z, r, eta:");
            m_log->trace("{:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}",x,y,z,r,etahit);
            nHitsallTrackers++;
        }
        m_log->trace("");
    }

    m_log->trace("Total number of tracker hits is {}",nHitsallTrackers);
    m_log->trace("-------------------------");

    //ACTS Trajectories
    auto trajectories = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories");

    m_log->trace("Number of ACTS Trajectories: {}", trajectories.size());
    m_log->trace("");

    // Loop over the trajectories
    for (const auto& traj : trajectories) {

        // Get the entry index for the single trajectory
        // The trajectory entry indices and the multiTrajectory
        const auto &mj = traj->multiTrajectory();
        const auto &trackTips = traj->tips();

        // Skip empty
        if (trackTips.empty()) {
            m_log->trace("Empty multiTrajectory.");
            continue;
        }
        auto &trackTip = trackTips.front();

        // Collect the trajectory summary info
        auto trajState = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
        int m_nMeasurements = trajState.nMeasurements;
        int m_nStates = trajState.nStates;
        auto m_chi2Sum = trajState.chi2Sum;

        //General Trajectory Information
        m_log->trace("Number of elements in trackTips {}", trackTips.size());
        m_log->trace("Number of measurements in trajectory: {}", m_nMeasurements);
        m_log->trace("Number of states in trajectory     : {}", m_nStates);
        m_log->trace("Total chi-square of trajectory    : {:>8.2f}",m_chi2Sum);
   
        //Initial Trajectory Parameters
        const auto &initial_bound_parameters = traj->trackParameters(trackTip);
        const auto &parameter  = initial_bound_parameters.parameters();
        const auto &covariance = *initial_bound_parameters.covariance();

        m_log->trace("{:>8} {:>8} {:>8} {:>8} {:>8} {:>10} {:>10} {:>10}",
                    "[loc 0]","[loc 1]", "[phi]", "[theta]", "[q/p]", "[err phi]", "[err th]", "[err q/p]");
        m_log->trace("{:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>10.2g} {:>10.2g} {:>10.2g}",
                    parameter[Acts::eBoundLoc0],
                    parameter[Acts::eBoundLoc1],
                    parameter[Acts::eBoundPhi],
                    parameter[Acts::eBoundTheta],
                    parameter[Acts::eBoundQOverP],
                    sqrt(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
                    sqrt(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
                    sqrt(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP))
                    );

        auto p_traj = fabs(1. / parameter[Acts::eBoundQOverP]);
        auto eta_traj = -log( tan(parameter[Acts::eBoundTheta]/2.) );
        m_log->trace("Trajectory p, eta:");
        m_log->trace("{:>10.2f} {:>10.2f}",p_traj,eta_traj);
        m_log->trace("");

        //Information at tracking layers
        int m_nCalibrated = 0;
        int state_counter = 0;
        // visit the track points
        mj.visitBackwards(trackTip, [&](auto &&trackstate) {

            state_counter++;
            m_log->trace("Now at State number {}",state_counter);

            // get volume info
            auto geoID = trackstate.referenceSurface().geometryId();
            auto volume = geoID.volume();
            auto layer = geoID.layer();

            if (trackstate.hasCalibrated()) {
                m_nCalibrated++;
                m_log->trace("This is a calibrated state.");
            }
            else{
                m_log->trace("This is NOT a calibrated state.");
            }

            // get track state parameters and their covariances
            const auto &state_params = trackstate.predicted();
            const auto &state_covar = trackstate.predictedCovariance();

            //First print same information as for initial track parameters
            m_log->trace("{:>8} {:>8} {:>8} {:>8} {:>8} {:>10} {:>10} {:>10}",
                    "[loc 0]","[loc 1]", "[phi]", "[theta]", "[q/p]", "[err phi]", "[err th]", "[err q/p]");
            m_log->trace("{:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>10.2g} {:>10.2g} {:>10.2g}",
                    state_params[Acts::eBoundLoc0],
                    state_params[Acts::eBoundLoc1],
                    state_params[Acts::eBoundPhi],
                    state_params[Acts::eBoundTheta],
                    state_params[Acts::eBoundQOverP],
                    sqrt(state_covar(Acts::eBoundPhi, Acts::eBoundPhi)),
                    sqrt(state_covar(Acts::eBoundTheta, Acts::eBoundTheta)),
                    sqrt(state_covar(Acts::eBoundQOverP, Acts::eBoundQOverP))
                    );

            // convert local to global
            auto global = trackstate.referenceSurface().localToGlobal(
                    m_geo_provider->getActsGeometryContext(),
                    {parameter[Acts::eBoundLoc0], parameter[Acts::eBoundLoc1]},
                    {0, 0, 0} );
            auto global_r = sqrt(global.x()*global.x()+global.y()*global.y());
            m_log->trace("State global x, y, z, r and pathlength:");
            m_log->trace("{:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}",global.x(),global.y(),global.z(),global_r,trackstate.pathLength());
            
            m_log->trace("");
        }); //End visiting track points
        m_log->trace("Number of calibrated states: {}",m_nCalibrated);
    } //End loop over trajectories

    m_log->trace("-------------------------");

}

//------------------
// Finish
//------------------
void trackqa_processor::Finish()
{
	m_log->trace("trackqa_processor finished\n");
}

