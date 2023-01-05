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
    
    m_tntuple_track = new TNtuple("track","track with tof","particle_pdg:particle_px:particle_py:particle_pz:particle_p:track_px:track_py:track_pz:track_p:proj_n:proj_det_0:proj_x_0:proj_y_0:proj_z_0:proj_r_0:proj_phi_0:proj_pathlength_0:simhit_x_0:simhit_y_0:simhit_z_0:simhit_r_0:simhit_phi_0:simhit_t_0:simhit_dca_0:rechit_x_0:rechit_y_0:rechit_z_0:rechit_r_0:rechit_phi_0:rechit_t_0:rechit_dca_0:proj_det_1:proj_x_1:proj_y_1:proj_z_1:proj_r_1:proj_phi_1:proj_pathlength_1:simhit_x_1:simhit_y_1:simhit_z_1:simhit_r_1:simhit_phi_1:simhit_t_1:simhit_dca_1:rechit_x_1:rechit_y_1:rechit_z_1:rechit_r_1:rechit_phi_1:rechit_t_1:rechit_dca_1");
    m_tntuple_track->SetDirectory(m_dir_main);
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void TofEfficiency_processor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {

    // List TOF Barrel hits from barrel
    logger()->trace("TOF barrel sim hits:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[time]");
    for (auto hit: barrelSimHits()) {
        auto& pos = hit->getPosition();
        float r=sqrt(pos.x*pos.x+pos.y*pos.y);
        float phi=acos(pos.x/r); if(pos.y<0) phi+=3.1415927;
        m_th2_btof_phiz->Fill(phi, pos.z);
        m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.4f}", pos.x, pos.y, pos.z, hit->getTime());
    }

    logger()->trace("TOF barrel rec hits:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[time]");
    for (auto hit: barrelRecHits()) {
        auto& pos = hit->getPosition();
        float r=sqrt(pos.x*pos.x+pos.y*pos.y);
        float phi=acos(pos.x/r); if(pos.y<0) phi+=3.1415927;
        m_th2_btof_phiz->Fill(phi, pos.z);
        m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.4f}", pos.x, pos.y, pos.z, hit->getTime());
    }

    // List TOF endcap hits
    logger()->trace("TOF endcap sim hits:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[time]");
    for (auto hit: endcapSimHits()) {
        auto& pos = hit->getPosition();
        float r=sqrt(pos.x*pos.x+pos.y*pos.y);
        float phi=acos(pos.x/r); if(pos.y<0) phi+=3.1415927;
        m_th2_ftof_rphi->Fill(r, phi);
        m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.4f}", pos.x, pos.y, pos.z, hit->getTime());
    }

    logger()->trace("TOF endcap rec hits:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[time]");
    for (auto hit: endcapRecHits()) {
        auto& pos = hit->getPosition();
        float r=sqrt(pos.x*pos.x+pos.y*pos.y);
        float phi=acos(pos.x/r); if(pos.y<0) phi+=3.1415927;
        m_th2_ftof_rphi->Fill(r, phi);
        m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.4f}", pos.x, pos.y, pos.z, hit->getTime());
    }

    // Now go through reconstructed tracks points
    logger()->trace("Going over tracks:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[length]");
    for( auto track_segment : trackSegments() ){
        int particle_pdg=0;
        float particle_px=-1000, particle_py=-1000, particle_pz=-1000, particle_p=-1000;
        float track_px=-1000, track_py=-1000, track_pz=-1000, track_p=0, track_minlength=1e6;
        int proj_n=0;
        int proj_det[2]={0,0};
        float proj_x[2]={-1000,-1000}, proj_y[2]={-1000,-1000}, proj_z[2]={-1000,-1000}, proj_r[2]={-1000,-1000}, proj_phi[2]={-1000,-1000}, proj_pathlength[2]={-1000,-1000};
        float simdistance_closest[2]={1e6,1e6};
        float simhit_x[2]={-1000,-1000}, simhit_y[2]={-1000,-1000}, simhit_z[2]={-1000,-1000}, simhit_r[2]={-1000,-1000}, simhit_phi[2]={-1000,-1000}, simhit_t[2]={-1000,-1000};
        float recdistance_closest[2]={1e6,1e6};
        float rechit_x[2]={-1000,-1000}, rechit_y[2]={-1000,-1000}, rechit_z[2]={-1000,-1000}, rechit_r[2]={-1000,-1000}, rechit_phi[2]={-1000,-1000}, rechit_t[2]={-1000,-1000};
        
        logger()->trace(" Track trajectory");
        for(auto point: track_segment->getPoints()) {
            auto &mom = point.momentum;
            m_log->trace("Momentum:  {:>10.2f} {:>10.2f} {:>10.2f}", mom.x, mom.y, mom.z);
            if(point.pathlength<track_minlength) {
                track_px=mom.x;
                track_py=mom.y;
                track_pz=mom.z;
                track_p=sqrt(mom.x*mom.x+mom.y*mom.y+mom.z*mom.z);
                track_minlength=point.pathlength;
                m_log->trace("Project:  {:>10.2f} {:>10.2f} {:>10.2f}", mom.x, mom.y, mom.z);
            }
            
            auto &pos = point.position;
            m_log->trace("Position:  {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", pos.x, pos.y, pos.z, point.pathlength);
            int det=IsTOFHit(pos.x, pos.y, pos.z);
            if(det==0) continue;
            if(proj_n>1) {
                logger()->trace("Find more than two TOF hits");
                continue;
            }
            
            proj_det[proj_n]=det;
            proj_x[proj_n]=pos.x;
            proj_y[proj_n]=pos.y;
            proj_z[proj_n]=pos.z;
            proj_r[proj_n]=sqrt(proj_x[proj_n]*proj_x[proj_n]+proj_y[proj_n]*proj_y[proj_n]);
            proj_phi[proj_n]=acos(proj_x[proj_n]/proj_r[proj_n]); if(proj_y[proj_n]<0) proj_phi[proj_n]*=-1;
            proj_pathlength[proj_n]=point.pathlength;
            
            if(proj_det[proj_n]==1) {
                logger()->trace("Find TOF barrel hits:");
                for (auto hit: barrelSimHits()) {
                    auto& hitpos = hit->getPosition();
                    float simdistance=sqrt((hitpos.x-pos.x)*(hitpos.x-pos.x)+(hitpos.y-pos.y)*(hitpos.y-pos.y)+(hitpos.z-pos.z)*(hitpos.z-pos.z));
                    if(simdistance<simdistance_closest[proj_n]) {
                        simdistance_closest[proj_n]=simdistance;
                        simhit_x[proj_n]=hitpos.x;
                        simhit_y[proj_n]=hitpos.y;
                        simhit_z[proj_n]=hitpos.z;
                        simhit_r[proj_n]=sqrt(hitpos.x*hitpos.x+hitpos.y*hitpos.y);
                        simhit_phi[proj_n]=acos(hitpos.x/simhit_r[proj_n]); if(hitpos.y<0) simhit_phi[proj_n]*=-1;
                        simhit_t[proj_n]=hit->getTime();
                    }
                }
                for (auto hit: barrelRecHits()) {
                    auto& hitpos = hit->getPosition();
                    float recdistance=sqrt((hitpos.x-pos.x)*(hitpos.x-pos.x)+(hitpos.y-pos.y)*(hitpos.y-pos.y)+(hitpos.z-pos.z)*(hitpos.z-pos.z));
                    if(recdistance<recdistance_closest[proj_n]) {
                        recdistance_closest[proj_n]=recdistance;
                        rechit_x[proj_n]=hitpos.x;
                        rechit_y[proj_n]=hitpos.y;
                        rechit_z[proj_n]=hitpos.z;
                        rechit_r[proj_n]=sqrt(hitpos.x*hitpos.x+hitpos.y*hitpos.y);
                        rechit_phi[proj_n]=acos(hitpos.x/rechit_r[proj_n]); if(hitpos.y<0) rechit_phi[proj_n]*=-1;
                        rechit_t[proj_n]=hit->getTime();
                    }
                }
            }
            if(proj_det[proj_n]==2) {
                logger()->trace("Find TOF endcap hits:");
                for (auto hit: endcapSimHits()) {
                    auto& hitpos = hit->getPosition();
                    float simdistance=sqrt((hitpos.x-pos.x)*(hitpos.x-pos.x)+(hitpos.y-pos.y)*(hitpos.y-pos.y)+(hitpos.z-pos.z)*(hitpos.z-pos.z));
                    if(simdistance<simdistance_closest[proj_n]) {
                        simdistance_closest[proj_n]=simdistance;
                        simhit_x[proj_n]=hitpos.x;
                        simhit_y[proj_n]=hitpos.y;
                        simhit_z[proj_n]=hitpos.z;
                        simhit_r[proj_n]=sqrt(hitpos.x*hitpos.x+hitpos.y*hitpos.y);
                        simhit_phi[proj_n]=acos(hitpos.x/simhit_r[proj_n]); if(hitpos.y<0) simhit_phi[proj_n]*=-1;
                        simhit_t[proj_n]=hit->getTime();
                    }
                }
                for (auto hit: endcapRecHits()) {
                    auto& hitpos = hit->getPosition();
                    float recdistance=sqrt((hitpos.x-pos.x)*(hitpos.x-pos.x)+(hitpos.y-pos.y)*(hitpos.y-pos.y)+(hitpos.z-pos.z)*(hitpos.z-pos.z));
                    if(recdistance<recdistance_closest[proj_n]) {
                        recdistance_closest[proj_n]=recdistance;
                        rechit_x[proj_n]=hitpos.x;
                        rechit_y[proj_n]=hitpos.y;
                        rechit_z[proj_n]=hitpos.z;
                        rechit_r[proj_n]=sqrt(hitpos.x*hitpos.x+hitpos.y*hitpos.y);
                        rechit_phi[proj_n]=acos(hitpos.x/rechit_r[proj_n]); if(hitpos.y<0) rechit_phi[proj_n]*=-1;
                        rechit_t[proj_n]=hit->getTime();
                    }
                }
            }
            
            proj_n++;
            if(proj_n>1) logger()->trace("Find more than one TOF hits");
        }

        //if(proj_n!=0) {
        {
            logger()->trace("Going over particles:");
            m_log->trace("Track:  {:>10.2f} {:>10.2f} {:>10.2f}", track_px, track_py, track_pz);
            for( auto mc_particle : mcParticles() ){
                auto &mom = mc_particle->getMomentum();
                m_log->trace("Particle:  {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", mom.x, mom.y, mom.z, abs(mom.x-track_px), abs(mom.y-track_py), abs(mom.z-track_pz));
                if(abs(mom.x-track_px)<5e-2&&abs(mom.y-track_py)<5e-2&&abs(mom.z-track_pz)<5e-2) {
                    particle_pdg=mc_particle->getPDG();
                    particle_px=mom.x;
                    particle_py=mom.y;
                    particle_pz=mom.z;
                    particle_p=sqrt(particle_px*particle_px+particle_py*particle_py+particle_pz*particle_pz);
                    m_log->trace("Find matched MC particle");
                }
            }

            float tmp[52]={(float)particle_pdg, particle_px, particle_py, particle_pz, particle_p, track_px, track_py, track_pz, track_p, (float)proj_n, (float)proj_det[0], proj_x[0], proj_y[0], proj_z[0], proj_r[0], proj_phi[0], proj_pathlength[0], simhit_x[0], simhit_y[0], simhit_z[0], simhit_r[0], simhit_phi[0], simhit_t[0], simdistance_closest[0], rechit_x[0], rechit_y[0], rechit_z[0], rechit_r[0], rechit_phi[0], rechit_t[0], recdistance_closest[0], (float)proj_det[0], proj_x[1], proj_y[1], proj_z[1], proj_r[1], proj_phi[1], proj_pathlength[1], simhit_x[1], simhit_y[1], simhit_z[1], simhit_r[1], simhit_phi[1], simhit_t[1], simdistance_closest[1], rechit_x[1], rechit_y[1], rechit_z[1], rechit_r[1], rechit_phi[1], rechit_t[1], recdistance_closest[1]};
            m_tntuple_track->Fill(tmp);
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

