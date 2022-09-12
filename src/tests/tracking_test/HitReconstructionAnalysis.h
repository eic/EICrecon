// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_HITRECONSTRUCTIONANALYSIS_H
#define EICRECON_HITRECONSTRUCTIONANALYSIS_H

#include <TH1F.h>
#include <TH3F.h>
#include <TH2F.h>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

class HitReconstructionAnalysis {
public:
    void init(JApplication *app, TDirectory *plugin_tdir);

    void process(const std::shared_ptr<const JEvent> &event);

private:

    /// This is edm4hep::SimTrackerHits names of different detector readouts
    std::vector<std::string> m_data_names = {
            "BarrelTrackerHit",     // Barrel Tracker
            "BarrelVertexHit",      // Vertex
            "EndcapTrackerHit",     // End Cap tracker
            "MPGDTrackerHit"        // MPGD
    };

    /// Hits count histogram for each hits readout name
    std::vector<std::shared_ptr<TH1F>> m_hits_count_hists;

    /// Hits occupancy histogram for each hits readout name
    std::vector<std::shared_ptr<TH2F>> m_hits_occup_hists;

    /// Total occupancy of all m_data_names
    TH2F * m_total_occup_th2;                 /// MC Particles px,py


};


#endif //EICRECON_HITRECONSTRUCTIONANALYSIS_H
