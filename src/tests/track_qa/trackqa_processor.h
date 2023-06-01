#ifndef EICRECON_TRK_QA_PROCESSOR_H
#define EICRECON_TRK_QA_PROCESSOR_H

#include <JANA/JEventProcessor.h>
#include <JANA/JEventProcessorSequentialRoot.h>

#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include "algorithms/tracking/ActsGeometryProvider.h"

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

#include <iostream>
// #include <fstream>
// #include <string>
#include <vector>
#include <map>
// #include <sstream>
// #include <cstdlib>
using namespace std;

class JEvent;
class JApplication;

class trackqa_processor:
        public JEventProcessor,
        public eicrecon::SpdlogMixin<trackqa_processor>   // this automates proper log initialization
{
public:
    explicit trackqa_processor(JApplication *);
    ~trackqa_processor() override = default;

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

    //Histograms
    TH2 *h1a; //Rec track momentum vs. true momentum
    TH1 *hchi2; //chi^2 histogram
    TH1 *heta; //eta histogram
    TH1 *hp; //p histogram
    TH1 *hpt; //pt histogram
    TH1 *hhits; //number of hits
    TH1 *hNDF; //number of degrees of freedom
    TH1 *hchi2_by_hits; //chi^2 divided by the # of hits
    TH1 *hchi2_by_NDF; //chi^2 divided by the # of degrees of freedom
    TH1 *hchi2_by_meas; //chi^2 divided by the # of meas

    //Looking at chi^2 and # of hits
    TH2 *hchi2_vs_eta; //chi^2 vs eta
    TH2 *hchi2_vs_hits; //chi^2 vs # of hits (chi^2 is in y-axis, # of hits is in x-axis)
    TH2 *hchi2_vs_hits_zoomed; //chi^2 vs # of hits (chi^2 is in y-axis, # of hits is in x-axis) zoomed in
    vector<TH2*> hchi2_vs_hits_etabins; //chi^2 vs # of hits in 16 bins of eta
    
    TH2 *hhits_vs_eta; //# of hits vs eta
    TH2 *hhits_vs_eta_1; //# of hits vs eta, require at least 1 track
    TH2 *htracks_vs_eta; //# of tracks vs eta
    TH3 *heta_vs_p_vs_chi2; //#eta vs p vs chi^2
    
    TH2 *hNDF_states; //Number of states vs. NDF 

    //now looking at the number of measurements per track
    TH2 *hmeasptrack_vs_eta; //# of measurements per track vs eta
    TH2 *hmeasptrack_vs_hits; //# of measurements per track vs # of hits
    TH2 *hmeasptrack_vs_chi2perNDF; //# of measurements per track vs chi^2
    vector<TH2*> hmeasptrack_vs_hits_etabins; //# of measurements per track vs # of hits in 16 bins of eta
    vector<TH2*> hmeasptrack_vs_hits_etabins_zoomed; //# of measurements per track vs # of hits in 16 bins of eta zoomed in (smaller range)
    vector<TH2*> hmeasptrack_vs_chi2perNDF_etabins; //# of measurements per track vs chi^2 in 16 bins of eta
    TH2 *hmeasptrack_vs_calstates; //# of measurements per tracks vs # of calibrated states

    TH2 *hholes_vs_hits; //# of holes vs # of hits
    TH2 *houtliers_vs_hits; //# of outliers vs # of hits
    TH2 *hsummation; //compare # calibrated states with # of trajectories + # outliers //This comparison doesn't make sense - remove later
    TH2 *hsummation2; //confirm that # hits = # of outliers + # meas per track
    TH2 *hsummation3; //compare #meas + #outliers + #holes to #states

    //look at the individual chi^2 per measurement
    TH2 *hmeaschi2_vs_chi2;
    TH2 *hmeaschi2_vs_eta;
    TH2 *hmeaschi2_vs_hits;

    //look at the r,z, and residuals
    vector<TH1*> hhits_in_r;
    vector<TH1*> hhits_in_z;
    vector<TH2*> hhits_r_vs_z;

    vector<TH1*> hmeas_in_r;
    vector<TH1*> hmeas_in_z;
    vector<TH2*> hmeas_r_vs_z;

    vector<TH1*> hresiduals;
    vector<TH1*> hresiduals_vollaybins;
    vector<vector<TH1*>> hresiduals_layers_in_pbins;

    vector<float> m_res_x_hit;   ///< hit residual x
    TH2 *hmeaschi2_vs_volID;
    TH2 *hmeaschi2_vs_layID;
    TH2 *hmeaschi2_vs_vollayIDs;

    TH1 *hvolID; //just the volume ID
    TH1 *hlayID;
    TH1 *hvollayIDs;
    // int vollay_arr[20] = {0,22,142,144,146,192,242,262,264,266,282,302,312,332,342,344,346,352,362,382}; //indices of all (volID*10 + layID) indices - ARCHES
    int vollay_arr[20] = {0,22,122,124,126,172,222,242,244,246,262,282,292,312,322,324,326,332,342,362}; //indices of all (volID*10 + layID) indices - BRYCE CANYON
    char vollay_identities[20][20] = {"all","dead","b disk 5","b disk 4","b disk 3","b disk 2","b disk 1",
                                    "vertex 1","vertex 2","vertex 3","f disk 1","barrel sagitta 1","f disk 2",
                                    "barrel sagitta 2","f disk 3","f disk 4","f disk 5",
                                    "MPGD barrel","TOF barrel","MPGD DIRC"}; //corresponding labels of all (volID*10 + layID) indices
    char mom_bins_arr[4][20] = {"1-2","2-5","5-7","7-10"};
    map<int,int> vollay_index;
    // vector<TH2*> htrackstate_r_vs_vollayIDs; //the r position of where track is at a trackstate vs each volume ID * 10 + layer ID
    // vector<TH2*> htrackstate_z_vs_vollayIDs;
    vector<TH1*> htrackstate_r; //the r position of where track is at a trackstate vs each volume ID * 10 + layer ID
    vector<TH1*> htrackstate_z;
    vector<TH2*> htrackstate_r_vs_z;
    vector<TH2*> hmeas_outliers_r_vs_z;
    vector<TH2*> hmeas_holes_r_vs_z;


    int test_counter;

private:

    std::shared_ptr<const ActsGeometryProvider> m_geo_provider;

    /// Directory to store histograms to
    TDirectory *m_dir_main{};
    TDirectory *m_dir_sub{};
    TDirectory *m_dir_res{};

};

#endif //EICRECON_TRK_QA_PROCESSOR_H
