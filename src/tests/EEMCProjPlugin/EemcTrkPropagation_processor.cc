
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

    //Define histograms
    h1a = new TH2D("h1a","",100,0,25,100,0,25);
    h1a->GetXaxis()->SetTitle("True Momentum [GeV/c]");h1a->GetXaxis()->CenterTitle();
    h1a->GetYaxis()->SetTitle("Rec. Track Momentum [GeV/c]");h1a->GetYaxis()->CenterTitle();
    h1a->SetDirectory(m_dir_main);

    h1b = new TH2D("h1b","",100,0,25,100,0,25);
    h1b->GetXaxis()->SetTitle("True Energy [GeV]");h1b->GetXaxis()->CenterTitle();
    h1b->GetYaxis()->SetTitle("Rec. Cluster Energy [GeV]");h1b->GetYaxis()->CenterTitle();
    h1b->SetDirectory(m_dir_main);

    h2a = new TH1D("h2a","",100,-1900,-1800);
    h2a->GetXaxis()->SetTitle("Track Z Projection to EEMC [mm]");h2a->GetXaxis()->CenterTitle();
    h2a->SetLineColor(kBlack);h2a->SetLineWidth(2);
    h2a->SetDirectory(m_dir_main);

    h2b = new TH1D("h2b","",100,-1900,-1800);
    h2b->GetXaxis()->SetTitle("EEMC Cluster Z position [mm]");h2b->GetXaxis()->CenterTitle();
    h2b->SetLineColor(kBlack);h2b->SetLineWidth(2);
    h2b->SetDirectory(m_dir_main);

    h3a = new TH2D("h3a","",100,-1000,1000,100,-1000,1000);
    h3a->GetXaxis()->SetTitle("Track X Projection to EEMC [mm]");h3a->GetXaxis()->CenterTitle();
    h3a->GetYaxis()->SetTitle("EEMC Cluster X position [mm]");h3a->GetYaxis()->CenterTitle();
    h3a->GetYaxis()->SetTitleOffset(1.2);
    h3a->SetDirectory(m_dir_main);

    h3b = new TH2D("h3b","",100,-1000,1000,100,-1000,1000);
    h3b->GetXaxis()->SetTitle("Track Y Projection to EEMC [mm]");h3b->GetXaxis()->CenterTitle();
    h3b->GetYaxis()->SetTitle("EEMC Cluster Y position [mm]");h3b->GetYaxis()->CenterTitle();
    h3b->GetYaxis()->SetTitleOffset(1.2);
    h3b->SetDirectory(m_dir_main);

    h4a = new TH1D("h4a","",100,-80,80);
    h4a->GetXaxis()->SetTitle("EEMC Cluster X - Projection X [mm]");h4a->GetXaxis()->CenterTitle();
    h4a->SetLineColor(kBlack);h4a->SetLineWidth(2);
    h4a->SetDirectory(m_dir_main);

    h4b = new TH1D("h4b","",100,-80,80);
    h4b->GetXaxis()->SetTitle("EEMC Cluster Y - Projection Y [mm]");h4b->GetXaxis()->CenterTitle();
    h4b->SetLineColor(kBlack);h4b->SetLineWidth(2);
    h4b->SetDirectory(m_dir_main);

    h5a = new TH1D("h5a","",100,0,1.5);
    h5a->GetXaxis()->SetTitle("EEMC Cluster E / Rec. Track Momentum");h5a->GetXaxis()->CenterTitle();
    h5a->SetLineColor(kBlack);h5a->SetLineWidth(2);
    h5a->SetDirectory(m_dir_main);

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
    m_log->trace("Propagating through {} trajectories", trajectories.size());
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
        auto traj_mom = projection_point->momentum;
        auto mom_mag = sqrt( traj_mom.x*traj_mom.x + traj_mom.y*traj_mom.y + traj_mom.z*traj_mom.z );
        m_log->trace("Trajectory index, proj x, proj y, proj z, pathlength, momentum:");
        m_log->trace("   {:>10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", traj_index, pos.x, pos.y, pos.z, length, mom_mag);

        //Fill histograms
        if(num_primary==1 && num_clus==1 && num_rec==1){
            h1a->Fill(mcp,recp);
            h1b->Fill(mce,cluse);
            h2a->Fill(pos.z);
            h2b->Fill(clus_z);
            h3a->Fill(pos.x,clus_x);
            h3b->Fill(pos.y,clus_y);
            h4a->Fill(clus_x-pos.x);
            h4b->Fill(clus_y-pos.y);
            h5a->Fill(cluse/recp);
        }

    }
}


//------------------
// Finish
//------------------
void EemcTrkPropagation_processor::Finish()
{
	m_log->trace("EemcTrkPropagation_processor finished\n");

}

