// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>

#include "services/io/podio/datamodel_glue.h"
#include <JANA/JEventUnfolder.h>

struct TimeframeSplitter : public JEventUnfolder {

  float m_timeframe_width = 2000.0; // ns
  float m_timesplit_width = 20.0;   // ns
  bool m_use_timeframe    = false;  // Use timeframes to split events, or use timeslices

  size_t m_event_number_ts = 0; // Event number for the current timeslice
  size_t m_event_number_orig = 0; // Event number for the current timeslice
  std::vector<Int_t> m_vTargetEvent; // List of original event numbers for each timeslice

  std::vector<std::string> m_simtrackerhit_collection_names = {
    "B0TrackerHits_aligned",       "BackwardMPGDEndcapHits_aligned", "DIRCBarHits_aligned",
    "DRICHHits_aligned",           "ForwardMPGDEndcapHits_aligned",  "ForwardOffMTrackerHits_aligned",
    "ForwardRomanPotHits_aligned", "LumiSpecTrackerHits_aligned",    "MPGDBarrelHits_aligned",
    "OuterMPGDBarrelHits_aligned", "RICHEndcapNHits_aligned",        "SiBarrelHits_aligned",
    "TOFBarrelHits_aligned",       "TOFEndcapHits_aligned",          "TaggerTrackerHits_aligned",
    "TrackerEndcapHits_aligned",   "VertexBarrelHits_aligned"
  };
  std::vector<std::string> m_simtrackerhit_collection_names_out = {
    "B0TrackerHits",       "BackwardMPGDEndcapHits", "DIRCBarHits",
    "DRICHHits",           "ForwardMPGDEndcapHits",  "ForwardOffMTrackerHits",
    "ForwardRomanPotHits", "LumiSpecTrackerHits",    "MPGDBarrelHits",
    "OuterMPGDBarrelHits", "RICHEndcapNHits",        "SiBarrelHits",
    "TOFBarrelHits",       "TOFEndcapHits",          "TaggerTrackerHits",
    "TrackerEndcapHits",   "VertexBarrelHits"
  };

  // std::vector<std::string> m_simtrackerhit_collection_names = {"SiBarrelHits_aligned"};
  // std::vector<std::string> m_simtrackerhit_collection_names_out = {"SiBarrelHits"};


  // std::vector<std::string> m_simtrackerhit_collection_names     = {"SiBarrelHits",
  //                                                                  "VertexBarrelHits"};
  // std::vector<std::string> m_simtrackerhit_collection_names_out = {"SiBarrelHits",
  //                                                                  "VertexBarrelHits"};

  std::vector<std::string> m_simcalorimeterhit_collection_names = {
      "B0ECalHits",      "EcalBarrelImagingHits", "EcalBarrelScFiHits",    "EcalEndcapNHits",
      "EcalEndcapPHits", "EcalEndcapPInsertHits", "EcalFarForwardZDCHits", "EcalLumiSpecHits",
      "HcalBarrelHits",  "HcalEndcapNHits",       "HcalEndcapPInsertHits", "HcalFarForwardZDCHits",
      "LFHCALHits",      "LumiDirectPCALHits"};

  std::vector<std::string> m_calohitcontribution_collection_names = {
      "B0ECalHitsContributions",
      "EcalBarrelImagingHitsContributions",
      "EcalBarrelScFiHitsContributions",
      "EcalEndcapNHitsContributions",
      "EcalEndcapPHitsContributions",
      "EcalEndcapPInsertHitsContributions",
      "EcalLumiSpecHitsContributions",
      "EcalFarForwardZDCHitsContributions",
      "HcalBarrelHitsContributions",
      "HcalEndcapNHitsContributions",
      "HcalEndcapPInsertHitsContributions",
      "HcalFarForwardZDCHitsContributions",
      "LFHCALHitsContributions",
      "LumiDirectPCALHitsContributions"};

  PodioInput<edm4hep::EventHeader> m_event_header_in{this,
                                                     {.name = "EventHeader", .is_optional = true}};
  PodioOutput<edm4hep::EventHeader> m_event_header_out{this, "EventHeader"};

  PodioInput<edm4hep::MCParticle> m_mcparticles_in{this, {.name = "MCParticles"}};
  PodioOutput<edm4hep::MCParticle> m_mcparticles_out{this, "MCParticles"};

  VariadicPodioInput<edm4hep::SimTrackerHit> m_simtrackerhits_in{
      this, {.names = m_simtrackerhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4hep::SimTrackerHit> m_simtrackerhits_out{
      this, m_simtrackerhit_collection_names_out};

  VariadicPodioInput<edm4hep::SimCalorimeterHit> m_simcalorimeterhits_in{
      this, {.names = m_simcalorimeterhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4hep::SimCalorimeterHit> m_simcalorimeterhits_out{
      this, m_simcalorimeterhit_collection_names};

  VariadicPodioInput<edm4hep::CaloHitContribution> m_calohitcontributions_in{
      this, {.names = m_calohitcontribution_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4hep::CaloHitContribution> m_calohitcontributions_out{
      this, m_calohitcontribution_collection_names};

  PodioOutput<edm4hep::EventHeader> m_event_header_ts_out{this, "EventHeader_TS"};
  PodioOutput<edm4hep::MCParticle> m_mcparticles_ts_out{this, "MCParticles_TS"};

  TimeframeSplitter() {
    SetTypeName(NAME_OF_THIS);
    SetParentLevel(JEventLevel::Timeslice);
    SetChildLevel(JEventLevel::PhysicsEvent);
  }

  std::vector<std::tuple<size_t, const edm4hep::SimTrackerHitCollection*, size_t>>
      m_hitStartIndices_simTracker;
  std::vector<std::tuple<size_t, const edm4hep::SimCalorimeterHitCollection*, size_t>>
      m_hitStartIndices_simCalorimeter;

  Result Unfold(const JEvent& parent, JEvent& child, int child_idx) override {
    if (child_idx == 0) {

      m_hitStartIndices_simTracker.clear();
      for (size_t detID = 0; detID < m_simtrackerhits_in().size(); ++detID) {
        const auto* detHitPtr = m_simtrackerhits_in().at(detID);
        m_hitStartIndices_simTracker.emplace_back(detID, detHitPtr, 0);
      }

      m_hitStartIndices_simCalorimeter.clear();
      m_hitStartIndices_simCalorimeter.resize(m_simcalorimeterhits_in().size(),
                                              std::make_tuple(0, nullptr, 0));
      for (size_t detID = 0; detID < m_simcalorimeterhits_in().size(); ++detID) {
        const auto* detHitPtr = m_simcalorimeterhits_in().at(detID);
        m_hitStartIndices_simCalorimeter.emplace_back(detID, detHitPtr, 0);
      }
      m_event_number_orig++;
    }

    LOG_INFO(GetLogger()) << "Running TimeframeSplitter::Unfold() on timeslice #"
                          << parent.GetEventNumber() << LOG_END;

    // == s == Internal Interval loop  ========================================================-
    bool bAdmitedInterval = false;
    float iTimeSlice      = m_timesplit_width * child_idx;
    float eTimeSlice      = m_timesplit_width * (child_idx + 1.0);
    while (eTimeSlice <= m_timeframe_width) {
      // LOG_INFO(GetLogger()) << "TimeframeSplitter: timeslice " << parent.GetEventNumber() << " timeStamp " << timeStamp << LOG_END;
      iTimeSlice = m_timesplit_width * child_idx;
      eTimeSlice = m_timesplit_width * (child_idx + 1.0);
      // std::cout << "child_idx = " << child_idx << ":: TimeframeSplitter: timeslice "
      //           << parent.GetEventNumber() << " iTimeSlice " << iTimeSlice << " eTimeSlice "
      //           << eTimeSlice << std::endl;

      std::vector<std::unique_ptr<edm4hep::SimTrackerHitCollection>> tempAllDetectorSimTrackerHits;

      // Loop through SimTrackerHit collections and split them into time slice      
      for (auto& [detID, detHitPtr, start_index] : m_hitStartIndices_simTracker) {
          auto tempSimTrackerHits = std::make_unique<edm4hep::SimTrackerHitCollection>();          
          bool bAllScan = true; // Scan all hits in the collection until we find a hit that is later than the end of the time slice
          
          if (detHitPtr != nullptr){
            tempSimTrackerHits->setSubsetCollection(true);

            for (size_t hitID = start_index; hitID < detHitPtr->size(); ++hitID) {
                const auto& hit = detHitPtr->at(hitID);
                auto hitTime = hit.getTime();

                if (hitTime >= eTimeSlice) {
                    start_index = hitID;
                    bAllScan = false;
                    break;
                }
                tempSimTrackerHits->push_back(hit);
            }
            if(bAllScan) start_index = detHitPtr->size();
          }
          tempAllDetectorSimTrackerHits.push_back(std::move(tempSimTrackerHits));
      }

      // Loop through SimTrackerHit collections and split them into time slice
      for (auto& [detID, detHitPtr, start_index] : m_hitStartIndices_simCalorimeter) {
        auto& coll_out = m_simcalorimeterhits_out().at(detID);
        coll_out->setSubsetCollection(true);
        if (detHitPtr == nullptr)
          continue;
        bool bAllScan = true;
        for (size_t hitID = start_index; hitID < detHitPtr->size(); ++hitID) {
          const auto& hit = detHitPtr->at(hitID);

          auto hitContributions = hit.getContributions();
          bool contributeFlag   = false;
          for (const auto& contribution : hitContributions) {
            auto hitTime = contribution.getTime();
            if (hitTime >= eTimeSlice) {
              start_index    = hitID;
              bAllScan       = false;
              contributeFlag = true;
              // std::cout << "ChecKuma BreaKumaaaaaa!! : " << hitTime << " hitId: " << hitID << ", start_index: " << start_index << std::endl;
              break;
            }

            auto& coll_out_contribution = m_calohitcontributions_out().at(detID);
            coll_out_contribution->setSubsetCollection(true);

            coll_out_contribution->push_back(contribution);
          }
          if(bAllScan) start_index = detHitPtr->size();
            
      }      
      
      // for(const auto& tempSimTrackerHits : tempAllDetectorSimTrackerHits) {
      //     if(!tempSimTrackerHits->empty()){
      //       bAdmitedInterval = true;            
      //       break;
      //     }
      // }

      bool m_bDetTriggerLists[17] = {};
      for(size_t iDet = 0; iDet < 17; iDet++){
        if (tempAllDetectorSimTrackerHits[iDet]->empty()) continue;
        m_bDetTriggerLists[iDet] = true;
      }
      bool m_bCombinTriggerLists[3] = {};
      if(m_bDetTriggerLists[1]  && m_bDetTriggerLists[15]) m_bCombinTriggerLists[0] = true; // Trigger1
      if(m_bDetTriggerLists[12]&&(m_bDetTriggerLists[8]||m_bDetTriggerLists[9])) m_bCombinTriggerLists[1] = true; // Trigger2
      if(m_bDetTriggerLists[5]  && m_bDetTriggerLists[15]) m_bCombinTriggerLists[2] = true; // Trigger3
      for(size_t iTrig = 0; iTrig < 3; iTrig++){
        if(m_bCombinTriggerLists[iTrig]){
          bAdmitedInterval = true;
          break;
        }
      }

      if(bAdmitedInterval){
          Int_t detID = 0;
          for(auto& tempSimTrackerHits : tempAllDetectorSimTrackerHits) {

            auto& coll_out = m_simtrackerhits_out().at(detID);
            coll_out->setSubsetCollection(true);
            coll_out = std::move(tempSimTrackerHits);     
            detID++;
          }

          // For now, a one-to-one relationship between timeslices and events
          child.SetEventNumber(parent.GetEventNumber());
          child.SetRunNumber(parent.GetRunNumber());
          
          edm4hep::MutableEventHeader event_header_ts;
          event_header_ts.setRunNumber(m_event_number_ts * 10000 + child_idx);
          event_header_ts.setEventNumber(m_event_number_ts);
          event_header_ts.setTimeStamp(iTimeSlice+1);
          event_header_ts.setWeight(m_event_number_ts * 10000 + child_idx); // Just a dummy weight for now
          m_event_header_ts_out()->push_back(event_header_ts);
          m_event_number_ts++;
          
          // Insert an EventHeader object into the physics event
          // For now this is just a ref to the timeslice header
          m_event_header_out()->setSubsetCollection(true);
          m_event_header_out()->push_back(m_event_header_in()->at(0));

        // For now, a one-to-one relationship between timeslices and events
        child.SetEventNumber(parent.GetEventNumber());
        child.SetRunNumber(parent.GetRunNumber());

          // == s == Basic container for simulatin data (but not related to data)  =========
          // Insert MCParticles into the physics event
          m_mcparticles_out()->setSubsetCollection(true);
          for (const auto& mcparticle : *m_mcparticles_in) {
            std::cout << "MC particle status: " << mcparticle.getGeneratorStatus() << std::endl;
            m_mcparticles_out()->push_back(mcparticle);
          }

          // edm4hep::MutableMCParticle mcparticles_ts;
          // m_mcparticles_ts_out()->setSubsetCollection(true);
          for (const auto& mcparticle : *m_mcparticles_in) {
            auto& mc_cp = m_mcparticles_ts_out()->create();

            mc_cp.setPDG(mcparticle.getPDG());
            mc_cp.setGeneratorStatus(mcparticle.getGeneratorStatus());
            mc_cp.setSimulatorStatus(mcparticle.getSimulatorStatus());
            mc_cp.setCharge(mcparticle.getCharge());
            mc_cp.setTime(mcparticle.getTime());
            mc_cp.setMass(mcparticle.getMass());

            mc_cp.setMomentum(mcparticle.getMomentum());
            mc_cp.setVertex(mcparticle.getVertex());
            mc_cp.setEndpoint(mcparticle.getEndpoint());
            mc_cp.setSpin(mcparticle.getSpin());
            mc_cp.setColorFlow(mcparticle.getColorFlow());


            // mcparticles_ts->push_back(mcparticle);
            // m_mcparticles_ts_out()->push_back(std::move(mcparticle));
          }

          
          // == e == Basic container for simulatin data (but not related to data)  =========
    
          if(child_idx > 0) m_vTargetEvent.push_back(m_event_number_orig);

        // == s == Basic container for simulatin data (but not related to data)  =========
        // Insert MCParticles into the physics event
        m_mcparticles_out()->setSubsetCollection(true);
        for (const auto& mcparticle : *m_mcparticles_in) {
          // TODO: Decide which of these belong to this physics event
          m_mcparticles_out()->push_back(mcparticle);
        }
        // == e == Basic container for simulatin data (but not related to data)  =========

        break; // Break out of the interval loop if we have already admitted this interval
      }

      child_idx++;
    }
    // == s == Internal Interval loop  ========================================================-

    // Produce exactly one physics event per timeframe for now
    // return JEventUnfolder::Result::NextChildNextParent;
    if(eTimeSlice + 1 > m_timeframe_width && !bAdmitedInterval){

      return Result::KeepChildNextParent;
    }
    return (eTimeSlice + 1 > m_timeframe_width) ? Result::NextChildNextParent
                                                : Result::NextChildKeepParent;
  }
};
