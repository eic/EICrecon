#include "tracking_studiesProcessor.h"
#include <services/rootfile/RootFile_service.h>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>
#include "algorithms/tracking/JugTrack/TrackingResultTrajectory.hpp"
#include <spdlog/spdlog.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include "edm4eic/vector_utils.h"

#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/fmt/ostr.h>

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
// #include <extensions/spdlog/SpdlogMixin.h>
     
// The following just makes this a JANA plugin
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new tracking_studiesProcessor());
    }
}

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void tracking_studiesProcessor::InitWithGlobalRootLock(){
    std::string plugin_name=("tracking_studies");

    // InitLogger(plugin_name);
    // Get JANA application
    auto app = GetApplication();
    auto acts_service = GetApplication()->GetService<ACTSGeo_service>();

    std::string log_level_str = "debug";
    m_log = app->GetService<Log_service>()->logger(plugin_name);
    app->SetDefaultParameter(plugin_name + ":LogLevel", log_level_str, "LogLevel: trace, debug, info, warn, err, critical, off");
    m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

    m_geo_provider= acts_service->actsGeoProvider();
    m_propagation_algo.init(acts_service->actsGeoProvider(), m_log);

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
    hPosTestX  = new TH1D("hPosTestX",  "BEMC hit energy (raw)",  100, -100, 100);
    nHitsTrackVsEtaVsP = new TH3D("nHitsTrackVsEtaVsP", "", 100,-4.,4., 20, -0.5, 19.5, 150,0.,15.);
    nHitsEventVsEtaVsP = new TH3D("nHitsEventVsEtaVsP", "", 100,-4.,4., 20, -0.5, 19.5, 150,0.,15.);
    hPosTestX->SetDirectory(m_dir_main);
    nHitsTrackVsEtaVsP->SetDirectory(m_dir_main);
    nHitsEventVsEtaVsP->SetDirectory(m_dir_main);

    auto transform = Acts::Transform3::Identity();

    // Create propagation surfaces for cherenkov angular studies
    // make a reference disk to mimic center of mRICH
    const auto mRICH_center_Z = -1186.-20.; //center of aerogel
    const auto mRICH_center_MinR = 80.0;
    const auto mRICH_center_MaxR = 630;
    auto mRICH_center_Bounds = std::make_shared<Acts::RadialBounds>(mRICH_center_MinR, mRICH_center_MaxR);
    auto mRICH_center_Trf = transform * Acts::Translation3(Acts::Vector3(0, 0, mRICH_center_Z));
    m_mRICH_center_surface = Acts::Surface::makeShared<Acts::DiscSurface>(mRICH_center_Trf, mRICH_center_Bounds);

    // make a reference disk to mimic center of dRICH
    const auto dRICH_center_Z = 1950.+60.; // center of detector
    const auto dRICH_center_MinR = 84.0;
    const auto dRICH_center_MaxR = 1100.+(1212.82-1100.)*(60./120.);
    auto dRICH_center_Bounds = std::make_shared<Acts::RadialBounds>(dRICH_center_MinR, dRICH_center_MaxR);
    auto dRICH_center_Trf = transform * Acts::Translation3(Acts::Vector3(0, 0, dRICH_center_Z));
    m_dRICH_center_surface = Acts::Surface::makeShared<Acts::DiscSurface>(dRICH_center_Trf, dRICH_center_Bounds);


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
    nEventsFilled++;
    // cout << "tracking_studiesProcessor::ProcessSequential" << endl;
    double mceta = 0;
    double mcphi = 0;
    double mcp = 0;
    for( auto mcparticle : mcParticles() ){
        if(mcparticle->getGeneratorStatus() != 1) continue;
        auto& mom = mcparticle->getMomentum();
        // cout << "MC particle: " << mom.x << " " << mom.y << " " << mom.z << "\ttotmom: " << sqrt(mom.x*mom.x+mom.y*mom.y+mom.z*mom.z) << endl;
        // determine mceta from momentum
        mceta = -log(tan(atan2(sqrt(mom.x*mom.x+mom.y*mom.y),mom.z)/2.));
        // determine mcphi from momentum
        mcphi = atan2(mom.y, mom.x);
        // determine mc momentum
        mcp = sqrt(mom.x*mom.x+mom.y*mom.y+mom.z*mom.z);

    }

    int iseg = 0;
    for( auto track_segment : trackSegments() ){
        // cout << "\t\ttrksegment " << iseg << endl;
        int ipnt = 0;
        for(auto point: track_segment->getPoints()) {
            auto &pos = point.position;
            auto &mom = point.momentum;
            auto &theta = point.theta;
            auto &phi = point.phi;
            // cout << "\t\ttrkpoints: " << ipnt << endl;
            // cout << "\t\t\t point: radius: " << sqrt(pos.x*pos.x+pos.y*pos.y) << " z: " << pos.z << endl;
            // // cout << "\t\t\t momentum: " << mom.x << " " << mom.y << " " << mom.z << endl;
            // cout << "\t\t\t theta: " << theta << "\t phi: " << phi << endl;
            // cout << "\t\t\t calculated phi from momentum: " << atan2(mom.y, mom.x) << endl;
            // cout << "\t\t\t calculated theta from momentum: " << acos(mom.z/sqrt(mom.x*mom.x+mom.y*mom.y+mom.z*mom.z)) << endl;
            // cout << "\t\t\t calculated phi from position: " << atan2(pos.y, pos.x) << endl;
            // cout << "\t\t\t calculated theta from position: " << acos(pos.z/sqrt(pos.x*pos.x+pos.y*pos.y+pos.z*pos.z)) << endl;
            // hPosTestX->Fill(  pos.x);
            ipnt++;
        }
        iseg++;
    }

    auto acts_results = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories");
    m_log->debug("ACTS Trajectories( size: {} )", std::size(acts_results));
    m_log->debug("{:>10} {:>10}  {:>10} {:>10} {:>10} {:>10} {:>12} {:>12} {:>12} {:>8}",
                 "[loc 0]","[loc 1]", "[phi]", "[theta]", "[q/p]", "[p]", "[err phi]", "[err th]", "[err q/p]", "[chi2]" );
  // Loop over the trajectories
    for (const auto& traj : acts_results) {

        // Get the entry index for the single trajectory
        // The trajectory entry indices and the multiTrajectory
        const auto &mj = traj->multiTrajectory();
        const auto &trackTips = traj->tips();
        if (trackTips.empty()) {
            // m_log->debug("Empty multiTrajectory.");
            continue;
        }

        // projection to special surfaces
        {
            edm4eic::TrackPoint* projection_point_mRICH;
            try {
                // >>> try to propagate to surface <<<
                projection_point_mRICH = m_propagation_algo.propagate(traj, m_mRICH_center_surface);
            }
            catch(std::exception &e) {
                m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
                // cout << "Exception in underlying algorithm: " << e.what() << ". Trajectory is skipped" << endl;
            }

            if(!projection_point_mRICH) {
                m_log->trace("   could not propagate to mRICH!");
                // cout << "   could not propagate!" << endl;
                // continue;
            } else {
                // Now go through reconstructed tracks points

                auto pos = projection_point_mRICH->position;
                auto length =  projection_point_mRICH->pathlength;
                auto mom =  projection_point_mRICH->momentum;
                // m_log->trace("   {:>10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", traj_index, pos.x, pos.y, pos.z, length);
                cout << "\tmRICH_projection pos: " << pos.x << " " << pos.y << " " << pos.z << " " << length << endl;
                cout << "\t\t mom: " << mom.x << " " << mom.y << " " << mom.z << endl;
            }

            edm4eic::TrackPoint* projection_point_dRICH;
            try {
                // >>> try to propagate to surface <<<
                projection_point_dRICH = m_propagation_algo.propagate(traj, m_dRICH_center_surface);
            }
            catch(std::exception &e) {
                m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
                // cout << "Exception in underlying algorithm: " << e.what() << ". Trajectory is skipped" << endl;
            }

            if(!projection_point_dRICH) {
                m_log->trace("   could not propagate to dRICH!");
                // cout << "   could not propagate!" << endl;
                // continue;
            } else {
                // Now go through reconstructed tracks points

                auto pos = projection_point_dRICH->position;
                auto length =  projection_point_dRICH->pathlength;
                auto mom =  projection_point_dRICH->momentum;
                // m_log->trace("   {:>10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", traj_index, pos.x, pos.y, pos.z, length);
                cout << "\tdRICH_projection pos: " << pos.x << " " << pos.y << " " << pos.z << " " << length << endl;
                cout << "\t\t mom: " << mom.x << " " << mom.y << " " << mom.z << endl;
            }
        }

        const auto &trackTip = trackTips.front();

        // Collect the trajectory summary info
        auto trajState = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
        // auto trajState       = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
        if (traj->hasTrackParameters(trackTip)) {
            const auto &boundParam = traj->trackParameters(trackTip);
            const auto &parameter = boundParam.parameters();
            const auto &covariance = *boundParam.covariance();
            // cout << "track parameters: " << parameter << endl;
            // cout << "\tphi: " << parameter[Acts::eBoundPhi] << endl;
            // cout << "\teta: " << parameter[Acts::eBoundTheta] << endl;
            int  m_nMeasurements = trajState.nMeasurements;
            //TODO also add holes to count -> should agree with number of hits per event
            //TODO ahh I made one mistake, # of outliers  + # of measurements should sum to # of hits
            //TODO also check chi2
            // cout << "\tnMeasurements: " << m_nMeasurements << endl;

            nHitsTrackVsEtaVsP->Fill(mceta,m_nMeasurements,mcp);
            nTracksFilled++;
            // m_log->debug("{:>10.2f} {:>10.2f}  {:>10.2f} {:>10.3f} {:>10.4f} {:>10.3f} {:>12.4e} {:>12.4e} {:>12.4e} {:>8.2f}",
            //     parameter[Acts::eBoundLoc0],
            //     parameter[Acts::eBoundLoc1],
            //     parameter[Acts::eBoundPhi],
            //     parameter[Acts::eBoundTheta],
            //     parameter[Acts::eBoundQOverP],
            //     1.0 / parameter[Acts::eBoundQOverP],
            //     sqrt(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
            //     sqrt(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
            //     sqrt(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)),
            //     trajState.chi2Sum);
        }

        // visit the track points
        mj.visitBackwards(trackTip, [&](auto &&trackstate) {
            // get volume info
            auto geoID = trackstate.referenceSurface().geometryId();
            auto volume = geoID.volume();
            auto layer = geoID.layer();

            // if (trackstate.hasCalibrated()) {
            //     m_nCalibrated++;
            // }

            // get track state parameters and their covariances
            const auto &parameter = trackstate.predicted();
            const auto &covariance = trackstate.predictedCovariance();


            // convert local to global
            auto global = trackstate.referenceSurface().localToGlobal(
                    m_geo_provider->getActsGeometryContext(),
                    {parameter[Acts::eBoundLoc0], parameter[Acts::eBoundLoc1]},
                    {0, 0, 0}
            );
            // global position
            const decltype(edm4eic::TrackPoint::position) position{
                    static_cast<float>(global.x()),
                    static_cast<float>(global.y()),
                    static_cast<float>(global.z())
            };

            const decltype(edm4eic::TrackPoint::momentum) momentum = edm4eic::sphericalToVector(
                    static_cast<float>(1.0 / std::abs(parameter[Acts::eBoundQOverP])),
                    static_cast<float>(parameter[Acts::eBoundTheta]),
                    static_cast<float>(parameter[Acts::eBoundPhi])
            );
            const float theta(parameter[Acts::eBoundTheta]);
            const float phi(parameter[Acts::eBoundPhi]);
            // cout << "\tmomentum : " << momentum << "\t total: " << sqrt(momentum.x*momentum.x + momentum.y*momentum.y + momentum.z*momentum.z) << endl;

        });
    }


    std::vector<std::string> m_data_names = {
        "SiBarrelHits",         // Barrel Tracker
        "MPGDBarrelHits",       // MPGD
        "VertexBarrelHits",     // Vertex
        "TrackerEndcapHits",    // End Cap tracker
        "TOFEndcapHits",        // End Cap TOF
        "TOFBarrelHits",        // Barrel TOF
        // "DRICHHits",        // Barrel TOF
        "MPGDDIRCHits",        // Barrel TOF
        // "DIRCBarHits",        // Barrel TOF
        // "MRICHHits",        // Barrel TOF
    };

    int nHitsTrackers = 0;
    for(size_t name_index = 0; name_index < m_data_names.size(); name_index++ ) {
        string data_name = m_data_names[name_index];
        auto hits = event->Get<edm4hep::SimTrackerHit>(data_name);
        // cout << "Data from: " << data_name << " has " << hits.size() << " hits" << endl;
            // count_hist->Fill(hits.size());
        for(auto hit: hits) {
            float x = hit->getPosition().x;
            float y = hit->getPosition().y;
            float z = hit->getPosition().z;
            auto& mom = hit->getMomentum();
            // cout << "Hit: " << x << " " << y << " " << z << " " << mom.x << " " << mom.y << " " << mom.z << endl;
            // cout << "\teta(pos): " << 2*atan(exp(-1*z/sqrt(x*x+y*y))) << "\tphi(pos): " << atan2(y,x) << endl;
            // cout << "\teta(mom): " << 2*atan(exp(-1*mom.z/sqrt(mom.x*mom.x+mom.y*mom.y))) << "\tphi(mom): " << atan2(mom.y, mom.x) << endl;
            float r = sqrt(x*x + y*y);
            float etahit = -log(tan(r/z/2));
            nHitsTrackers++;
        }
    }
    nHitsEventVsEtaVsP->Fill(mceta,nHitsTrackers,mcp);


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
        // }); // make sure you don't forget to close the lambda function and also close the visitBackwards function call
*/
