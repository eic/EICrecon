#pragma once

#include <Acts/Surfaces/CylinderSurface.hpp>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <TDirectory.h>
#include <TTree.h>
#include <memory>

#include "algorithms/tracking/TrackPropagation.h"
#include "extensions/spdlog/SpdlogMixin.h"

class PidAngleRes_processor:
        public JEventProcessor,
        public eicrecon::SpdlogMixin   // this automates proper log initialization
{
public:
    explicit PidAngleRes_processor(JApplication *);
    ~PidAngleRes_processor() override = default;

    //----------------------------
    // Init
    //
    // This is called once before the first call to the Process method
    // below. You may, for example, want to open an output file here.
    // Only one thread will call this.
    void Init() override;

    //----------------------------
    // Process
    //
    // This is called for every event. Multiple threads may call this
    // simultaneously. If you write something to an output file here
    // then make sure to protect it with a mutex or similar mechanism.
    // Minimize what is done while locked since that directly affects
    // the multi-threaded performance.
    void Process(const std::shared_ptr<const JEvent>& event) override;

    //----------------------------
    // Finish
    //
    // This is called once after all events have been processed. You may,
    // for example, want to close an output file here.
    // Only one thread will call this.
    void Finish() override;

private:

    /// Directory to store histograms to
    TDirectory *m_dir_main{};

    TTree *TreeTruth = nullptr;
    TTree *TreeSeed  = nullptr;
    /// Tracking propagation algorithm
    eicrecon::TrackPropagation m_propagation_algo;

    /// A surface to propagate to
    std::shared_ptr<Acts::CylinderSurface> m_dirc_surf;

    typedef struct {double p,pT,eta,theta,phi,x,y,z,r;} HIT; //Truth hit info from reference
    typedef struct {double p,pT,eta,theta,phi,x,y,z,r;} PROJ;//Info from propagated track
    typedef struct {double deta,dtheta,dphi,dz,dR;} RES;//resolutions
    typedef struct {double theta,phi,err_theta,err_phi,err_thetaphi,px,py,pz,posx,posy,posz;} POINT;//infor from covariance matrix

    HIT   DIRCTruthhit;
    PROJ  DIRCTruthproj;
    RES   DIRCTruthres;
    POINT DIRCTruthpoint;

    HIT   DIRCSeedhit;
    PROJ  DIRCSeedproj;
    RES   DIRCSeedres;
    POINT DIRCSeedpoint;

};
