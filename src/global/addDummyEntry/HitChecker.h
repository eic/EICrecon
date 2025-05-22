// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/Components/JOmniFactory.h>
// #include <JANA/Utils/JEventKey.h>

#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>

#include "services/io/podio/datamodel_glue.h"

struct HitChecker : public JOmniFactory<HitChecker> {
    JEventLevel m_factory_level;

    PodioInput<edm4hep::SimTrackerHit> check_hits_in {this};
    PodioOutput<edm4hep::SimTrackerHit> check_hits_out {this};

    void Configure() {
        m_factory_level = GetLevel();
    }

    void ChangeRun(int32_t /*run_nr*/) {
    }

    void Execute(int64_t run_number, uint64_t event_number) {

        LOG_INFO(GetLogger()) <<m_factory_level << LOG_END;

        // if (m_factory_level == JEventLevel::Timeslice) {
        //     auto& ts = GetParent(JEventLevel::Timeslice);
        //     auto ts_nr = ts.GetEventNumber();
        //     LOG_INFO(GetLogger()) << "HitChecker: Timeslice " << ts_nr << LOG_END;
        // }
        
        check_hits_out()->setSubsetCollection(true);
        auto& coll_out = check_hits_out();
        for (auto hit : *check_hits_in) {
            auto posi = hit.getPosition();
            std::cout << "Hit in timeslice: " << hit.getCellID() << " " << posi.x << " " << posi.y << " " << posi.z << std::endl;
            std::cout << "Time in timeslice: " << hit.getTime() << std::endl;
            coll_out->push_back(hit);
        }

    }
};
