// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <sys/resource.h>


#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>

#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/TrackerHitCollection.h>

#include "services/io/podio/datamodel_glue.h"
#include <JANA/JEventUnfolder.h>

struct TimeframeSplitter : public JEventUnfolder {

  Parameter<float> timeframe_width{this, "timeframe_width", 2000.0, "Width of each timeframe in ns"};
  Parameter<float> timesplit_width{this, "timesplit_width", 2000.0, "Width of each timeslice in ns"};
  // float m_timeframe_width = 2000.0; // ns
  // float m_timesplit_width = 2000.0; // ns
  bool m_use_timeframe    = false;  // Use timeframes to split events, or use timeslices

  size_t m_event_number_ts = 0; // Event number for the current timeslice
  size_t m_event_number_orig = 0; // Event number for the current timeslice
  std::vector<Int_t> m_vTargetEvent; // List of original event numbers for each timeslice

  // std::vector<std::string> m_simtrackerhit_collection_names = {
  //   "B0TrackerHits",       "BackwardMPGDEndcapHits", "DIRCBarHits",
  //   "DRICHHits",           "ForwardMPGDEndcapHits",  "ForwardOffMTrackerHits",
  //   "ForwardRomanPotHits", "LumiSpecTrackerHits",    "MPGDBarrelHits",
  //   "OuterMPGDBarrelHits", "RICHEndcapNHits",        "SiBarrelHits",
  //   "TOFBarrelHits",       "TOFEndcapHits",          "TaggerTrackerHits",
  //   "TrackerEndcapHits",   "VertexBarrelHits"
  // };
  // std::vector<std::string> m_simtrackerhit_collection_names_out = {
  //   "B0TrackerHits",       "BackwardMPGDEndcapHits", "DIRCBarHits",
  //   "DRICHHits",           "ForwardMPGDEndcapHits",  "ForwardOffMTrackerHits",
  //   "ForwardRomanPotHits", "LumiSpecTrackerHits",    "MPGDBarrelHits",
  //   "OuterMPGDBarrelHits", "RICHEndcapNHits",        "SiBarrelHits",
  //   "TOFBarrelHits",       "TOFEndcapHits",          "TaggerTrackerHits",
  //   "TrackerEndcapHits",   "VertexBarrelHits"
  // };
  
  std::vector<std::string> m_simtrackerhit_collection_names = {
    "B0TrackerRecHits_TK_aligned",       "BackwardMPGDEndcapRecHits_TK_aligned", "DIRCBarRecHits_TK_aligned",
    "DRICHRecHits_TK_aligned",           "ForwardMPGDEndcapRecHits_TK_aligned",  "ForwardOffMTrackerRecHits_TK_aligned",
    "ForwardRomanPotRecHits_TK_aligned", "LumiSpecTrackerRecHits_TK_aligned",    "MPGDBarrelRecHits_TK_aligned",
    "OuterMPGDBarrelRecHits_TK_aligned", "RICHEndcapNRecHits_TK_aligned",        "SiBarrelTrackerRecHits_TK_aligned",
    "TOFBarrelRecHits_TK_aligned",       "TOFEndcapRecHits_TK_aligned",          "TaggerTrackerRecHits_TK_aligned",
    "SiEndcapTrackerRecHits_TK_aligned",   "SiBarrelVertexRecHits_TK_aligned"
  };
  std::vector<std::string> m_simtrackerhit_collection_names_out = {
    "B0TrackerRecHits",       "BackwardMPGDEndcapRecHits", "DIRCBarRecHits",
    "DRICHRecHits",           "ForwardMPGDEndcapRecHits",  "ForwardOffMTrackerRecHits",
    "ForwardRomanPotRecHits", "LumiSpecTrackerRecHits",    "MPGDBarrelRecHits",
    "OuterMPGDBarrelRecHits", "RICHEndcapNRecHits",        "SiBarrelTrackerRecHits",
    "TOFBarrelRecHits",       "TOFEndcapRecHits",          "TaggerTrackerRecHits",
    "SiEndcapTrackerRecHits",   "SiBarrelVertexRecHits"
  };


  // std::vector<std::string> m_simtrackerhit_collection_names = {"SiBarrelRecHits_TK"};
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

  // VariadicPodioInput<edm4hep::SimTrackerHit> m_simtrackerhits_in{
  //     this, {.names = m_simtrackerhit_collection_names, .is_optional = true}};
  // VariadicPodioOutput<edm4hep::SimTrackerHit> m_simtrackerhits_out{
  //     this, m_simtrackerhit_collection_names_out};

  VariadicPodioInput<edm4eic::TrackerHit> m_simtrackerhits_in{
      this, {.names = m_simtrackerhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4eic::TrackerHit> m_simtrackerhits_out{
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

  TimeframeSplitter() {
    SetTypeName(NAME_OF_THIS);
    SetParentLevel(JEventLevel::Timeslice);
    SetChildLevel(JEventLevel::PhysicsEvent);
  }

  // std::vector<std::tuple<size_t, const edm4hep::SimTrackerHitCollection*, size_t>>
  //     m_hitStartIndices_simTracker;

  std::vector<std::tuple<size_t, const edm4eic::TrackerHitCollection*, size_t>>
      m_hitStartIndices_simTracker;
  std::vector<std::tuple<size_t, const edm4hep::SimCalorimeterHitCollection*, size_t>>
      m_hitStartIndices_simCalorimeter;

  Result Unfold(const JEvent& parent, JEvent& child, int child_idx) override {

    float m_timeframe_width = timeframe_width();
    float m_timesplit_width = timesplit_width();

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
    float iTimeSlice = m_timesplit_width * child_idx;
    float eTimeSlice = m_timesplit_width * (child_idx + 1.0);
    while (eTimeSlice <= m_timeframe_width){      
      // LOG_INFO(GetLogger()) << "TimeframeSplitter: timeslice " << parent.GetEventNumber() << " timeStamp " << timeStamp << LOG_END;
      iTimeSlice = m_timesplit_width * child_idx;
      eTimeSlice = m_timesplit_width * (child_idx + 1.0);
      
      std::vector<std::unique_ptr<edm4eic::TrackerHitCollection>> tempAllDetectorSimTrackerHits;
      // std::vector<std::unique_ptr<edm4hep::SimTrackerHitCollection>> tempAllDetectorSimTrackerHits;

      // Loop through SimTrackerHit collections and split them into time slice      
      for (auto& [detID, detHitPtr, start_index] : m_hitStartIndices_simTracker) {
          // auto tempSimTrackerHits = std::make_unique<edm4hep::SimTrackerHitCollection>();
          auto tempSimTrackerHits = std::make_unique<edm4eic::TrackerHitCollection>();
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
          if (detHitPtr == nullptr) continue;
          bool bAllScan = true;
          for (size_t hitID = start_index; hitID < detHitPtr->size(); ++hitID) {
              const auto& hit = detHitPtr->at(hitID);

              auto hitContributions = hit.getContributions();
              bool contributeFlag = false;
              for(const auto& contribution : hitContributions){
                auto hitTime = contribution.getTime();
                if (hitTime >= eTimeSlice) {
                  start_index = hitID;
                  bAllScan = false;
                  contributeFlag = true;
                  break;
                }

                auto& coll_out_contribution = m_calohitcontributions_out().at(detID);
                coll_out_contribution->setSubsetCollection(true);

                coll_out_contribution->push_back(contribution);
              }
              if(contributeFlag) break;              

              coll_out->push_back(hit);
          }
          if(bAllScan) start_index = detHitPtr->size();
            
      }      
      

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
            // coll_out->setSubsetCollection(true);
            // coll_out = std::move(tempSimTrackerHits);
            for (auto&& hit : *tempSimTrackerHits) {
              coll_out->push_back(hit.clone(true)); 
            }
            tempSimTrackerHits->clear();
            detID++;
          }

          // For now, a one-to-one relationship between timeslices and events
          child.SetEventNumber(parent.GetEventNumber());
          child.SetRunNumber(parent.GetRunNumber());
          
          edm4hep::MutableEventHeader event_header_ts;
          event_header_ts.setRunNumber(m_event_number_ts * 10000 + child_idx);
          event_header_ts.setEventNumber(m_event_number_ts);
          event_header_ts.setTimeStamp(iTimeSlice+1);
          
          // event_header_ts.setWeight(memoryUsage); // Just a dummy weight for now
          m_event_header_ts_out()->push_back(event_header_ts);
          m_event_number_ts++;
          
          // Insert an EventHeader object into the physics event
          // For now this is just a ref to the timeslice header
          m_event_header_out()->setSubsetCollection(true);
          m_event_header_out()->push_back(m_event_header_in()->at(0));

          // == s == Basic container for simulatin data (but not related to data)  =========        
          for (const auto& mcparticle : *m_mcparticles_in) {
            m_mcparticles_out()->push_back(mcparticle.clone(true));            
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
