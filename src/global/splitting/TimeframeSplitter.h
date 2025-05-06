// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/Utils/JEventLevel.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include "services/io/podio/datamodel_glue.h"
#include <JANA/JEventUnfolder.h>

struct TimeframeSplitter : public JEventUnfolder {

    // PodioInput<ExampleCluster> m_timeslice_clusters_in {this, {.name = "ts_protoclusters", .level = JEventLevel::Timeslice}};

    // PodioOutput<ExampleCluster> m_event_clusters_out {this, "evt_protoclusters"};
    // PodioOutput<EventInfo> m_event_info_out {this, "evt_info"};

    VariadicPodioInput<edm4hep::SimTrackerHit> m_simtrackerhits_in {this, {.names={
                "MPGDBarrelHits",
                "SiBarrelHits",
                "TOFEndcapHits"
            },
            .is_optional=true}};

    VariadicPodioOutput<edm4hep::SimTrackerHit> m_simtrackerhits_out {this, {
                "MPGDBarrelHits",
                "SiBarrelHits",
                "TOFEndcapHits"
            }};


    TimeframeSplitter() {
        SetTypeName(NAME_OF_THIS);
        SetParentLevel(JEventLevel::Timeslice);
        SetChildLevel(JEventLevel::PhysicsEvent);

        for (size_t coll_index=0; coll_index < m_simtrackerhits_in().size(); ++ coll_index) {

            const auto* coll_in = m_simtrackerhits_in().at(coll_index);
            auto& coll_out = m_simtrackerhits_out().at(coll_index);
            coll_out->setSubsetCollection(true);
            for (const auto& hit : *coll_in) {
                coll_out->push_back(hit);
            }
        }
    }


    Result Unfold(const JEvent& parent, JEvent& child, int child_idx) override {

        auto timeslice_nr = parent.GetEventNumber();
        size_t event_nr = 100*timeslice_nr + child_idx;
        child.SetEventNumber(event_nr);

        // For now, a one-to-one relationship between timeslices and events

        /*
        auto event_clusters_out = std::make_unique<ExampleClusterCollection>();
        event_clusters_out->setSubsetCollection(true);
        event_clusters_out->push_back(m_timeslice_clusters_in()->at(child_idx));

        auto event_info_out = std::make_unique<EventInfoCollection>();
        event_info_out->push_back(MutableEventInfo(event_nr, timeslice_nr, 0));

        m_event_clusters_out() = std::move(event_clusters_out);
        m_event_info_out() = std::move(event_info_out);
        */

        return (child_idx == 2) ? Result::NextChildNextParent : Result::NextChildKeepParent;
    }
};
