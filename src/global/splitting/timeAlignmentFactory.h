// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/Components/JOmniFactory.h>
// #include <JANA/Utils/JEventKey.h>

#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>

#include "services/io/podio/datamodel_glue.h"

struct timeAlignmentFactory : public JOmniFactory<timeAlignmentFactory> {
    JEventLevel m_factory_level;

    PodioInput<edm4hep::SimTrackerHit> check_hits_in {this};
    PodioOutput<edm4hep::SimTrackerHit> check_hits_out {this};

    Double_t m_time_offset = 0.0; // Time offset to apply to hits

    void Configure(){
    }

    void ChangeRun(int32_t /*run_nr*/) {
    }

    void Execute(int64_t run_number, uint64_t event_number) {

        // Sort hits by time
        std::vector<edm4hep::MutableSimTrackerHit> sorted_hits;
        for (const auto& hit : *check_hits_in) {
            edm4hep::MutableSimTrackerHit copiedHit = hit.clone();
            copiedHit.setTime(hit.getTime() - m_time_offset);
            sorted_hits.push_back(copiedHit);
        }

        std::sort(sorted_hits.begin(), sorted_hits.end(), [](const auto& a, const auto& b) {
            return a.getTime() < b.getTime();
        });

        for (auto hit : sorted_hits) {
            auto hitTime = hit.getTime();
            check_hits_out()->push_back(hit);
        }

    }
};
