// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKING_OCCUPANCY_ANALYSIS_H
#define EICRECON_TRACKING_OCCUPANCY_ANALYSIS_H

#include <TH1F.h>
#include <TH3F.h>
#include <TH2F.h>
#include <JANA/JApplication.h>

class TrackingOccupancyAnalysis {

public:
    void init(JApplication *app, TDirectory *plugin_tdir);

    void process(const std::shared_ptr<const JEvent> &event);

private:

    /// This is edm4hep::SimTrackerHits names of different detector readouts
    std::vector<std::string> m_data_names = {
            "SagittaSiBarrelHits",      // Barrel Tracker
            "OuterSiBarrelHits",
            "VertexBarrelHits",         // Vertex
            "InnerTrackerEndcapPHits",  // End Cap tracker
            "InnerTrackerEndcapNHits",
            "MiddleTrackerEndcapPHits",
            "MiddleTrackerEndcapNHits",
            "OuterTrackerEndcapPHits",
            "OuterTrackerEndcapNHits",
            "InnerMPGDBarrelHits",      // MPGD
            "OuterMPGDBarrelHits"
    };

    /// Hits count histogram for each hits readout name
    std::vector<std::shared_ptr<TH1F>> m_hits_count_hists;

    /// Hits occupancy histogram for each hits readout name
    std::vector<std::shared_ptr<TH2F>> m_hits_occup_hists;

    /// Total occupancy of all m_data_names
    TH2F * m_total_occup_th2;                 /// MC Particles px,py


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


#endif //EICRECON_TRACKING_OCCUPANCY_ANALYSIS_H
