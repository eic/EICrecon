#include "TofEfficiency_processor.h"
#include <services/rootfile/RootFile_service.h>

#include <Evaluator/DD4hepUnits.h>
#include <TVector3.h>

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void TofEfficiency_processor::InitWithGlobalRootLock(){
    std::string plugin_name=("tof_efficiency");

    InitLogger(plugin_name);

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

    auto phi_limit_min = 0;
    auto phi_limit_max = 6.29;
    auto z_limit_min = -1250;
    auto z_limit_max = 1250;
    auto r_limit_min = 50;
    auto r_limit_max = 675;

    m_th2_btof_phiz = new TH2F("btof_phiz", "Hit position for Barrel TOF", 100, phi_limit_min, phi_limit_max, 100, z_limit_min, z_limit_max);
    m_th2_ftof_rphi = new TH2F("ftof_rphi", "Hit position for Forward TOF", 100, r_limit_min, r_limit_max, 100, phi_limit_min, phi_limit_max);
    m_th2_btof_phiz->SetDirectory(m_dir_main);
    m_th2_ftof_rphi->SetDirectory(m_dir_main);

    
//    m_tntuple_track = new TNtuple("track","track with tof","det:pdg:p:track_phi:proj_x:proj_y:proj_z:proj_pathlength:proj_nhits_tof:tofhit_x:tofhit_y:tofhit_z:tofhit_t:tofhit_dca:tofhit_barrel_n:tofhit_endcap_n");
    m_tntuple_track = new TNtuple("track","track with tof","det:pdg:p:track_phi:track_n:proj_pathlength:proj_nhits_tof:tofhit_t:tofhit_dca:tofhit_barrel_n:tofhit_endcap_n");
    m_tntuple_track->SetDirectory(m_dir_main);
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void TofEfficiency_processor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {
    // go through MC particles to find primary particle. thus only works for particle gun events at the moment:
    logger()->trace("MC particles:");
    m_log->trace("   {:>10} {:>10}", "[pdg]", "[status]");
    int thisPDG = 0;
    for (auto mc_part: mcParticles()) {
        m_log->trace("   {:>10} {:>10}", mc_part->getPDG(), mc_part->getGeneratorStatus());
    	if (mc_part->getGeneratorStatus() == 1){
    	    thisPDG = mc_part->getPDG();
    	    break;
        }
    }


    // List TOF Barrel hits from barrel
    logger()->trace("TOF barrel hits:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[time]");
    for (auto hit: barrelHits()) {
        auto& pos = hit->getPosition();
        float r=sqrt(pos.x*pos.x+pos.y*pos.y);
        float phi=acos(pos.x/r); if(pos.y<0) phi+=3.1415927;
        m_th2_btof_phiz->Fill(phi, pos.z);
        m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.4f}", pos.x, pos.y, pos.z, hit->getTime());
    }

    // List TOF endcap hits
    logger()->trace("TOF endcap hits:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[time]");
    for (auto hit: endcapHits()) {
        auto& pos = hit->getPosition();
        float r=sqrt(pos.x*pos.x+pos.y*pos.y);
        float phi=acos(pos.x/r); if(pos.y<0) phi+=3.1415927;
        m_th2_ftof_rphi->Fill(r, phi);
        m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.4f}", pos.x, pos.y, pos.z, hit->getTime());
    }

    // Now go through reconstructed tracks points
    logger()->trace("Going over tracks:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[length]");

    // count track projections into tof:
    int tracks_into_tof = 0;
    for( int i_trk = 0; i_trk < trackSegments().size(); i_trk++){
        auto track_segment = trackSegments().at(i_trk);

        for(auto point: track_segment->getPoints()) {
            auto &pos = point.position;

            int det=IsTOFHit(pos.x, pos.y, pos.z);
            if(det != 0) {
            	// track with tof hit
            	tracks_into_tof += 1;
            }
        }
    }

    for( int i_trk = 0; i_trk < trackSegments().size(); i_trk++){
    	auto track_segment = trackSegments().at(i_trk);
    	auto reco_track = recoTracks().at(i_trk);
// DANGER!! was this wrong? \/
//    	i_trk = i_trk + 1;
// DANGE!! was this wrong?  /\
        logger()->trace(" Track trajectory");
        
	auto p_track = reco_track->getMomentum();
	auto p_track_mag = sqrt(p_track.x*p_track.x + p_track.y*p_track.y + p_track.z*p_track.z);
	auto p_track_phi = acos(p_track.z / p_track_mag);

        for(auto point: track_segment->getPoints()) {
            auto &pos = point.position;
            m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", pos.x, pos.y, pos.z, point.pathlength);

            int det=IsTOFHit(pos.x, pos.y, pos.z);
            float distance_closest=1e6;
            float hit_x=-1000, hit_y=-1000, hit_z=-1000, hit_t=-1000;
            float hit_px=-1000, hit_py=-1000, hit_pz=-1000, hit_e=-1000;
            if(det==1) {
                for (auto hit: barrelHits()) {
                    auto& hitpos = hit->getPosition();
                    float distance=sqrt((hitpos.x-pos.x)*(hitpos.x-pos.x)+(hitpos.y-pos.y)*(hitpos.y-pos.y)+(hitpos.z-pos.z)*(hitpos.z-pos.z));
                    if(distance<distance_closest) {
                        distance_closest=distance;
                        hit_x=hitpos.x;
                        hit_y=hitpos.y;
                        hit_z=hitpos.z;
                        hit_t=hit->getTime();
                    }
                }
            }
            if(det==2) {
                for (auto hit: endcapHits()) {
                    auto& hitpos = hit->getPosition();
                    float distance=sqrt((hitpos.x-pos.x)*(hitpos.x-pos.x)+(hitpos.y-pos.y)*(hitpos.y-pos.y)+(hitpos.z-pos.z)*(hitpos.z-pos.z));
                    if(distance<distance_closest) {
                        distance_closest=distance;
                        hit_x=hitpos.x;
                        hit_y=hitpos.y;
                        hit_z=hitpos.z;
                        hit_t=hit->getTime();
                    }
                }
            }
//                                          "det:pdg:p:track_phi:proj_x:proj_y:proj_z:proj_pathlength:proj_nhits_tof:tofhit_x:tofhit_y:tofhit_z:tofhit_t:tofhit_dca:tofhit_barrel_n:tofhit_endcap_n:");

//            if(det!=0) m_tntuple_track->Fill({det, thisPDG, p_track_mag, p_track_phi, pos.x, pos.y, pos.z, point.pathlength, tracks_into_tof, hit_x, hit_y, hit_z, hit_t, distance_closest, int(barrelHits().size()), int(endcapHits().size())});
            if(det!=0) m_tntuple_track->Fill(det, thisPDG, p_track_mag, p_track_phi, trackSegments().size(), point.pathlength, tracks_into_tof, hit_t, distance_closest, barrelHits().size(), endcapHits().size());
        }
    }
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void TofEfficiency_processor::FinishWithGlobalRootLock() {

    // Do any final calculations here.

}

int TofEfficiency_processor::IsTOFHit(float x, float y, float z) {
    const float btof_rmin=630;
    const float btof_rmax=642;
    const float btof_zmin=-1210;
    const float btof_zmax=+1210;

    const float ftof_rmin=65;
    const float ftof_rmax=680;
    const float ftof_zmin=1900;
    const float ftof_zmax=1940;

    float r=sqrt(x*x+y*y);

    if(r>btof_rmin&&r<btof_rmax&&z>btof_zmin&&z<btof_zmax) return 1;
    else if(r>ftof_rmin&&r<ftof_rmax&&z>ftof_zmin&&z<ftof_zmax) return 2;
    else return 0;
}
