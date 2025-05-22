// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/Components/JOmniFactory.h>
// #include <JANA/Utils/JEventKey.h>

#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>

#include "services/io/podio/datamodel_glue.h"

struct addDummyEntryFactory : public JOmniFactory<addDummyEntryFactory> {
    JEventLevel m_factory_level;

    PodioInput<edm4hep::SimTrackerHit> hits_in {this};
    PodioOutput<edm4hep::SimTrackerHit> hits_out {this};

    void Configure() {
        m_factory_level = GetLevel();
    }

    void ChangeRun(int32_t /*run_nr*/) {
    }

    void Execute(int64_t run_number, uint64_t event_number) {

        LOG_INFO(GetLogger()) <<m_factory_level << LOG_END;
        
        std::vector<double> hitTimes;

        LOG_INFO(GetLogger()) << "CheeeeeeeeeeeeCKum Start AddDummyEntryFactory " <<  LOG_END;
        // hits_out()->setSubsetCollection(true);
        auto& hit_out = hits_out();
        for (const auto& hit : *hits_in) {
            edm4hep::MutableSimTrackerHit physSiBarrelHits;

            Double_t hitTime = hit.getTime();
            hitTimes.push_back(hitTime);
            
            // LOG_INFO(GetLogger()) << "HitCellID: " << hit.getCellID() << " hitTime: " << hitTime << LOG_END;

            physSiBarrelHits.setPosition({hit.getPosition().x, hit.getPosition().y, hit.getPosition().z});
            physSiBarrelHits.setTime(hitTime);
            physSiBarrelHits.setCellID(hit.getCellID());
            physSiBarrelHits.setEDep(hit.getEDep());
            physSiBarrelHits.setMomentum({hit.getMomentum().x, hit.getMomentum().y, hit.getMomentum().z});
            physSiBarrelHits.setPathLength(hit.getPathLength());
            physSiBarrelHits.setQuality(hit.getQuality());
            hit_out->push_back(physSiBarrelHits);
        }
        
        for (int iSplitTime = 0; iSplitTime < 5; ++iSplitTime) {
            Double_t splitTime = iSplitTime * 4; // microsecond (timeframe = 20microsecond)
            
            bool hitFound = false;
            for (const auto& hitTime : hitTimes) {
                if(splitTime < hitTime && hitTime < splitTime + 0.4) {
                    hitFound = true;
                    break;
                }
            }
            if(hitFound) continue;

            // Fill dummy information here, or copy is also possible
            edm4hep::MutableSimTrackerHit dummySiBarrelHits;
            dummySiBarrelHits.setPosition({0.0, 0.0, 0.0});
            dummySiBarrelHits.setTime(splitTime);
            dummySiBarrelHits.setCellID(12000000000000000000);
            // dummySiBarrelHits.setCellID(12000000000000000000);
            dummySiBarrelHits.setEDep(0.001);
            dummySiBarrelHits.setMomentum({0.0, 0.0, 0.0});
            dummySiBarrelHits.setPathLength(0.0);
            dummySiBarrelHits.setQuality(0);
        
            hit_out->push_back(dummySiBarrelHits);
        }

    }
};
