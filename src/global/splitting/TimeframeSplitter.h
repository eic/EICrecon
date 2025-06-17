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

  float m_timeframe_width = 20.0; // ns
  float m_timesplit_width = 4.0; // ns
  bool m_use_timeframe    = false;  // Use timeframes to split events, or use timeslices

  // std::vector<std::string> m_simtrackerhit_collection_names = {
  //     "B0TrackerHits_aligned",       "BackwardMPGDEndcapHits_aligned", "DIRCBarHits_aligned",
  //     "DRICHHits_aligned",           "ForwardMPGDEndcapHits_aligned",  "ForwardOffMTrackerHits_aligned",
  //     "ForwardRomanPotHits_aligned", "LumiSpecTrackerHits_aligned",    "MPGDBarrelHits_aligned",
  //     "OuterMPGDBarrelHits_aligned", "RICHEndcapNHits_aligned",        "SiBarrelHits_aligned",
  //     "TOFBarrelHits_aligned",       "TOFEndcapHits_aligned",          "TaggerTrackerHits_aligned",
  //     "TrackerEndcapHits_aligned",   "VertexBarrelHits_aligned"};

  // std::vector<std::string> m_simtrackerhit_collection_names_out = {
  //     "B0TrackerHits",       "BackwardMPGDEndcapHits", "DIRCBarHits",
  //     "DRICHHits",           "ForwardMPGDEndcapHits",  "ForwardOffMTrackerHits",
  //     "ForwardRomanPotHits", "LumiSpecTrackerHits",    "MPGDBarrelHits",
  //     "OuterMPGDBarrelHits", "RICHEndcapNHits",        "SiBarrelHits",
  //     "TOFBarrelHits",       "TOFEndcapHits",          "TaggerTrackerHits",
  //     "TrackerEndcapHits",   "VertexBarrelHits"};

  std::vector<std::string> m_simtrackerhit_collection_names = {"SiBarrelHits_aligned"};
  std::vector<std::string> m_simtrackerhit_collection_names_out = {"SiBarrelHits"};

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
      m_hitStartIndices_simTracker.resize(m_simtrackerhits_in().size(),
                                          std::make_tuple(0, nullptr, 0));
      for (size_t detID = 0; detID < m_simtrackerhits_in().size(); ++detID) {
        const auto* detHitPtr = m_simtrackerhits_in().at(detID);
        if (detHitPtr == nullptr)
          continue;
        m_hitStartIndices_simTracker.emplace_back(detID, detHitPtr, 0);
      }

      m_hitStartIndices_simCalorimeter.clear();
      m_hitStartIndices_simCalorimeter.resize(m_simcalorimeterhits_in().size(),
                                              std::make_tuple(0, nullptr, 0));
      for (size_t detID = 0; detID < m_simcalorimeterhits_in().size(); ++detID) {
        const auto* detHitPtr = m_simcalorimeterhits_in().at(detID);
        if (detHitPtr == nullptr)
          continue;
        m_hitStartIndices_simCalorimeter.emplace_back(detID, detHitPtr, 0);
      }
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
      // std::cout << "child_idx = " << child_idx << ":: TimeframeSplitter: timeslice "
      //           << parent.GetEventNumber() << " iTimeSlice " << iTimeSlice << " eTimeSlice "
      //           << eTimeSlice << std::endl;
      
      std::vector<std::unique_ptr<edm4hep::SimTrackerHitCollection>> tempAllDetectorSimTrackerHits;
      // Loop through SimTrackerHit collections and split them into time slice      
      for (auto& [detID, detHitPtr, start_index] : m_hitStartIndices_simTracker) {          
          if (detHitPtr == nullptr) continue;

          bool bAllScan = true; // Scan all hits in the collection until we find a hit that is later than the end of the time slice
          auto tempSimTrackerHits = std::make_unique<edm4hep::SimTrackerHitCollection>(); // ??? Kuma test
          tempSimTrackerHits->setSubsetCollection(true);

          for (size_t hitID = start_index; hitID < detHitPtr->size(); ++hitID) {
              const auto& hit = detHitPtr->at(hitID);
              auto hitTime = hit.getTime();
                            
              if (hitTime >= eTimeSlice) {
                  start_index = hitID;
                  bAllScan = false;
                  // std::cout << "ChecKuma BreaKumaaaaaa!! : " << hitTime << " hitId: " << hitID << ", start_index: " << start_index << std::endl;
                  break;
              }
              tempSimTrackerHits->push_back(hit);
          }
          tempAllDetectorSimTrackerHits.push_back(std::move(tempSimTrackerHits));
          if(bAllScan) start_index = detHitPtr->size();
          
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
                  // std::cout << "ChecKuma BreaKumaaaaaa!! : " << hitTime << " hitId: " << hitID << ", start_index: " << start_index << std::endl;
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
      
      for(const auto& tempSimTrackerHits : tempAllDetectorSimTrackerHits) {
          if(!tempSimTrackerHits->empty()){
            bAdmitedInterval = true;            
            break;
          }
      }

      if(bAdmitedInterval){
          Int_t detID = 0;
          for(auto& tempSimTrackerHits : tempAllDetectorSimTrackerHits) {
            
            if(!tempSimTrackerHits->empty()){
              // std::cout << "ChecKuma: TimeframeSplitter: timeslice "
              //           << parent.GetEventNumber() << " child_idx " << child_idx
              //           << " detID " << detID << " has hits in time slice: "
              //           << tempSimTrackerHits->size() << std::endl;
              auto& coll_out = m_simtrackerhits_out().at(detID);              
              coll_out->setSubsetCollection(true);

              coll_out = std::move(tempSimTrackerHits);
              detID++;
            }
          }
          
          // For now, a one-to-one relationship between timeslices and events
          child.SetEventNumber(parent.GetEventNumber());
          child.SetRunNumber(parent.GetRunNumber());
          
          // Insert an EventHeader object into the physics event
          // For now this is just a ref to the timeslice header
          m_event_header_out()->setSubsetCollection(true);
          m_event_header_out()->push_back(m_event_header_in()->at(0));


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
      // std::cout << "CheeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeecKuma child_idx: " << child_idx
      //         << ", eTimeSlice: " << eTimeSlice << ", m_timeframe_width: " << m_timeframe_width
      //         << std::endl;
      return Result::KeepChildNextParent;
    }
    return (eTimeSlice + 1 > m_timeframe_width) ? Result::NextChildNextParent
                                                : Result::NextChildKeepParent;

  
  }
};
