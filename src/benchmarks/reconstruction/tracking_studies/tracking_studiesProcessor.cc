#include "tracking_studiesProcessor.h"
#include "algorithms/tracking/JugTrack/TrackingResultTrajectory.hpp"
#include "edm4eic/vector_utils.h"
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>
#include <TVector3.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <services/rootfile/RootFile_service.h>
#include <spdlog/spdlog.h>

#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <services/log/Log_service.h>
#include <spdlog/fmt/ostr.h>

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
// #include <extensions/spdlog/SpdlogMixin.h>

// The following just makes this a JANA plugin
extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  app->Add(new tracking_studiesProcessor());
}
}

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void tracking_studiesProcessor::InitWithGlobalRootLock() {
  std::string plugin_name = ("tracking_studies");

  // InitLogger(plugin_name);

  // InitLogger(plugin_name);
  // Get JANA application
  auto app          = GetApplication();
  auto acts_service = GetApplication()->GetService<ACTSGeo_service>();

  std::string log_level_str = "off";//"debug";
  m_log                     = app->GetService<Log_service>()->logger(plugin_name);
  app->SetDefaultParameter(plugin_name + ":LogLevel", log_level_str,
                           "LogLevel: trace, debug, info, warn, err, critical, off");
  m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

  m_geo_provider = acts_service->actsGeoProvider();
  m_propagation_algo.init(acts_service->actsGeoProvider(), m_log);

  m_geoSvc = GetApplication()->GetService<JDD4hep_service>();
  if (!m_geoSvc) {
    std::cout << "No JDD4hep_service found" << std::endl;
    throw std::runtime_error("No JDD4hep_service found");
  }

  // Ask service locator a file to write histograms to
  auto root_file_service = app->GetService<RootFile_service>();

  // Get TDirectory for histograms root file
  auto globalRootLock = app->GetService<JGlobalRootLock>();
  globalRootLock->acquire_write_lock();
  auto file = root_file_service->GetHistFile();
  globalRootLock->release_lock();

  // Create a directory for this plugin. And subdirectories for series of histograms
  m_dir_main = file->mkdir(plugin_name.c_str());

  // Create histograms here. e.g.
  hPosTestRZ = new TH2D("hPosTestRZ", "", 500, -200, 300, 100, 0, 100);
  hPosTestRZ->SetDirectory(m_dir_main);
  hMindRMatched = new TH2D("hMindRMatched", "", 300, 0, 30, 5, -0.5, 4.5);
  hMindRMatched->SetDirectory(m_dir_main);

  hThetaResoVsEta = new TH2D("hThetaResoVsEta", "", 400, -4, 4, 1000, -0.1, 0.1);
  hThetaResoVsP   = new TH2D("hThetaResoVsP", "", 400, 0, 20, 1000, -0.1, 0.1);
  hPhiResoVsEta   = new TH2D("hPhiResoVsEta", "", 400, -4, 4, 1000, -0.1, 0.1);
  hPhiResoVsP     = new TH2D("hPhiResoVsP", "", 400, 0, 20, 1000, -0.1, 0.1);
  hThetaResoVsEta->SetDirectory(m_dir_main);
  hThetaResoVsP->SetDirectory(m_dir_main);
  hPhiResoVsEta->SetDirectory(m_dir_main);
  hPhiResoVsP->SetDirectory(m_dir_main);

  hzPosResoVsPhi = new TH2D("hzPosResoVsPhi", "", 360, -M_PI, M_PI, 500, -20.0, 20.0);
  hzPosResoVsEta = new TH2D("hzPosResoVsEta", "", 400, -4, 4, 500, -20.0, 20.0);
  hzPosAbsResoVsEta = new TH2D("hzPosAbsResoVsEta", "", 400, -4, 4, 500, -20.0, 20.0);
  hzPosResoVsP   = new TH2D("hzPosResoVsP", "", 400, 0, 20, 500, -20.0, 20.0);
  hzPosResoVsPhi->SetDirectory(m_dir_main);
  hzPosResoVsEta->SetDirectory(m_dir_main);
  hzPosAbsResoVsEta->SetDirectory(m_dir_main);
  hzPosResoVsP->SetDirectory(m_dir_main);

  // nHitsTrackVsEtaVsP =
  //     new TH3D("nHitsTrackVsEtaVsP", "", 100, -4., 4., 20, -0.5, 19.5, 150, 0., 15.);
  // nHitsEventVsEtaVsP =
  //     new TH3D("nHitsEventVsEtaVsP", "", 100, -4., 4., 20, -0.5, 19.5, 150, 0., 15.);
  // nHitsTrackVsEtaVsP->SetDirectory(m_dir_main);
  // nHitsEventVsEtaVsP->SetDirectory(m_dir_main);

  hzPosResoVsEtaVsP = new TH3D("hzPosResoVsEtaVsP", "", 400, -4, 4, 500, -20.0, 20.0, 400, 0, 20);
  hThetaResoVsEtaVsP = new TH3D("hThetaResoVsEtaVsP", "", 400, -4, 4, 500, -0.1, 0.1, 400, 0, 20);
  hPhiResoVsEtaVsP   = new TH3D("hPhiResoVsEtaVsP", "", 400, -4, 4, 500, -0.1, 0.1, 400, 0, 20);
  hzPosResoVsEtaVsP->SetDirectory(m_dir_main);
  hThetaResoVsEtaVsP->SetDirectory(m_dir_main);
  hPhiResoVsEtaVsP->SetDirectory(m_dir_main);

  auto transform = Acts::Transform3::Identity();

  // Create propagation surfaces for cherenkov angular studies
  // make a reference disk to mimic center of PFRICH
  const auto PFRICH_center_Z    = -1236. - 20.; // center of aerogel
  const auto PFRICH_center_MinR = 80.0;
  const auto PFRICH_center_MaxR = 630;
  auto PFRICH_center_Bounds =
      std::make_shared<Acts::RadialBounds>(PFRICH_center_MinR, PFRICH_center_MaxR);
  auto PFRICH_center_Trf = transform * Acts::Translation3(Acts::Vector3(0, 0, PFRICH_center_Z));
  m_PFRICH_center_surface =
      Acts::Surface::makeShared<Acts::DiscSurface>(PFRICH_center_Trf, PFRICH_center_Bounds);

  // make a reference disk to mimic center of dRICH
  const auto dRICH_center_Z = 1970; //1815. /*directly behind tof*/; // entrance of detector
  // const auto dRICH_center_Z    = 1950. + 60.; // center of detector
  const auto dRICH_center_MinR = 84.0;
  const auto dRICH_center_MaxR = 1100. + (1212.82 - 1100.) * (60. / 120.);
  auto dRICH_center_Bounds =
      std::make_shared<Acts::RadialBounds>(dRICH_center_MinR, dRICH_center_MaxR);
  auto dRICH_center_Trf = transform * Acts::Translation3(Acts::Vector3(0, 0, dRICH_center_Z));
  m_dRICH_center_surface =
      Acts::Surface::makeShared<Acts::DiscSurface>(dRICH_center_Trf, dRICH_center_Bounds);

  // make a reference cylinder to mimic center of DIRC
  const auto DIRC_R     = 700.0+8.5; // minr dirc + half height of bar box
  const auto DIRC_halfz = 2900;//1700.;
  // auto DIRC_center_Bounds =
  // std::make_shared<Acts::RadialBounds>(DIRC_center_MinR, DIRC_center_MaxR);
  auto DIRC_center_Trf = transform * Acts::Translation3(Acts::Vector3(0, 0, 0));
  m_DIRC_center_surface =
      Acts::Surface::makeShared<Acts::CylinderSurface>(DIRC_center_Trf, DIRC_R, DIRC_halfz);

  // Acts::Surface::makeShared<Acts::CylinderSurface>(
  //         Acts::Transform3::Identity(), radius, halfz);

  // //DIRC
  // zmin = 0;
  // zmax = 0;
  // rmin = 70; //max=+3cm
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void tracking_studiesProcessor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {
  using namespace std;
  int verbose = 0;
  nEventsFilled++;
  // cout << "tracking_studiesProcessor::ProcessSequential" << endl;
  double mceta = 0;
  double mcphi = 0;
  double mcp   = 0;
  for (auto mcparticle : mcParticles()) {
    if (mcparticle->getGeneratorStatus() != 1)
      continue;
    auto& mom = mcparticle->getMomentum();
    // cout << "MC particle: " << mom.x << " " << mom.y << " " << mom.z << "\ttotmom: " <<
    // sqrt(mom.x*mom.x+mom.y*mom.y+mom.z*mom.z) << endl; determine mceta from momentum
    mceta = -log(tan(atan2(sqrt(mom.x * mom.x + mom.y * mom.y), mom.z) / 2.));
    // determine mcphi from momentum
    mcphi = atan2(mom.y, mom.x);
    // determine mc momentum
    mcp = sqrt(mom.x * mom.x + mom.y * mom.y + mom.z * mom.z);
  }

  int iseg = 0;
  for (auto track_segment : trackSegments()) {
    // cout << "\t\ttrksegment " << iseg << endl;
    int ipnt = 0;
    for (auto point : track_segment->getPoints()) {
      auto& pos   = point.position;
      auto& mom   = point.momentum;
      auto& theta = point.theta;
      auto& phi   = point.phi;
      // cout << "\t\ttrkpoints: " << ipnt << endl;
      // cout << "\t\t\t point: radius: " << sqrt(pos.x*pos.x+pos.y*pos.y) << " z: " << pos.z <<
      // endl;
      // // cout << "\t\t\t momentum: " << mom.x << " " << mom.y << " " << mom.z << endl;
      // cout << "\t\t\t theta: " << theta << "\t phi: " << phi << endl;
      // cout << "\t\t\t calculated phi from momentum: " << atan2(mom.y, mom.x) << endl;
      // cout << "\t\t\t calculated theta from momentum: " <<
      // acos(mom.z/sqrt(mom.x*mom.x+mom.y*mom.y+mom.z*mom.z)) << endl; cout << "\t\t\t calculated
      // phi from position: " << atan2(pos.y, pos.x) << endl; cout << "\t\t\t calculated theta from
      // position: " << acos(pos.z/sqrt(pos.x*pos.x+pos.y*pos.y+pos.z*pos.z)) << endl;
      // hPosTestX->Fill(  pos.x);
      ipnt++;
    }
    iseg++;
  }

  auto acts_results = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories");
  m_log->debug("ACTS Trajectories( size: {} )", std::size(acts_results));
  // m_log->debug("{:>10} {:>10}  {:>10} {:>10} {:>10} {:>10} {:>12} {:>12} {:>12} {:>8}", "[loc 0]",
  //              "[loc 1]", "[phi]", "[theta]", "[q/p]", "[p]", "[err phi]", "[err th]", "[err q/p]",
  //              "[chi2]");
  // Loop over the trajectories
  for (const auto& traj : acts_results) {

    // Get the entry index for the single trajectory
    // The trajectory entry indices and the multiTrajectory
    const auto& mj        = traj->multiTrajectory();
    const auto& trackTips = traj->tips();
    if (trackTips.empty()) {
      // m_log->debug("Empty multiTrajectory.");
      continue;
    }

    // projection to special surfaces
    if(abs(mceta)<1.6){
      std::unique_ptr<edm4eic::TrackPoint> projection_point_DIRC;
      try {
        // >>> try to propagate to surface <<<
        projection_point_DIRC = m_propagation_algo.propagate(traj, m_DIRC_center_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
        // cout << "Exception in underlying algorithm: " << e.what() << ". Trajectory is
        // skipped" << endl;
      }

      if (!projection_point_DIRC) {
        m_log->trace("   could not propagate to DIRC!");
        if (verbose)
          cout << "   could not propagate to DIRC!" << endl;
        // continue;
      } else {

        auto proj_pos    = projection_point_DIRC->position;
        auto proj_length = projection_point_DIRC->pathlength;
        auto proj_mom    = projection_point_DIRC->momentum;
        TVector3 proj_mom_vec(proj_mom.x, proj_mom.y, proj_mom.z);
        // TVector3 proj_pos_vec(proj_pos.x, proj_pos.y, proj_pos.z);
        // m_log->trace("   {:>10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", traj_index, pos.x,
        // pos.y, pos.z, length);
        if (verbose)
          cout << "\tDIRC_projection pos: " << proj_pos.x << " " << proj_pos.y << " " << proj_pos.z
               << " " << proj_length << endl;
        // cout << "\t\t mom: " << proj_mom.x << " " << proj_mom.y << " " << proj_mom.z << endl;

        // loop over dRICH hits
        bool hit_found = false;
        double mindR   = 1000;
        TVector3 hit_pos_closest;
        TVector3 hit_mom_closest;

        for (const auto hit : event->Get<edm4hep::SimTrackerHit>("DIRCBarHits")) {

          auto hit_pos = hit->getPosition();
          auto hit_mom = hit->getMomentum();
          if (verbose)
            cout << "\tDIRC_hit pos: " << hit_pos.x << " " << hit_pos.y << " " << hit_pos.z << endl;
          // cout << "\t\t mom: " << hit_mom.x << " " << hit_mom.y << " " << hit_mom.z << endl;
          hit_found     = true;
          double deltaR = sqrt((hit_pos.x - proj_pos.x) * (hit_pos.x - proj_pos.x) +
                               (hit_pos.y - proj_pos.y) * (hit_pos.y - proj_pos.y) +
                               (hit_pos.z - proj_pos.z) * (hit_pos.z - proj_pos.z));
          if (deltaR < mindR) {
            mindR           = deltaR;
            hit_pos_closest = TVector3(hit_pos.x, hit_pos.y, hit_pos.z);
            hit_mom_closest = TVector3(hit_mom.x, hit_mom.y, hit_mom.z);
          }
        }
        hMindRMatched->Fill(mindR,1);

        if (hit_found) {
          hThetaResoVsEtaVsP->Fill(mceta, proj_mom_vec.Theta() - hit_mom_closest.Theta(), mcp);
          hThetaResoVsEta->Fill(mceta, proj_mom_vec.Theta() - hit_mom_closest.Theta());
          hThetaResoVsP->Fill(mcp, proj_mom_vec.Theta() - hit_mom_closest.Theta());
          
          hzPosResoVsEtaVsP->Fill(mceta, proj_pos.z - hit_pos_closest.z(), mcp);
          hzPosResoVsEta->Fill(mceta, proj_pos.z - hit_pos_closest.z());
          hzPosResoVsPhi->Fill(mcphi, proj_pos.z - hit_pos_closest.z());
          hzPosAbsResoVsEta->Fill(mceta, abs(proj_pos.z) - abs(hit_pos_closest.z()));
          hzPosResoVsP->Fill(mcp, proj_pos.z - hit_pos_closest.z());

          hPhiResoVsEtaVsP->Fill(mceta, proj_mom_vec.Phi() - hit_mom_closest.Phi(), mcp);
          hPhiResoVsEta->Fill(mceta, proj_mom_vec.Phi() - hit_mom_closest.Phi());
          hPhiResoVsP->Fill(mcp, proj_mom_vec.Phi() - hit_mom_closest.Phi());
        }
      }
    }
    if(mceta>1.0){
      std::unique_ptr<edm4eic::TrackPoint> projection_point_dRICH;
      try {
        // >>> try to propagate to surface <<<
        projection_point_dRICH = m_propagation_algo.propagate(traj, m_dRICH_center_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
        // cout << "Exception in underlying algorithm: " << e.what() << ". Trajectory is skipped" <<
        // endl;
      }

      if (!projection_point_dRICH) {
        m_log->trace("   could not propagate to dRICH!");
        if (verbose)
          cout << "   could not propagate to dRICH!" << endl;
        // continue;
      } else {
        // Now go through reconstructed tracks points

        auto proj_pos    = projection_point_dRICH->position;
        auto proj_length = projection_point_dRICH->pathlength;
        auto proj_mom    = projection_point_dRICH->momentum;
        TVector3 proj_mom_vec(proj_mom.x, proj_mom.y, proj_mom.z);
        // TVector3 proj_pos_vec(proj_pos.x, proj_pos.y, proj_pos.z);
        // m_log->trace("   {:>10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", traj_index, pos.x,
        // pos.y, pos.z, length);
        if (verbose)
          cout << "\tdRICH_projection pos: " << proj_pos.x << " " << proj_pos.y << " " << proj_pos.z
               << " " << proj_length << endl;
        // cout << "\t\t mom: " << proj_mom.x << " " << proj_mom.y << " " << proj_mom.z << endl;

        // loop over dRICH hits
        bool hit_found = false;
        double mindR   = 1000;
        TVector3 hit_pos_closest;
        TVector3 hit_mom_closest;

        for (const auto hit : event->Get<edm4hep::SimTrackerHit>("DRICHHits")) {

          auto hit_pos = hit->getPosition();
          auto hit_mom = hit->getMomentum();
          if (verbose)
            cout << "\tdRICH_hit pos: " << hit_pos.x << " " << hit_pos.y << " " << hit_pos.z
                 << endl;
          // cout << "\t\t mom: " << hit_mom.x << " " << hit_mom.y << " " << hit_mom.z << endl;
          hit_found     = true;
          double deltaR = sqrt((hit_pos.x - proj_pos.x) * (hit_pos.x - proj_pos.x) +
                               (hit_pos.y - proj_pos.y) * (hit_pos.y - proj_pos.y) +
                               (hit_pos.z - proj_pos.z) * (hit_pos.z - proj_pos.z));
          if (deltaR < mindR) {
            mindR           = deltaR;
            hit_pos_closest = TVector3(hit_pos.x, hit_pos.y, hit_pos.z);
            hit_mom_closest = TVector3(hit_mom.x, hit_mom.y, hit_mom.z);
          }
        }
        hMindRMatched->Fill(mindR,2);

        if (hit_found) {
          hThetaResoVsEtaVsP->Fill(mceta, proj_mom_vec.Theta() - hit_mom_closest.Theta(), mcp);
          hThetaResoVsEta->Fill(mceta, proj_mom_vec.Theta() - hit_mom_closest.Theta());
          hThetaResoVsP->Fill(mcp, proj_mom_vec.Theta() - hit_mom_closest.Theta());
          
          // hzPosResoVsEtaVsP->Fill(mceta, proj_pos.z - hit_pos_closest.z(), mcp);
          // hzPosResoVsEta->Fill(mceta, proj_pos.z - hit_pos_closest.z());
          // hzPosResoVsP->Fill(mcp, proj_pos.z - hit_pos_closest.z());

          hPhiResoVsEtaVsP->Fill(mceta, proj_mom_vec.Phi() - hit_mom_closest.Phi(), mcp);
          hPhiResoVsEta->Fill(mceta, proj_mom_vec.Phi() - hit_mom_closest.Phi());
          hPhiResoVsP->Fill(mcp, proj_mom_vec.Phi() - hit_mom_closest.Phi());
        }
      }
    }
    if(mceta<-1.0){
      std::unique_ptr<edm4eic::TrackPoint> projection_point_PFRICH;
      try {
        // >>> try to propagate to surface <<<
        projection_point_PFRICH = m_propagation_algo.propagate(traj, m_PFRICH_center_surface);
      } catch (std::exception& e) {
        m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
        // cout << "Exception in underlying algorithm: " << e.what() << ". Trajectory is skipped" <<
        // endl;
      }

      if (!projection_point_PFRICH) {
        m_log->trace("   could not propagate to PFRICH!");
        if (verbose)
          cout << "   could not propagate to PFRICH!" << endl;
        // continue;
      } else {
        // Now go through reconstructed tracks points

        auto proj_pos    = projection_point_PFRICH->position;
        auto proj_length = projection_point_PFRICH->pathlength;
        auto proj_mom    = projection_point_PFRICH->momentum;
        TVector3 proj_mom_vec(proj_mom.x, proj_mom.y, proj_mom.z);
        // TVector3 proj_pos_vec(proj_pos.x, proj_pos.y, proj_pos.z);
        // m_log->trace("   {:>10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", traj_index, pos.x,
        // pos.y, pos.z, length);
        if (verbose)
          cout << "\tPFRICH_projection pos: " << proj_pos.x << " " << proj_pos.y << " " << proj_pos.z
               << " " << proj_length << endl;
        // cout << "\t\t mom: " << proj_mom.x << " " << proj_mom.y << " " << proj_mom.z << endl;

        // loop over PFRICH hits
        bool hit_found = false;
        double mindR   = 1000;
        TVector3 hit_pos_closest;
        TVector3 hit_mom_closest;

        for (const auto hit : event->Get<edm4hep::SimTrackerHit>("PFRICHHits")) {

          auto hit_pos = hit->getPosition();
          auto hit_mom = hit->getMomentum();
          if (verbose)
            cout << "\tPFRICH_hit pos: " << hit_pos.x << " " << hit_pos.y << " " << hit_pos.z
                 << endl;
          // cout << "\t\t mom: " << hit_mom.x << " " << hit_mom.y << " " << hit_mom.z << endl;
          hit_found     = true;
          double deltaR = sqrt((hit_pos.x - proj_pos.x) * (hit_pos.x - proj_pos.x) +
                               (hit_pos.y - proj_pos.y) * (hit_pos.y - proj_pos.y) +
                               (hit_pos.z - proj_pos.z) * (hit_pos.z - proj_pos.z));
          if (deltaR < mindR) {
            mindR           = deltaR;
            hit_pos_closest = TVector3(hit_pos.x, hit_pos.y, hit_pos.z);
            hit_mom_closest = TVector3(hit_mom.x, hit_mom.y, hit_mom.z);
          }
        }
        hMindRMatched->Fill(mindR,0);

        if (hit_found) {
          hThetaResoVsEtaVsP->Fill(mceta, proj_mom_vec.Theta() - hit_mom_closest.Theta(), mcp);
          hThetaResoVsEta->Fill(mceta, proj_mom_vec.Theta() - hit_mom_closest.Theta());
          hThetaResoVsP->Fill(mcp, proj_mom_vec.Theta() - hit_mom_closest.Theta());
          
          // hzPosResoVsEtaVsP->Fill(mceta, proj_pos.z - hit_pos_closest.z(), mcp);
          // hzPosResoVsEta->Fill(mceta, proj_pos.z - hit_pos_closest.z());
          // hzPosResoVsP->Fill(mcp, proj_pos.z - hit_pos_closest.z());

          hPhiResoVsEtaVsP->Fill(mceta, proj_mom_vec.Phi() - hit_mom_closest.Phi(), mcp);
          hPhiResoVsEta->Fill(mceta, proj_mom_vec.Phi() - hit_mom_closest.Phi());
          hPhiResoVsP->Fill(mcp, proj_mom_vec.Phi() - hit_mom_closest.Phi());
        }
      }
    }

    const auto& trackTip = trackTips.front();

    // Collect the trajectory summary info
    // auto trajState = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
    // auto trajState       = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
    // if (traj->hasTrackParameters(trackTip)) {
    //   const auto& boundParam = traj->trackParameters(trackTip);
    //   const auto& parameter  = boundParam.parameters();
    //   const auto& covariance = *boundParam.covariance();
    //   // cout << "track parameters: " << parameter << endl;
    //   // cout << "\tphi: " << parameter[Acts::eBoundPhi] << endl;
    //   // cout << "\teta: " << parameter[Acts::eBoundTheta] << endl;
    //   int m_nMeasurements = trajState.nMeasurements;
    //   // TODO also add holes to count -> should agree with number of hits per event
    //   // TODO ahh I made one mistake, # of outliers  + # of measurements should sum to # of hits
    //   // TODO also check chi2
    //   //  cout << "\tnMeasurements: " << m_nMeasurements << endl;

    //   // nHitsTrackVsEtaVsP->Fill(mceta, m_nMeasurements, mcp);
    //   nTracksFilled++;
    //   // m_log->debug("{:>10.2f} {:>10.2f}  {:>10.2f} {:>10.3f} {:>10.4f} {:>10.3f} {:>12.4e}
    //   // {:>12.4e} {:>12.4e} {:>8.2f}",
    //   //     parameter[Acts::eBoundLoc0],
    //   //     parameter[Acts::eBoundLoc1],
    //   //     parameter[Acts::eBoundPhi],
    //   //     parameter[Acts::eBoundTheta],
    //   //     parameter[Acts::eBoundQOverP],
    //   //     1.0 / parameter[Acts::eBoundQOverP],
    //   //     sqrt(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
    //   //     sqrt(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
    //   //     sqrt(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)),
    //   //     trajState.chi2Sum);
    // }
    bool trackstateVerbosity = false;
    if (trackstateVerbosity)
      cout << "\n\nnew event" << endl;
    // // visit the track points
    mj.visitBackwards(trackTip, [&](auto&& trackstate) {
      // get volume info
      auto geoID  = trackstate.referenceSurface().geometryId();
      auto volume = geoID.volume();
      auto layer  = geoID.layer();
      auto module = geoID.sensitive();
      if (trackstateVerbosity)
        cout << "surfname: " << trackstate.referenceSurface().name() << endl;
      auto detEl              = trackstate.referenceSurface().associatedDetectorElement();
      const auto* det_element = dynamic_cast<const Acts::DD4hepDetectorElement*>(detEl);
      // cout << "detEl: " << det_element->identifier() << endl;

      auto volman = m_geoSvc->detector()->volumeManager();

      if (det_element == nullptr) {
        if (trackstateVerbosity)
          cout << "\tdet_element is null" << endl;
      } else {
        auto* vol_ctx = volman.lookupContext(det_element->identifier());
        if (vol_ctx == nullptr) {
          if (trackstateVerbosity)
            cout << "\tvol_ctx is null" << endl;
        } else {
          auto de = vol_ctx->element;
          if (trackstateVerbosity)
            cout << "\t" << de.path() << endl;
          if (trackstateVerbosity)
            cout << "\t" << de.placementPath() << endl;
        }
      }
      // m_init_log->debug("  de.path          = {}", de.path());
      // m_init_log->debug("  de.placementPath = {}", de.placementPath());
      // print detEl name
      // cout << "volume: " << volume << endl;
      // auto volName = geoID.volume().volumeName();
      // .volume()
      // auto referenceSurface = trackstate.referenceSurface();
      // cout << "volName: " << volName << endl;

      // convert geoID to bitfield and cout
      // Acts::GeometryIdentifier geoID_bitfield(geoID);
      // cout << "geoID: " << geoID_bitfield << endl;
      // if (trackstate.hasCalibrated()) {
      //     m_nCalibrated++;
      // }

      // get track state parameters and their covariances
      const auto& parameter = trackstate.predicted();
      // cout << "track parameters: " << parameter << endl;
      const auto& covariance = trackstate.predictedCovariance();

      // convert local to global
      auto global = trackstate.referenceSurface().localToGlobal(
          m_geo_provider->getActsGeometryContext(),
          {parameter[Acts::eBoundLoc0], parameter[Acts::eBoundLoc1]}, {0, 0, 0});
      // global position
      const decltype(edm4eic::TrackPoint::position) position{static_cast<float>(global.x()),
                                                             static_cast<float>(global.y()),
                                                             static_cast<float>(global.z())};
      // if(position.z/10>160.)
      if (trackstateVerbosity)
        cout << "\t\ttrack point: " << position << endl;
      hPosTestRZ->Fill(position.z / 10.,
                       sqrt(position.x * position.x + position.y * position.y) / 10.);
      const decltype(edm4eic::TrackPoint::momentum) momentum = edm4eic::sphericalToVector(
          static_cast<float>(1.0 / std::abs(parameter[Acts::eBoundQOverP])),
          static_cast<float>(parameter[Acts::eBoundTheta]),
          static_cast<float>(parameter[Acts::eBoundPhi]));
      const float theta(parameter[Acts::eBoundTheta]);
      const float phi(parameter[Acts::eBoundPhi]);
      // cout << "\tmomentum : " << momentum << "\t total: " << sqrt(momentum.x*momentum.x +
      // momentum.y*momentum.y + momentum.z*momentum.z) << endl;
    });
  }

  // std::vector<std::string> m_data_names = {
  //     "SiBarrelHits",      // Barrel Tracker
  //     "MPGDBarrelHits",    // MPGD
  //     "VertexBarrelHits",  // Vertex
  //     "TrackerEndcapHits", // End Cap tracker
  //     "TOFEndcapHits",     // End Cap TOF
  //     "TOFBarrelHits",     // Barrel TOF
  //     // "DRICHHits",        // Barrel TOF
  //     // "MPGDDIRCHits", // Barrel TOF
  //     // "DIRCBarHits"
  //     // ,        // Barrel TOF
  //     // "PFRICHHits",        // Barrel TOF
  // };

  // int nHitsTrackers = 0;
  // for (size_t name_index = 0; name_index < m_data_names.size(); name_index++) {
  //   string data_name = m_data_names[name_index];
  //   auto hits        = event->Get<edm4hep::SimTrackerHit>(data_name);
  //   if (hits.size() == 0) {
  //     // cout << "no hits for " << data_name << endl;
  //     continue;
  //   }
  //   // cout << "Data from: " << data_name << " has " << hits.size() << " hits" << endl;
  //   // count_hist->Fill(hits.size());
  //   for (auto hit : hits) {
  //     float x   = hit->getPosition().x;
  //     float y   = hit->getPosition().y;
  //     float z   = hit->getPosition().z;
  //     auto& mom = hit->getMomentum();
  //     // cout << "Hit: " << x << " " << y << " " << z << " " << mom.x << " " << mom.y << " " <<
  //     // mom.z << endl; cout << "\teta(pos): " << 2*atan(exp(-1*z/sqrt(x*x+y*y))) << "\tphi(pos): "
  //     // << atan2(y,x) << endl; cout << "\teta(mom): " <<
  //     // 2*atan(exp(-1*mom.z/sqrt(mom.x*mom.x+mom.y*mom.y))) << "\tphi(mom): " << atan2(mom.y,
  //     // mom.x) << endl;
  //     float r      = sqrt(x * x + y * y);
  //     float etahit = -log(tan(r / z / 2));
  //     nHitsTrackers++;
  //   }
  // }
  // nHitsEventVsEtaVsP->Fill(mceta, nHitsTrackers, mcp);
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void tracking_studiesProcessor::FinishWithGlobalRootLock() {

  // Do any final calculations here.
  // nHitsTrackVsEtaVsP->Scale(1.0/nTracksFilled);
  // nHitsEventVsEtaVsP->Scale(1.0/nEventsFilled);
}

/* CODE GRAVEYARD!
        // mj.visitBackwards(trackTip, [&](const auto &state)
        // {
        //                 std::cout << __LINE__ << std::endl;

        //     /// Only fill the track states with non-outlier measurement
        //     const auto typeFlags = state.typeFlags();
        //     if( !typeFlags.test(Acts::TrackStateFlag::MeasurementFlag) )
        //     { return true; }
        //                 std::cout << __LINE__ << std::endl;

        //     // only fill for state vectors with proper smoothed parameters
        //     if( !state.hasSmoothed()) return true;
        //                 std::cout << __LINE__ << std::endl;

        //     // get smoothed fitted parameters
        //     const Acts::BoundTrackParameters params(state.referenceSurface().getSharedPtr(),
        //         state.smoothed(),
        //         state.smoothedCovariance());

        //     // now you can access the track state parameters with e.g. the following:
        //     // position
        //     /// this is an eigen 3 vector
        //     std::cout << __LINE__ << std::endl;
        //     // const auto global = params.position(getActsGeometryContext());
        //                 std::cout << __LINE__ << std::endl;

        //     // momentum, also eigen 3 vector
        //     const auto momentum = params.momentum();
        //     cout << "momentum magnitude: " << momentum.norm() << endl;
        // }); // make sure you don't forget to close the lambda function and also close the
   visitBackwards function call
*/

// auto diskbounds    = std::make_shared<Acts::RadialBounds>(minR, maxR);
// auto disktransform = transform * Acts::Translation3(Acts::Vector3(0, 0, posZ));
// std::shared_ptr<Acts::DiscSurface> disk_surface =
//     Acts::Surface::makeShared<Acts::DiscSurface>(disktransform, diskbounds);

// auto acts_results = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories");
// // loop over trajectories
// for (const auto& traj : acts_results) {
//   std::unique_ptr<edm4eic::TrackPoint> projection_point_disk;
//   // try projection to disk surface
//   try {
//     projection_point_disk = m_propagation_algo.propagate(traj, disk_surface);
//   } catch (std::exception& e) {
//   }

//   if (projection_point_disk) {
//     auto proj_pos = projection_point_disk->position;
//     // loop below over clusters to find matches
//   }
// }
