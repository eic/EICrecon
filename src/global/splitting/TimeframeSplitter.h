// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <sys/resource.h>

#include "TMath.h"

#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociation.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>

#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/TrackerHitCollection.h>

#include "services/io/podio/datamodel_glue.h"
#include <JANA/JEventUnfolder.h>

struct TimeframeSplitter : public JEventUnfolder {

  Parameter<float> timeframe_width{this, "timeframe_width", 2000.0,
                                   "Width of each timeframe in ns"};
  Parameter<float> timesplit_width{this, "timesplit_width", 2000.0,
                                   "Width of each timeslice in ns"};
  Parameter<float> timeResolution_Silicon{this, "timeResolution_Silicon", 2000.0,
                                          "time resolution of Silicon detector in ns"};
  Parameter<float> timeResolution_MPGD{this, "timeResolution_MPGD = 10.0", 10.0,
                                       "time resolution of MPGD detector in ns"};
  Parameter<float> timeResolution_TOF{this, "timeResolution_TOF = 1.0", 1.0,
                                      "time resolution of TOF detector in ns"};
  // float m_timeframe_width = 2000.0; // ns
  // float m_timesplit_width = 2000.0; // ns
  bool m_use_timeframe = false; // Use timeframes to split events, or use timeslices

  size_t m_event_number_ts   = 0;    // Event number for the current timeslice
  size_t m_event_number_orig = 0;    // Event number for the current timeslice
  std::vector<Int_t> m_vTargetEvent; // List of original event numbers for each timeslice

  std::vector<std::string> m_simtrackerhit_collection_names = {
      "TOFBarrelRecHits_TK_aligned",
      "TOFEndcapRecHits_TK_aligned",
      "MPGDBarrelRecHits_TK_aligned",
      "OuterMPGDBarrelRecHits_TK_aligned",
      "BackwardMPGDEndcapRecHits_TK_aligned",
      "ForwardMPGDEndcapRecHits_TK_aligned",
      "SiBarrelVertexRecHits_TK_aligned",
      "SiBarrelTrackerRecHits_TK_aligned",
      "SiEndcapTrackerRecHits_TK_aligned",
      "TaggerTrackerRecHits_TK_aligned",
      "B0TrackerRecHits_TK_aligned",
      "DIRCBarRecHits_TK_aligned",
      "DRICHRecHits_TK_aligned",
      "ForwardOffMTrackerRecHits_TK_aligned",
      "ForwardRomanPotRecHits_TK_aligned",
      "LumiSpecTrackerRecHits_TK_aligned",
      "RICHEndcapNRecHits_TK_aligned"};
  std::vector<std::string> m_simtrackerhit_collection_names_out = {
      "TOFBarrelRecHits",       "TOFEndcapRecHits",          "MPGDBarrelRecHits",
      "OuterMPGDBarrelRecHits", "BackwardMPGDEndcapRecHits", "ForwardMPGDEndcapRecHits",
      "SiBarrelVertexRecHits",  "SiBarrelTrackerRecHits",    "SiEndcapTrackerRecHits",
      "TaggerTrackerRecHits",   "B0TrackerRecHits",          "DIRCBarRecHits",
      "DRICHRecHits",           "ForwardOffMTrackerRecHits", "ForwardRomanPotRecHits",
      "LumiSpecTrackerRecHits", "RICHEndcapNRecHits"};

  std::vector<std::string> m_simtrackerhitAsso_collection_names = {
      "TOFBarrelRawHitAssociations_TK",
      "TOFEndcapRawHitAssociations_TK",
      "MPGDBarrelRawHitAssociations_TK",
      "OuterMPGDBarrelRawHitAssociations_TK",
      "BackwardMPGDEndcapRawHitAssociations_TK",
      "ForwardMPGDEndcapRawHitAssociations_TK",
      "SiBarrelVertexRawHitAssociations_TK",
      "SiBarrelRawHitAssociations_TK",
      "SiEndcapTrackerRawHitAssociations_TK",
      "TaggerTrackerRawHitAssociations_TK",
      "B0TrackerRawHitAssociations_TK",
      "DIRCBarRawHitAssociations_TK",
      "DRICHRawHitAssociations_TK",
      "ForwardOffMTrackerRawHitAssociations_TK",
      "ForwardRomanPotRawHitAssociations_TK",
      "LumiSpecTrackerRawHitAssociations_TK",
      "RICHEndcapNRawHitAssociations_TK"};
  std::vector<std::string> m_simtrackerhitAsso_collection_names_out = {
      "TOFBarrelRawHitAssociations",
      "TOFEndcapRawHitAssociations",
      "MPGDBarrelRawHitAssociations",
      "OuterMPGDBarrelRawHitAssociations",
      "BackwardMPGDEndcapRawHitAssociations",
      "ForwardMPGDEndcapRawHitAssociations",
      "SiBarrelVertexRawHitAssociations",
      "SiBarrelRawHitAssociations",
      "SiEndcapTrackerRawHitAssociations",
      "TaggerTrackerRawHitAssociations",
      "B0TrackerRawHitAssociations",
      "DIRCBarRawHitAssociations",
      "DRICHRawHitAssociations",
      "ForwardOffMTrackerRawHitAssociations",
      "ForwardRomanPotRawHitAssociations",
      "LumiSpecTrackerRawHitAssociations",
      "RICHEndcapNRawHitAssociations"};

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

  VariadicPodioInput<edm4eic::TrackerHit> m_simtrackerhits_in{
      this, {.names = m_simtrackerhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4eic::TrackerHit> m_simtrackerhits_out{
      this, m_simtrackerhit_collection_names_out};

  VariadicPodioInput<edm4eic::MCRecoTrackerHitAssociation> m_simtrackerhitsAsso_in{
      this, {.names = m_simtrackerhitAsso_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4eic::MCRecoTrackerHitAssociation> m_simtrackerhitsAsso_out{
      this, m_simtrackerhitAsso_collection_names_out};

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

  // == Global Variables =======================
  size_t m_triggerDetSize = 9; // Number of detectors used for triggering
  // Int_t m_detId[10] = {12, 13, 1, 4, 8, 9, 11, 14, 15, 16}; // TOF and MPGD, Silicon excluded
  Int_t m_detId[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8}; // TOF and MPGD, Silicon excluded

  // float timeResolution_Silicon = 2000.0; // time resolution [ns]
  // float timeResolution_MPGD = 10.0; // time resolution [ns]
  // float timeResolution_TOF = 0.030; // time resolution [ns]
  // float timeResolution_TOF = 1.0; // time resolution [ns]

  bool bInitialLoop = true;
  std::vector<std::vector<unsigned int>> m_vOrigHitId;
  std::vector<std::vector<unsigned int>> m_vSameTSHitId;

  unsigned int startHitPoint[9] = {0};
  bool m_bDetLastHits[9]        = {false, false, false, false, false, false, false, false, false};

  bool m_bOnceTriggered        = false;
  bool m_bScanedAllTimeWindows = false;

  Int_t targetDetId                     = 0;
  size_t iTimeSlice                     = 0;
  std::vector<Double_t> m_vPhysCooTimes = {};
  // == Global Variables =======================

  Result Unfold(const JEvent& parent, JEvent& child, int child_idx) override {
    // std::cout << " <><><><> TimeframeSplitter: timeslice " << child_idx
    //           << " of timeframe " << parent.GetEventNumber() << ", targetDetID: " << targetDetId << " <><><><<><>"
    //           << std::endl;

    float m_timeframe_width = timeframe_width();
    float m_timesplit_width = timesplit_width();

    bool m_bTrigger                           = false;
    Int_t hitsCountsInTSDevInThetaPhi1[12][8] = {}; // Theta 0-12, Phi 0-8
    Int_t hitsCountsInTSDevInThetaPhi2[12][8] = {}; // Theta 0-11, Phi 0-8

    // == s == Register hits of TOF and MPGD detectors in the time slice ==================
    if (child_idx == 0) {
      m_vOrigHitId.resize(m_triggerDetSize);
      m_vSameTSHitId.resize(m_triggerDetSize);
      for (std::size_t iSub = 0; iSub < m_triggerDetSize; ++iSub) {
        Int_t subDet    = m_detId[iSub];
        size_t vHitSize = m_simtrackerhits_in().at(subDet)->size();
        std::vector<unsigned int> m_vOrigHitId_sub;
        m_vOrigHitId_sub.reserve(vHitSize);
        for (std::size_t i = 0; i < vHitSize; ++i)
          m_vOrigHitId_sub.push_back(i);
        m_vOrigHitId[iSub] = std::move(m_vOrigHitId_sub);
      }
      bInitialLoop = false;

      // == s == For MC Trigger Efficiency Estimation ~~~~~~~~
      for (const auto& mcparticle : *m_mcparticles_in) {
        auto parentMCP = *mcparticle.parents_begin();
        if (parentMCP.getObjectID().index != 0)
          continue;

        Double_t mcCollTime = mcparticle.getTime();
        m_vPhysCooTimes.push_back(mcCollTime);
        // std::cout << "    >>> MCParticle ID: " << " Time: " << mcCollTime << std::endl;
      }
      // == e == For MC Trigger Efficiency Estimation ~~~~~~~~

      // for(std::size_t iSub = 0; iSub < m_triggerDetSize; ++iSub){
      //    for(std::size_t iHit = 0; iHit <  m_simtrackerhits_in().at(iSub)->size(); ++iHit){
      //       std::cout << "    >>> Detector " << m_detId[iSub] << " Hit " << iHit << " Time: " << m_simtrackerhits_in().at(iSub)->at(iHit).getTime() << std::endl;
      //    }
      // }
    }
    // == e == Register hits of TOF and MPGD detectors in the time slice ==================

    // == s == Time-slice base detector loop ================================================
    for (size_t iBaseDet = targetDetId; iBaseDet < m_triggerDetSize; ++iBaseDet) {
      if (iBaseDet > 5 && m_bOnceTriggered) {
        m_bScanedAllTimeWindows = true;
        break;
      }

      Int_t baseDetID      = m_detId[iBaseDet];
      float baseDetTimeRes = timeResolution_TOF();
      if (iBaseDet > 5)
        baseDetTimeRes = timeResolution_Silicon();
      else if (iBaseDet > 1)
        baseDetTimeRes = timeResolution_MPGD();

      // == s == Time-slice base detector hits loop =======================================
      Int_t baseDetNumOfHits = m_vOrigHitId.at(iBaseDet).size();
      if (startHitPoint[iBaseDet] >= baseDetNumOfHits) {
        m_bDetLastHits[iBaseDet] = true;
        targetDetId++;
        if (targetDetId >= m_triggerDetSize)
          m_bScanedAllTimeWindows = true;
        continue;
      }

      for (size_t iBaseHit = startHitPoint[iBaseDet]; iBaseHit < baseDetNumOfHits; ++iBaseHit) {
        unsigned int baseHitID = m_vOrigHitId.at(iBaseDet).at(iBaseHit);
        const auto& baseHit    = m_simtrackerhits_in().at(baseDetID)->at(baseHitID);
        auto baseHitTime       = baseHit.getTime();
        m_vSameTSHitId.at(iBaseDet).push_back(baseHitID);
        Int_t baseThetaID1 = 999;
        Int_t baseThetaID2 = 999;
        Int_t basePhiID1   = 999;
        Int_t basePhiID2   = 999;
        thetaPhiBinCalc(baseHit, baseThetaID1, basePhiID1, baseThetaID2, basePhiID2);
        hitsCountsInTSDevInThetaPhi1[baseThetaID1][basePhiID1]++;
        hitsCountsInTSDevInThetaPhi2[baseThetaID2][basePhiID2]++;

        // Own detectors loop
        Int_t iCompDet  = iBaseDet;
        Int_t compDetID = m_detId[iCompDet];
        for (size_t iCompHit = iBaseHit + 1; iCompHit < baseDetNumOfHits; ++iCompHit) {
          unsigned int compHitID = m_vOrigHitId.at(iCompDet).at(iCompHit);
          const auto& compHit    = m_simtrackerhits_in().at(compDetID)->at(compHitID);
          Double_t compHitTime   = compHit.getTime();
          float compDetTimeRes   = timeResolution_TOF();
          if (iCompDet > 5)
            compDetTimeRes = timeResolution_Silicon();
          else if (iCompDet > 1)
            compDetTimeRes = timeResolution_MPGD();

          unsigned int bInTS = 1;
          // == s == Check if the hit is in the current time slice ==========================
          if (compHitTime - compDetTimeRes < baseHitTime + baseDetTimeRes) {
            if (compHitTime + compDetTimeRes > baseHitTime - baseDetTimeRes) {
              Int_t thetaID1 = 999;
              Int_t thetaID2 = 999;
              Int_t phiID1   = 999;
              Int_t phiID2   = 999;
              thetaPhiBinCalc(compHit, thetaID1, phiID1, thetaID2, phiID2);
              hitsCountsInTSDevInThetaPhi1[thetaID1][phiID1]++;
              hitsCountsInTSDevInThetaPhi2[thetaID2][phiID2]++;
              bInTS = 0;
            } else
              bInTS = 2;
          }

          if (bInTS == 0)
            m_vSameTSHitId.at(iCompDet).push_back(compHitID);
          else if (bInTS == 1) {
            // Update the start point for the next iteration
            startHitPoint[iBaseDet] = iCompHit;
            break; // Break if the hit time exceeds the current time slice
          }
          // == e == Check if the hit is in the current time slice ==========================
        }

        // Other detectors loop
        for (size_t iCompDet = iBaseDet + 1; iCompDet < m_triggerDetSize; ++iCompDet) {
          Int_t compDetID        = m_detId[iCompDet];
          Int_t compDetNumOfHits = m_vOrigHitId.at(iCompDet).size();

          for (size_t iCompHit = 0; iCompHit < compDetNumOfHits; ++iCompHit) {
            unsigned int compHitID = m_vOrigHitId.at(iCompDet).at(iCompHit);
            const auto& compHit    = m_simtrackerhits_in().at(compDetID)->at(compHitID);
            Double_t compHitTime   = compHit.getTime();
            float compDetTimeRes   = timeResolution_TOF();
            if (iCompDet > 5)
              compDetTimeRes = timeResolution_Silicon();
            else if (iCompDet > 1)
              compDetTimeRes = timeResolution_MPGD();

            // == s == Check if the hit is in the current time slice ==========================
            unsigned int bInTS = 1;
            if (compHitTime - compDetTimeRes < baseHitTime + baseDetTimeRes) {
              if (compHitTime + compDetTimeRes > baseHitTime - baseDetTimeRes) {
                Int_t thetaID1 = 999;
                Int_t thetaID2 = 999;
                Int_t phiID1   = 999;
                Int_t phiID2   = 999;
                thetaPhiBinCalc(compHit, thetaID1, phiID1, thetaID2, phiID2);
                hitsCountsInTSDevInThetaPhi1[thetaID1][phiID1]++;
                bInTS = 0;
              } else
                bInTS = 2;
            }

            if (bInTS == 0)
              m_vSameTSHitId.at(iCompDet).push_back(compHitID);
            else if (bInTS == 1)
              break;

            // == e == Check if the hit is in the current time slice ==========================
          }
        }

        if (iBaseHit == baseDetNumOfHits - 1 || startHitPoint[iBaseDet] == baseDetNumOfHits - 1) {
          m_bDetLastHits[iBaseDet] = true;
          targetDetId++;
          if (targetDetId >= m_triggerDetSize)
            m_bScanedAllTimeWindows = true;
        }

        // == s ==  Trigger Judgement ==================================================
        for (size_t iThetaBin = 0; iThetaBin < 12; ++iThetaBin) {
          for (size_t iPhiBin = 0; iPhiBin < 8; ++iPhiBin) {
            if (hitsCountsInTSDevInThetaPhi1[iThetaBin][iPhiBin] > 2)
              m_bTrigger = true;
            if (hitsCountsInTSDevInThetaPhi2[iThetaBin][iPhiBin] > 2)
              m_bTrigger = true;
          }
        }
        // == e ==  Trigger Judgement ====================================================

        if (m_bTrigger) {
          // == s ==  Register all tracker hits in the same time slice into output container
          for (size_t trkDetID = 0; trkDetID < m_simtrackerhits_in().size(); ++trkDetID) {
            auto& trkOutColl    = m_simtrackerhits_out().at(trkDetID);
            Int_t iTrigTrkDetID = 0;
            if (trkDetID < m_triggerDetSize) {
              trkOutColl->setSubsetCollection(true);

              for (size_t iHit = 0; iHit < m_vSameTSHitId.at(trkDetID).size(); ++iHit) {
                Int_t hitID        = m_vSameTSHitId.at(trkDetID).at(iHit);
                const auto& trkHit = m_simtrackerhits_in().at(trkDetID)->at(hitID);
                trkOutColl->push_back(trkHit);
                size_t origHitSize = m_vOrigHitId.at(trkDetID).size();
                for (size_t iOrigHit = 0; iOrigHit < origHitSize; ++iOrigHit) {
                  if (hitID != m_vOrigHitId.at(trkDetID).at(iOrigHit))
                    continue;
                  m_vOrigHitId.at(trkDetID).erase(m_vOrigHitId.at(trkDetID).begin() + iOrigHit);
                  iOrigHit--; // Adjust index after erasure
                  origHitSize--;
                }

                // std::cout << "     >>>>> Registered TRK DetID: " << trkDetID << ", HitID: " << hitID << ", Time: " << trkHit.getTime() << std::endl;
              }
            } else {
              // For other detectors, just copy all hits
              trkOutColl->setSubsetCollection(true);
              for (size_t iHit = 0; iHit < m_simtrackerhits_in().at(trkDetID)->size(); ++iHit) {
                const auto& trkHit = m_simtrackerhits_in().at(trkDetID)->at(iHit);
                trkOutColl->push_back(trkHit);
              }
            }
          }
          // == e ==  Register all tracker hits in the same time slice into output container

          // == s ==  Register all calo hits in the same time slice into output container
          for (size_t calDetID = 0; calDetID < m_simcalorimeterhits_in().size(); ++calDetID) {
            auto& caloOutColl = m_simcalorimeterhits_out().at(calDetID);
            caloOutColl->setSubsetCollection(true);
            if (caloOutColl == nullptr)
              continue;
            for (size_t caloHitID = 0; caloHitID < caloOutColl->size(); ++caloHitID) {
              const auto& caloHit = caloOutColl->at(caloHitID);

              auto hitContributions = caloHit.getContributions();
              for (const auto& contribution : hitContributions) {
                auto& caloOutColl_contribution = m_calohitcontributions_out().at(calDetID);
                caloOutColl_contribution->setSubsetCollection(true);
                caloOutColl_contribution->push_back(contribution);
              }
              caloOutColl->push_back(caloHit);
            }
          }
          // == e ==  Register all calo hits in the same time slice into output container

          // For now, a one-to-one relationship between timeslices and events
          child.SetEventNumber(parent.GetEventNumber());
          child.SetRunNumber(parent.GetRunNumber());

          // == s == For MC Trigger Efficiency Estimation ~~~~~~~~
          Int_t physEventWeight = 2;
          for (const auto& physCollTime : m_vPhysCooTimes) {
            if (physCollTime > baseHitTime - baseDetTimeRes &&
                physCollTime < baseHitTime + baseDetTimeRes) {
              physEventWeight = 1;
              m_vPhysCooTimes.erase(m_vPhysCooTimes.begin() + physCollTime);
              break;
            }
          }
          // == e == For MC Trigger Efficiency Estimation ~~~~~~~~

          edm4hep::MutableEventHeader event_header_ts;
          event_header_ts.setRunNumber(m_event_number_ts * 10000 + child_idx);
          event_header_ts.setEventNumber(m_event_number_ts);
          event_header_ts.setTimeStamp(iTimeSlice);
          event_header_ts.setWeight(physEventWeight);

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

          auto& asso_in_vec  = m_simtrackerhitsAsso_in();
          auto& asso_out_vec = m_simtrackerhitsAsso_out();
          for (size_t iDet = 0; iDet < asso_in_vec.size(); ++iDet) {
            const auto* inAsso_coll = asso_in_vec[iDet];
            auto& outAsso_coll      = asso_out_vec[iDet];
            outAsso_coll->setSubsetCollection();

            if (inAsso_coll == nullptr) {
              if (!outAsso_coll)
                outAsso_coll = std::make_unique<edm4eic::MCRecoTrackerHitAssociationCollection>();
              else
                outAsso_coll->clear();
              continue;
            }

            // if (!outAsso_coll) outAsso_coll = \
            //   std::make_unique<edm4eic::MCRecoTrackerHitAssociationCollection>();
            // else outAsso_coll->clear();

            for (const auto& asso : *inAsso_coll)
              outAsso_coll->push_back(asso);
          }

          // == e == Basic container for simulatin data (but not related to data)  =========

          if (iBaseHit != baseDetNumOfHits - 1)
            startHitPoint[iBaseDet]++;
          iTimeSlice++;
          m_bOnceTriggered = true;
        }

        for (size_t iDet = 0; iDet < m_triggerDetSize; ++iDet) {
          m_vSameTSHitId.at(iDet).clear();
          std::vector<unsigned int>().swap(m_vSameTSHitId.at(iDet));
        }

        if (m_bTrigger)
          break;

      } // == e == Time-slice base detector hits loop ==================================
      if (m_bTrigger)
        break;
    } // == e == Time-slice base detector loop ==================================

    if (m_bDetLastHits[8])
      m_bScanedAllTimeWindows = true;
    if (m_bScanedAllTimeWindows) {
      bInitialLoop     = true;
      m_bOnceTriggered = false;

      for (size_t iDet = 0; iDet < m_triggerDetSize; ++iDet) {
        m_vOrigHitId.at(iDet).clear();
        std::vector<unsigned int>().swap(m_vOrigHitId.at(iDet));
        startHitPoint[iDet]  = 0;
        m_bDetLastHits[iDet] = false;
      }
      m_vOrigHitId.clear();

      m_vPhysCooTimes.clear();
      std::vector<Double_t>().swap(m_vPhysCooTimes);

      m_bScanedAllTimeWindows = false;
      iTimeSlice              = 0;
      targetDetId             = 0;

      if (m_bTrigger)
        return Result::NextChildNextParent;
      else
        return Result::KeepChildNextParent;
    } else if (m_bTrigger) {
      child_idx++;
      return Result::NextChildKeepParent;
    }
    return Result::KeepChildNextParent;
  }

  inline void thetaPhiBinCalc(edm4eic::TrackerHit hit, Int_t& thetaID1, Int_t& phiID1,
                              Int_t& thetaID2, Int_t& phiID2) {
    Double_t hitX     = hit.getPosition()[0];
    Double_t hitY     = hit.getPosition()[1];
    Double_t hitZ     = hit.getPosition()[2];
    Double_t hitR     = TMath::Sqrt(hitX * hitX + hitY * hitY + hitZ * hitZ);
    Double_t hitTheta = 0.;
    hitTheta          = TMath::ACos(hitZ / hitR);
    if (hitTheta > 1.0)
      hitTheta = 1.0;
    if (hitTheta < -1.0)
      hitTheta = -1.0;

    Double_t hitPhi = TMath::ATan2(hitY, hitX) + 2 * TMath::Pi();
    if (hitPhi < 0)
      hitPhi += 2 * TMath::Pi();
    thetaID1 = hitTheta / (TMath::Pi() / 12.);
    thetaID2 = (hitTheta + TMath::Pi() / 24.) / (TMath::Pi() / 12.);
    phiID1   = hitPhi / (TMath::Pi() / 8.);
    phiID2   = (hitPhi + TMath::Pi() / 16.) / (TMath::Pi() / 8.);
  }
};
