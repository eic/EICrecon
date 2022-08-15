#ifndef EICRECON_OCCUPANCY_ANALYSIS_H
#define EICRECON_OCCUPANCY_ANALYSIS_H

#include <TH1F.h>
#include <TH3F.h>
#include <TH2F.h>

#include <JANA/JEventProcessor.h>

class JEvent;
class JApplication;

class OccupancyAnalysis:public JEventProcessor
{
public:
    explicit OccupancyAnalysis(JApplication *);
    ~OccupancyAnalysis() override = default;

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

    TDirectory* m_dir_main;               /// Main TDirectory for this plugin 'occupancy_ana'
    TH1F * m_th1_prt_pz;                  /// MC Particles pz
    TH1F * m_th1_prt_energy;              /// MC Particles total E
    TH1F * m_th1_prt_theta;               /// MC Particles theta angle
    TH1F * m_th1_prt_phi;                 /// MC Particles phi angle
    TH2F * m_th2_prt_pxy;                 /// MC Particles px,py

//    std::string VolNameToDetName(const std::string& vol_name);
//
//    bool hits_are_produced = false;
//    bool gen_particles_are_produced = false;
//
//    ej::EServicePool services;
//    std::vector<std::string> detector_names = {"Barrel_CTD", "VTX", "GEM", "fSI"};
//
//    TH2F * total_occ{};
//    TH2F * h_part_occ{};
//    TH2F * e_part_occ{};
//
//
//
//
//    std::recursive_mutex lock;
//
//    TH3F * th3_hits3d{};
//
//    PtrByNameMapHelper<TH3F> *th3_by_detector{};
//    PtrByNameMapHelper<TH2F> *th2_by_detector{};
//    PtrByNameMapHelper<TH2F> *th2_by_layer{};
//    PtrByNameMapHelper<TH1F> *th1_z_by_detector{};
//
//
//    TDirectory* dir_th2_by_detector{};    /// TDir for
//    TDirectory* dir_th2_by_layer{};
//    TDirectory* dir_th3_by_detector{};
//    TDirectory* dir_th1_by_detector{};
};

#endif //EICRECON_OCCUPANCY_ANALYSIS_H
