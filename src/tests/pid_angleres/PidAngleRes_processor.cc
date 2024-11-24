#include <Acts/Surfaces/CylinderSurface.hpp>
#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <JANA/Services/JGlobalRootLock.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <spdlog/logger.h>
#include <stddef.h>
#include <Eigen/Geometry>
#include <exception>
#include <gsl/pointers>
#include <map>
#include <string>
#include <vector>
#include <TVector3.h>


#include "PidAngleRes_processor.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/rootfile/RootFile_service.h"

#include <edm4hep/MCParticle.h>
#include <edm4hep/SimTrackerHit.h>
#include <edm4eic/Trajectory.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/TrackParameters.h>


//------------------
// OccupancyAnalysis (Constructor)
//------------------
PidAngleRes_processor::PidAngleRes_processor(JApplication *app) :
        JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void PidAngleRes_processor::Init()
{
    std::string plugin_name=("pid_anglres");

    // Get JANA application
    auto *app = GetApplication();

    // Ask service locator a file to write histograms to
    auto root_file_service = app->GetService<RootFile_service>();

    // Get TDirectory for histograms root file
    auto globalRootLock = app->GetService<JGlobalRootLock>();
    globalRootLock->acquire_write_lock();
    auto *file = root_file_service->GetHistFile();
    globalRootLock->release_lock();

    // Create a directory for this plugin. And subdirectories for series of histograms
    m_dir_main = file->mkdir(plugin_name.c_str());
    file->cd();

    TreeTruth = new TTree("TTruth","DIRC Ang. Res.");
      TreeTruth->Branch("DIRCTruthhit",  &DIRCTruthhit,"p/D:pT:eta:theta:phi:x:y:z:r");
      TreeTruth->Branch("DIRCTruthproj", &DIRCTruthproj,"p/D:pT:eta:theta:phi:x:y:z:r");
      TreeTruth->Branch("DIRCTruthres",  &DIRCTruthres,"deta/D:dtheta:dphi:dz:dR");
      TreeTruth->Branch("DIRCTruthpoint",&DIRCTruthpoint,"theta/D:phi:err_theta:err_phi:err_thetaphi:px:py:pz:posx:posy:posz");

    TreeSeed = new TTree("TSeed","DIRC Ang. Res.");
      TreeSeed->Branch("DIRCSeedhit",  &DIRCSeedhit,"p/D:pT:eta:theta:phi:x:y:z:r");
      TreeSeed->Branch("DIRCSeedproj", &DIRCSeedproj,"p/D:pT:eta:theta:phi:x:y:z:r:dir_theta:dir_phi:dir_error_theta:dir_error_phi_dir_error_thetaphi");
      TreeSeed->Branch("DIRCSeedres",  &DIRCSeedres,"deta/D:dtheta:dphi:dz:dR");
      TreeSeed->Branch("DIRCSeedpoint",&DIRCSeedpoint,"theta/D:phi:err_theta:err_phi:err_thetaphi:px:py:pz:posx:posy:posz");

    // Get log level from user parameter or default
    InitLogger(app, plugin_name);

    auto dd4hep_service = GetApplication()->GetService<DD4hep_service>();
    auto acts_service = GetApplication()->GetService<ACTSGeo_service>();

    m_propagation_algo.init(dd4hep_service->detector(), acts_service->actsGeoProvider(), logger());

    // Create HCal surface that will be used for propagation
    auto transform = Acts::Transform3::Identity();

    // make a reference disk to mimic electron-endcap HCal
    const auto dirc_zmin   = 2730.0;
    const auto dirc_zmax   = 1850.0;
    const auto dirc_length = dirc_zmin + dirc_zmax;
    const auto dirc_r      =  755.0;
    auto dirc_trf = transform * Acts::Translation3(Acts::Vector3(0,0,0));
    m_dirc_surf   = Acts::Surface::makeShared<Acts::CylinderSurface>(dirc_trf,dirc_r,0.5*dirc_length);
}


//------------------
// Process
//------------------
// This function is called every event
void PidAngleRes_processor::Process(const std::shared_ptr<const JEvent>& event)
{
    m_log->trace("PidAngleRes_processor event");

    // Get trajectories from tracking
    auto trajectories = event->Get<ActsExamples::Trajectories>("CentralCKFTruthSeededActsTrajectories");

    // Iterate over trajectories
    m_log->debug("Propagating through {} trajectories", trajectories.size());
    for (size_t traj_index = 0; traj_index < trajectories.size(); traj_index++) {
        auto &trajectory = trajectories[traj_index];
        m_log->trace(" -- trajectory {} --", traj_index);

        std::unique_ptr<edm4eic::TrackPoint> proj_DIRC_point;
        try {
            // >>> try to propagate to surface <<<
            proj_DIRC_point = m_propagation_algo.propagate(edm4eic::Track{}, trajectory, m_dirc_surf);
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

        if(!proj_DIRC_point) {
            m_log->trace("   could not propagate!", traj_index);
	    std::cout<<" could not propagate! traj_index " << traj_index << std::endl;
            continue;
        }

        // Now go through reconstructed tracks points
	std::cout << "pid_angleres: traj_index = " << traj_index << std::endl;

       auto DIRC_proj_pos   = proj_DIRC_point->position;
       auto DIRC_proj_len   = proj_DIRC_point->pathlength;
       auto DIRC_proj_mom   = proj_DIRC_point->momentum;
       auto DIRC_proj_theta = proj_DIRC_point->theta;
       auto DIRC_proj_phi   = proj_DIRC_point->phi;
       auto DIRC_proj_theta_theta_error = proj_DIRC_point->directionError.xx;
       auto DIRC_proj_phi_phi_error     = proj_DIRC_point->directionError.yy;
       auto DIRC_proj_theta_phi_error   = proj_DIRC_point->directionError.xy;
       TVector3 proj_pos_vector(DIRC_proj_pos.x, DIRC_proj_pos.y, DIRC_proj_pos.z);
       TVector3 proj_mom_vector(DIRC_proj_mom.x, DIRC_proj_mom.y, DIRC_proj_mom.z);

       std::cout << Form("proj_mom.x: %.2f, proj_mom.y: %.2f, proj_mom.z: %.2f, proj_mom.Mag: %.2f, proj_mom.theta: %.3f, proj_mom.phi: %.3f, proj_mom.eta: %.2f \n",
                          proj_mom_vector.x(), proj_mom_vector.y(), proj_mom_vector.z(), proj_mom_vector.Mag(), proj_mom_vector.Theta(), proj_mom_vector.Phi(), proj_mom_vector.PseudoRapidity());
       std::cout << Form("\tproj_pos.Perp: %.2f, proj_pos.x: %.2f, proj_pos.y:%.2f, proj_pso.z: %.2f, proj_pos.Mag: %.2f, proj_pos.theta: %.3f, proj_pos.phi: %.3f, proj_pos.eta: %.2f \n",
                          proj_pos_vector.Perp(),proj_pos_vector.x(),proj_pos_vector.y(),proj_pos_vector.z(),proj_pos_vector.Mag(),proj_pos_vector.Theta(),proj_pos_vector.Phi(),proj_pos_vector.PseudoRapidity());

       double dR               = 10e9;
       double matchHit_p       = 10e9;
       double matchHit_pT      = 10e9;
       double matchHit_eta     = 10e9;
       double matchHit_theta   = 10e9;
       double matchHit_phi     = 10e9;
       double matchHit_x       = 10e9;
       double matchHit_y       = 10e9;
       double matchHit_z       = 10e9;
       double matchHit_r       = 10e9;

       double matchProj_p      = 10e9;
       double matchProj_pT     = 10e9;
       double matchProj_eta    = 10e9;
       double matchProj_theta  = 10e9;
       double matchProj_phi    = 10e9;
       double matchProj_x      = 10e9;
       double matchProj_y      = 10e9;
       double matchProj_z      = 10e9;
       double matchProj_r      = 10e9;

       double match_dtheta     = 10e9;
       double match_dphi       = 10e9;
       double match_deta       = 10e9;
       double match_dz         = 10e9;
       double match_dR         = dR;
    //look through hits
      for(const auto hit: event->Get<edm4hep::SimTrackerHit>("PID01RefHits")) {
        if(hit->getQuality() != 0) continue;
        auto DIRChit_pos = hit->getPosition();
        auto DIRChit_mom = hit->getMomentum();
        TVector3 hit_mom_vector(DIRChit_mom.x,DIRChit_mom.y,DIRChit_mom.z);
        TVector3 hit_pos_vector(DIRChit_pos.x,DIRChit_pos.y,DIRChit_pos.z);
        dR = TMath::Sqrt(  (proj_pos_vector.x() - hit_pos_vector.x())*(proj_pos_vector.x()-hit_pos_vector.x())
                         + (proj_pos_vector.y() - hit_pos_vector.y())*(proj_pos_vector.y()-hit_pos_vector.y())
                         + (proj_pos_vector.z() - hit_pos_vector.z())*(proj_pos_vector.z()-hit_pos_vector.z()));

        std::cout << Form("hit_mom_vector.x: %.2f, hit_mom_vector.y: %.2f, hit_mom_vector.z: %.2f, hit_mom_vector.Mag: %.2f, hit_mom_vector.theta: %.3f, hit_mom_vector.phi: %.3f, hit_mom_vector.eta: %.2f\n",
                           hit_mom_vector.x(),hit_mom_vector.y(),hit_mom_vector.z(),hit_mom_vector.Mag(),hit_mom_vector.Theta(),hit_mom_vector.Phi(),hit_mom_vector.PseudoRapidity());
        std::cout << Form("\thit_pos_vector.Perp: %.2f hit_pos_vector.x: %.2f, hit_pos_vector.y: %.2f, hit_pos_vector.z: %.2f, hit_pos_vector.Mag: %.2f, hit_pos_vector.theta: %.3f, hit_pos_vector.phi: %.3f, hit_pos_vector.eta: %.2f\n",
                           hit_pos_vector.Perp(),hit_pos_vector.x(),hit_pos_vector.y(),hit_pos_vector.z(),hit_pos_vector.Mag(),hit_pos_vector.Theta(),hit_pos_vector.Phi(),hit_pos_vector.PseudoRapidity());

        if( (dR < match_dR) && ( (hit_pos_vector.Perp() !=0) || (proj_pos_vector.Perp()!=0) ) ){
	  //information from truth reference hit
          DIRCTruthhit.p      = hit_mom_vector.Mag();
          DIRCTruthhit.pT     = hit_mom_vector.Perp();
          DIRCTruthhit.eta    = hit_mom_vector.PseudoRapidity();
          DIRCTruthhit.theta  = hit_mom_vector.Theta();
          DIRCTruthhit.phi    = hit_mom_vector.Phi();
          DIRCTruthhit.x      = hit_pos_vector.x();
          DIRCTruthhit.y      = hit_pos_vector.y();
          DIRCTruthhit.z      = hit_pos_vector.z();
          DIRCTruthhit.r      = hit_pos_vector.Perp();

          //information from propagation
          DIRCTruthproj.p     = proj_mom_vector.Mag();
          DIRCTruthproj.pT    = proj_mom_vector.Perp();
          DIRCTruthproj.eta   = proj_mom_vector.PseudoRapidity();
          DIRCTruthproj.theta = proj_mom_vector.Theta();
          DIRCTruthproj.phi   = proj_mom_vector.Phi();
          DIRCTruthproj.x     = proj_pos_vector.x();
          DIRCTruthproj.y     = proj_pos_vector.y();
          DIRCTruthproj.z     = proj_pos_vector.z();
          DIRCTruthproj.r     = proj_pos_vector.Perp();

	  //Resolutions
          DIRCTruthres.dtheta    = DIRCTruthproj.theta - DIRCTruthhit.theta;
          DIRCTruthres.dphi      = DIRCTruthproj.phi   - DIRCTruthhit.phi;
          DIRCTruthres.deta      = DIRCTruthproj.eta   - DIRCTruthhit.eta;
          DIRCTruthres.dz        = DIRCTruthproj.z     - DIRCTruthhit.z;
          DIRCTruthres.dR        = dR;

          DIRCTruthpoint.px            = DIRC_proj_mom.x;
          DIRCTruthpoint.py            = DIRC_proj_mom.y;
          DIRCTruthpoint.pz            = DIRC_proj_mom.z;
          DIRCTruthpoint.posx          = DIRC_proj_pos.x;
          DIRCTruthpoint.posy          = DIRC_proj_pos.y;
          DIRCTruthpoint.posz          = DIRC_proj_pos.z;
          DIRCTruthpoint.theta         = DIRC_proj_theta;
          DIRCTruthpoint.phi           = DIRC_proj_phi;
          DIRCTruthpoint.err_theta     = DIRC_proj_theta_theta_error;
          DIRCTruthpoint.err_phi       = DIRC_proj_phi_phi_error;
          DIRCTruthpoint.err_thetaphi  = DIRC_proj_theta_phi_error;
        }
        std::cout << "--------------------\n\n";
        std::cout<<Form("DIRCTruth seeding: matchHit_p: %.2f, matchHit_eta: %.2f, matchHit_theta: %.3f, matchHit_phi: %.3f, matchHit_r: %2f, matchHit_z: %.2f\n",
                         DIRCTruthhit.p, DIRCTruthhit.eta, DIRCTruthhit.theta, DIRCTruthhit.phi, DIRCTruthhit.r, DIRCTruthhit.z);
        std::cout<<Form("matchProj_p: %.2f, matchProj_eta: %.2f, matchProj_theta: %.3f, matchProj_phi: %.3f, matchProj_r: %2f, matchProj_z: %.2f\n",
                         DIRCTruthproj.p, DIRCTruthproj.eta, DIRCTruthproj.theta, DIRCTruthproj.phi, DIRCTruthproj.r, DIRCTruthproj.z);
        std::cout <<Form("\tdtheta: %.5f, dphi: %.5f, deta: %.5f, dz: %.5f, dR: %.5f",
                           DIRCTruthres.dtheta, DIRCTruthres.dphi,DIRCTruthres.deta, DIRCTruthres.dz, DIRCTruthres.dR);
        std::cout << "--------------------\n\n";
      }//end hit loop
    TreeTruth->Fill();
    }//end trajectory loop

//=========
//Do seeded trajectory
//
//Do the truth seeded trajectories
  double dR = 10e9;
  double match_dR = dR;
  auto seed_trajectories = event->Get<ActsExamples::Trajectories>("CentralCKFActsTrajectories");
  std::cout << Form("Propagating through %d seeded trajectories\n",seed_trajectories.size());
  int  traj_size = seed_trajectories.size();
  //auto DIRC_proj_len[traj_size];   
  double DIRC_proj_theta[traj_size];
  double DIRC_proj_phi[traj_size];
  double DIRC_proj_theta_theta_error[traj_size];
  double DIRC_proj_phi_phi_error[traj_size];
  double DIRC_proj_theta_phi_error[traj_size];
  TVector3 proj_pos_vector[traj_size];
  TVector3 proj_mom_vector[traj_size];

  for(auto traj_index = 0; traj_index < seed_trajectories.size(); traj_index++){
    auto& seed_trajectory      = seed_trajectories[traj_index];
    std::cout << Form(" -- seed trajectory {%d} --\n", traj_index);
    const auto& seed_mj        = seed_trajectory->multiTrajectory();
    const auto& seed_trackTips = seed_trajectory->tips();
    if(seed_trackTips.empty()){
         m_log->trace("Empty seed multiTrajectory");
         continue;
    }
    std::unique_ptr<edm4eic::TrackPoint> proj_DIRC_seedpoint;
     //Test propagated trajectory hits surface
     try{
       //proj_DIRC_seedpoint = m_dirc_prop_algo.propagate(edm4eic::Track{},seed_trajectory,m_dirc_surf);
       proj_DIRC_seedpoint = m_propagation_algo.propagate(edm4eic::Track{}, seed_trajectory, m_dirc_surf);
     }catch(std::exception &e) {
       throw JException(e.what());
     }
     if(!proj_DIRC_seedpoint){
       m_log->trace("   could not propagate!",traj_index);
       std::cout << Form(" traj_index: %d could not propagate!",traj_index);
       continue;
    }
       std::cout << Form("directionError.xx: %.6f rad^2, sqrt(directionError.xx): %.3f mrad\n",proj_DIRC_seedpoint->directionError.xx,TMath::Sqrt(proj_DIRC_seedpoint->directionError.xx)*1000); 
       std::cout << Form("directionError.yy: %.6f rad^2, sqrt(directionError.yy): %.3f mrad\n",proj_DIRC_seedpoint->directionError.yy,TMath::Sqrt(proj_DIRC_seedpoint->directionError.yy)*1000); 
       std::cout << Form("directionError.xy: %.6f rad^2, sqrt(ABS(directionError.xy)): %.3f mrad\n",proj_DIRC_seedpoint->directionError.xy,TMath::Sqrt(TMath::Abs(proj_DIRC_seedpoint->directionError.xy))*1000); 
       auto DIRC_proj_pos                      = proj_DIRC_seedpoint->position;
       //DIRC_proj_len[traj_index]               = proj_DIRC_seedpoint->pathlength;
       auto DIRC_proj_mom                      = proj_DIRC_seedpoint->momentum;
       DIRC_proj_theta[traj_index]             = proj_DIRC_seedpoint->theta;
       DIRC_proj_phi[traj_index]               = proj_DIRC_seedpoint->phi;
       DIRC_proj_theta_theta_error[traj_index] = proj_DIRC_seedpoint->directionError.xx;
       DIRC_proj_phi_phi_error[traj_index]     = proj_DIRC_seedpoint->directionError.yy;
       DIRC_proj_theta_phi_error[traj_index]   = proj_DIRC_seedpoint->directionError.xy;
       proj_pos_vector[traj_index].SetX(DIRC_proj_pos.x);
       proj_pos_vector[traj_index].SetY(DIRC_proj_pos.y);
       proj_pos_vector[traj_index].SetZ(DIRC_proj_pos.z);
       proj_mom_vector[traj_index].SetX(DIRC_proj_mom.x);
       proj_mom_vector[traj_index].SetY(DIRC_proj_mom.y);
       proj_mom_vector[traj_index].SetZ(DIRC_proj_mom.z);

       double matchHit_p       = 10e9;
       double matchHit_pT      = 10e9;
       double matchHit_eta     = 10e9;
       double matchHit_theta   = 10e9;
       double matchHit_phi     = 10e9;
       double matchHit_x       = 10e9;
       double matchHit_y       = 10e9;
       double matchHit_z       = 10e9;
       double matchHit_r       = 10e9;

       double matchProj_p      = 10e9;
       double matchProj_pT     = 10e9;
       double matchProj_eta    = 10e9;
       double matchProj_theta  = 10e9;
       double matchProj_phi    = 10e9;
       double matchProj_x      = 10e9;
       double matchProj_y      = 10e9;
       double matchProj_z      = 10e9;
       double matchProj_r      = 10e9;

       double match_dtheta     = 10e9;
       double match_dphi       = 10e9;
       double match_deta       = 10e9;
       double match_dz         = 10e9;
              match_dR         = dR;
    //look through hits
      for(const auto hit: event->Get<edm4hep::SimTrackerHit>("PID01RefHits")) {
        std::cout << Form("hit: %d\n",hit);
        if(hit->getQuality() != 0) continue;
        auto DIRChit_pos = hit->getPosition();
        auto DIRChit_mom = hit->getMomentum();
        TVector3 hit_mom_vector(DIRChit_mom.x,DIRChit_mom.y,DIRChit_mom.z);
        TVector3 hit_pos_vector(DIRChit_pos.x,DIRChit_pos.y,DIRChit_pos.z);
        dR = TMath::Sqrt(  (proj_pos_vector[traj_index].x() - hit_pos_vector.x())*(proj_pos_vector[traj_index].x()-hit_pos_vector.x())
                         + (proj_pos_vector[traj_index].y() - hit_pos_vector.y())*(proj_pos_vector[traj_index].y()-hit_pos_vector.y())
                         + (proj_pos_vector[traj_index].z() - hit_pos_vector.z())*(proj_pos_vector[traj_index].z()-hit_pos_vector.z()));

        std::cout<<Form("Traj: %d, hit: %d, dR: %.5f, match_dR: %.5f\n",traj_index,hit,dR,match_dR);
        if( (dR < match_dR) && ( (hit_pos_vector.Perp() !=0) || (proj_pos_vector[traj_index].Perp()!=0) ) ){
          DIRCSeedhit.p      = hit_mom_vector.Mag();
          DIRCSeedhit.pT     = hit_mom_vector.Perp();
          DIRCSeedhit.eta    = hit_mom_vector.PseudoRapidity();
          DIRCSeedhit.theta  = hit_mom_vector.Theta();
          DIRCSeedhit.phi    = hit_mom_vector.Phi();
          DIRCSeedhit.x      = hit_pos_vector.x();
          DIRCSeedhit.y      = hit_pos_vector.y();
          DIRCSeedhit.z      = hit_pos_vector.z();
          DIRCSeedhit.r      = hit_pos_vector.Perp();

          DIRCSeedproj.p     = proj_mom_vector[traj_index].Mag();
          DIRCSeedproj.pT    = proj_mom_vector[traj_index].Perp();
          DIRCSeedproj.eta   = proj_mom_vector[traj_index].PseudoRapidity();
          DIRCSeedproj.theta = proj_mom_vector[traj_index].Theta();
          DIRCSeedproj.phi   = proj_mom_vector[traj_index].Phi();
          DIRCSeedproj.x     = proj_pos_vector[traj_index].x();
          DIRCSeedproj.y     = proj_pos_vector[traj_index].y();
          DIRCSeedproj.z     = proj_pos_vector[traj_index].z();
          DIRCSeedproj.r     = proj_pos_vector[traj_index].Perp();

          DIRCSeedres.dtheta    = DIRCSeedproj.theta - DIRCSeedhit.theta;
          DIRCSeedres.dphi      = DIRCSeedproj.phi   - DIRCSeedhit.phi;
          DIRCSeedres.deta      = DIRCSeedproj.eta   - DIRCSeedhit.eta;
          DIRCSeedres.dz        = DIRCSeedproj.z     - DIRCSeedhit.z;
          DIRCSeedres.dR        = dR;

          DIRCSeedpoint.theta            = DIRC_proj_theta[traj_index];
          DIRCSeedpoint.phi              = DIRC_proj_phi[traj_index];
          DIRCSeedpoint.err_theta        = DIRC_proj_theta_theta_error[traj_index];
          DIRCSeedpoint.err_phi          = DIRC_proj_phi_phi_error[traj_index];
          DIRCSeedpoint.err_thetaphi     = DIRC_proj_theta_phi_error[traj_index];
          DIRCSeedpoint.px               = DIRC_proj_mom.x;
          DIRCSeedpoint.py               = DIRC_proj_mom.y;
          DIRCSeedpoint.pz               = DIRC_proj_mom.z;
          DIRCSeedpoint.posx             = DIRC_proj_pos.x;
          DIRCSeedpoint.posy             = DIRC_proj_pos.y;
          DIRCSeedpoint.posz             = DIRC_proj_pos.z;
	  match_dR = dR;
       }
      }//end hit loop   
    std::cout<<Form("dR: %.5f, match_dR: %.5f\n",dR, match_dR);       
    std::cout<<Form("Theta Res: %.3f mrad, Phi Res: %.3f\n",DIRCSeedres.dtheta*1000,DIRCSeedres.dphi*1000);       

  }//end seeded trajectory loop
    std::cout<<Form("Post traj: dR: %.5f, match_dR: %.5f\n",dR, match_dR);
    std::cout<<Form("Post Theta Res: %.3f mrad, Phi Res: %.3f\n",DIRCSeedres.dtheta*1000,DIRCSeedres.dphi*1000);       
    std::cout<<Form("Cov. Matrix: TMath::Sqrt(direction.xx) [mrad] =  %.5f\n",TMath::Sqrt(DIRCSeedpoint.err_theta)*1000.0);
    std::cout<<Form("Cov. Matrix: TMath::Sqrt(direction.yy) [mrad] =  %.5f\n",TMath::Sqrt(DIRCSeedpoint.err_phi)*1000.0);
    std::cout<<Form("Cov. Matrix: TMath::Sqrt(TMath::Abs(direction.xy)) [mrad] =  %.5f\n",TMath::Sqrt(TMath::Abs(DIRCSeedpoint.err_thetaphi))*1000.0);
    TreeSeed->Fill();   
}
//------------------
// Finish
//------------------
void PidAngleRes_processor::Finish()
{
//    m_log->trace("PidAngleRes_processor finished\n");

}
