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
  float m_timeframe_width = 20.0; // us

  // std::vector<std::string> m_simtrackerhit_collection_names = {
  //     "B0TrackerHits",       "BackwardMPGDEndcapHits", "DIRCBarHits",
  //     "DRICHHits",           "ForwardMPGDEndcapHits",  "ForwardOffMTrackerHits",
  //     "ForwardRomanPotHits", "LumiSpecTrackerHits",    "MPGDBarrelHits",
  //     "OuterMPGDBarrelHits", "RICHEndcapNHits",        "SiBarrelHits",
  //     "TOFBarrelHits",       "TOFEndcapHits",          "TaggerTrackerHits",
  //     "TrackerEndcapHits",   "VertexBarrelHits"};

  std::vector<std::string> m_simtrackerhit_collection_names = {"SiBarrelHits"};

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
      this, m_simtrackerhit_collection_names};

  VariadicPodioInput<edm4hep::SimCalorimeterHit> m_simcalorimeterhits_in{
      this, {.names = m_simcalorimeterhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4hep::SimCalorimeterHit> m_simcalorimeterhits_out{
      this, m_simcalorimeterhit_collection_names};

  VariadicPodioInput<edm4hep::CaloHitContribution> m_calohitcontributions_in{
      this, {.names = m_calohitcontribution_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4hep::CaloHitContribution> m_calohitcontributions_out{
      this, m_calohitcontribution_collection_names};

  // Kuma Hit checker
  // PodioInput<edm4hep::SimTrackerHit> m_check_hits_in{this, {.name = "ts_checked_hits", .is_optional = true}};

  TimeframeSplitter() {
    SetTypeName(NAME_OF_THIS);
    SetParentLevel(JEventLevel::Timeslice);
    SetChildLevel(JEventLevel::PhysicsEvent);
  }

  Result Unfold(const JEvent& parent, JEvent& child, int child_idx) override {
    // float timeStamp = parent.GetEventTimeStamp();
    // LOG_INFO(GetLogger()) << "TimeframeSplitter: timeslice " << parent.GetEventNumber() << " timeStamp " << timeStamp << LOG_END;
    float iTimeSlice = 4.0 * child_idx; // 4.0 us per timeslice ( = 20 us / 5)
    float eTimeSlice = 4.0 * (child_idx + 1.0);
    std::cout << "child_idx = " << child_idx << ":: TimeframeSplitter: timeslice "
              << parent.GetEventNumber() << " iTimeSlice " << iTimeSlice << " eTimeSlice "
              << eTimeSlice << std::endl;

    LOG_INFO(GetLogger()) << "Running TimeframeSplitter::Unfold() on timeslice #"
                          << parent.GetEventNumber() << LOG_END;

    // For now, a one-to-one relationship between timeslices and events
    child.SetEventNumber(parent.GetEventNumber());
    child.SetRunNumber(parent.GetRunNumber());

    // Insert an EventHeader object into the physics event
    // For now this is just a ref to the timeslice header
    m_event_header_out()->setSubsetCollection(true);
    m_event_header_out()->push_back(m_event_header_in()->at(0));

    // Insert MCParticles into the physics event
    m_mcparticles_out()->setSubsetCollection(true);
    for (const auto& mcparticle : *m_mcparticles_in) {
      // TODO: Decide which of these belong to this physics event
      m_mcparticles_out()->push_back(mcparticle);
    }

    // Insert SimTrackerHits into the physics event
    for (size_t coll_index = 0; coll_index < m_simtrackerhits_in().size(); ++coll_index) {
      const auto* coll_in = m_simtrackerhits_in().at(coll_index);
      auto& coll_out      = m_simtrackerhits_out().at(coll_index);

      if (coll_in != nullptr) {

        coll_out->setSubsetCollection(true);
        for (const auto& hit : *coll_in) {
          // Get hit time
          auto hitTime = hit.getTime();
          // Separate a time frame into 5 time slices (one time frame = 20 us)
          if (hitTime < iTimeSlice || hitTime >= eTimeSlice)
            continue;

          coll_out->push_back(hit);
        }
      }
    }

    // Insert SimCalorimeterHits into the physics event
    for (size_t coll_index = 0; coll_index < m_simcalorimeterhits_in().size(); ++coll_index) {
      const auto* coll_in = m_simcalorimeterhits_in().at(coll_index);
      auto& coll_out      = m_simcalorimeterhits_out().at(coll_index);
      if (coll_in != nullptr) {
        coll_out->setSubsetCollection(true);
        for (const auto& hit : *coll_in) {
          // TODO: Decide which of these belong to this physics event
          coll_out->push_back(hit);
        }
      }
    }

    // Insert CaloHitContributions into the physics event
    for (size_t coll_index = 0; coll_index < m_calohitcontributions_in().size(); ++coll_index) {
      const auto* coll_in = m_calohitcontributions_in().at(coll_index);
      auto& coll_out      = m_calohitcontributions_out().at(coll_index);
      if (coll_in != nullptr) {
        coll_out->setSubsetCollection(true);
        for (const auto& contrib : *coll_in) {
          // TODO: Decide which of these belong to this physics event
          coll_out->push_back(contrib);
        }
      }
    }

    // Produce exactly one physics event per timeframe for now
    // return JEventUnfolder::Result::NextChildNextParent;
    return (eTimeSlice + 1 > m_timeframe_width) ? Result::NextChildNextParent
                                                : Result::NextChildKeepParent;
  }
};
