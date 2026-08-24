// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#pragma once

#include <JANA/Components/JComponent.h>
#include <JANA/Components/JPodioOutput.h>
#include <JANA/JEventUnfolder.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociation.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/MutableMCParticle.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <podio/ObjectID.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <spdlog/logger.h>
#include <stddef.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

struct TimeframeSplitter : public JEventUnfolder {

  Parameter<float> timeframeWidth{this, "timeframe_width", 2000.0, "Width of each timeframe in ns"};
  Parameter<float> timesplitWidth{this, "timesplit_width", 20.0, "Width of each timeslice in ns"};
  Parameter<float> timeResolution_SiMaps{this, "timeResolution_Silicon", 2000.0,
                                         "time resolution of Silicon detector in ns"};
  Parameter<float> timeResolution_MPGD{this, "timeResolution_MPGD", 30.0,
                                       "time resolution of MPGD detector in ns"};
  Parameter<float> timeResolution_ACLGad{this, "timeResolution_TOF", 20.0,
                                         "time resolution of TOF detector in ns"};
  Parameter<float> timeResolution_EMCal{this, "timeResolution_EMCal", 20.0,
                                        "time resolution of EMCal detector in ns"};

  Parameter<double> refInverseVelocity{
      this, "refInverseVelocity", 0.0034,
      "ns/mm estimated by MC average time of flight"}; //< ns/mm estimated by MC average time of flight / distance from IP to calorimeter

  // MPGD backward Endcap range -3.6 < eta < -1.72, +5%: -3.78 < eta < -1.634
  Parameter<double> backwardEtaMin{this, "backward_eta_min", -3.78,
                                   "Minimum eta for the backward trigger region"};
  Parameter<double> backwardEtaMax{this, "backward_eta_max", -1.634,
                                   "Maximum eta for the backward trigger region"};

  // Barrel trigger region covering MPGD, TOF, and ECal acceptance with margin
  Parameter<double> barrelEtaMin{this, "barrel_eta_min", -1.80,
                                 "Minimum eta for the barrel trigger region"};
  Parameter<double> barrelEtaMax{this, "barrel_eta_max", 1.81,
                                 "Maximum eta for the barrel trigger region"};

  // Forward trigger region covering MPGD, TOF, and ECal acceptance with margin
  Parameter<double> forwardEtaMin{this, "forward_eta_min", 1.77,
                                  "Minimum eta for the forward trigger region"};
  Parameter<double> forwardEtaMax{this, "forward_eta_max", 4.04,
                                  "Maximum eta for the forward trigger region"};

  std::pair<int, int> backEndEtaPhiBins(double hitEta, double hitPhi, int bShift);
  std::pair<int, int> barrelEtaPhiBins(double hitEta, double hitPhi, int bShift);
  std::pair<int, int> forwardEndEtaPhiBins(double hitEta, double hitPhi, int bShift);

  Parameter<size_t> ecalMultiplicityThreshold{
      this, "ecal_multiplicity_threshold", 10,
      "Minimum ECal grid-cell multiplicity for single triggers"};

  Parameter<size_t> backwardTrackerMatchThreshold{
      this, "backward_tracker_match_threshold", 10,
      "Tracker matching threshold for the backward trigger region"};

  Parameter<size_t> barrelTrackerMatchThreshold{
      this, "barrel_tracker_match_threshold", 5,
      "Tracker matching threshold for the barrel trigger region"};

  Parameter<size_t> forwardTrackerMatchThreshold{
      this, "forward_tracker_match_threshold", 5,
      "Tracker matching threshold for the forward trigger region"};

  Parameter<size_t> trackerMultiplicityThreshold{
      this, "tracker_multiplicity_threshold", 1,
      "Minimum matched tracker grid-cell multiplicity for single triggers"};

  Parameter<double> trigTimeWindowBef{this, "trigger_window_before", 10.0 * edm4eic::unit::ns,
                                      "Time window before the trigger time"};
  Parameter<double> trigTimeWindowAft{this, "trigger_window_after", 30.0 * edm4eic::unit::ns,
                                      "Time window after the trigger time"};

  Parameter<double> collisionTimeMarginBef{this, "collision_time_margin_before",
                                           10.0 * edm4eic::unit::ns,
                                           "Time margin before the collision time"};
  Parameter<double> collisionTimeMarginAft{this, "collision_time_margin_after",
                                           20.0 * edm4eic::unit::ns,
                                           "Time margin after the collision time"};

  bool m_use_timeframe = false; // Use timeframes to split events, or use timeslices

  std::shared_ptr<spdlog::logger> m_log;
  unsigned int m_OrigTFCount   = 0; //QA
  unsigned int m_NewEventCount = 0; //QA
  unsigned int m_PhysCount     = 0; //QA

  size_t m_eventNumber_TS = 0; // Event number for the current timeslice

  static constexpr double kPi = 3.14159265358979323846;

  static constexpr int kEtaPhiBins       = 10;
  static constexpr int kInvalidEtaPhiBin = -1;

  using EtaPhiGrid       = std::array<std::array<int, kEtaPhiBins>, kEtaPhiBins>;
  using EtaPhiTimeGrid   = std::array<std::array<double, kEtaPhiBins>, kEtaPhiBins>;
  using EtaPhiEnergyGrid = std::array<std::array<double, kEtaPhiBins>, kEtaPhiBins>;

  enum TrkCollectionType : size_t {
    kTrackerHit = 0,
    kTrackerHitAssociation,
    kTrackerHitLink,
    kSimTrackerHit,
    kRawTrackerHit,
    kTrkCollectionTypeSize
  };

  enum RichCollectionType : size_t {
    kRichRawHitAssociation = 0,
    kRichRawHitLink,
    kRichSimTrackerHit,
    kRichRawTrackerHit,
    kRichCollectionTypeSize
  };

  enum CalCollectionType : size_t {
    kCalorimeterHit = 0,
    kCalorimeterHitAssociation,
    kCalorimeterHitLink,
    kSimCalorimeterHit,
    kRawCalorimeterHit,
    kCalCollectionTypeSize
  };

  enum TrkCollectionIndex : size_t {
    kTrkB0 = 0,
    kTrkTOFBarrel,
    kTrkTOFEndcap,
    kTrkMPGDBarrel,
    kTrkOuterMPGDBarrel,
    kTrkBackwardMPGD,
    kTrkForwardMPGD,
    kTrkSiBarrelVertex,
    kTrkSiBarrel,
    kTrkSiEndcap,
    kTrkTagger,
    kTrkForwardRomanPot,
    kTrkForwardOffMTracker,
    kTrkCollectionSize
  };

  enum RichCollectionIndex : size_t { kRICHEndcapN = 0, kDIRCBar, kDRICH, kRichCollectionSize };

  enum CalCollectionIndex : size_t {
    kCalB0ECal = 0,
    kCalEcalBarrelImg,
    kCalEcalBarrelScFi,
    kCalEcalEndcapN,
    kCalEcalEndcapP,
    kCalEcalZDC,
    kCalEcalLumiSpec,
    kCalHcalBarrel,
    kCalHcalEndcapN,
    kCalHcalEndcapPInsert,
    kCalHcalZDC,
    kCalLFHCAL,
    kCalCollectionSize
  };

  enum SingleTriggerIndex : size_t {
    kSingleTrigBackEndcapECal = 0,
    kSingleTrigBackEndcapECalTrk,
    kSingleTrigCentBarrelECal,
    kSingleTrigCentBarrelECalTrk,
    kSingleTrigForwardEndcapECal,
    kSingleTrigForwardEndcapECalTrk,
    kSingleTrigB0Trk,
    kSingleTrigZDCECal,
    kNumOfSingleTrig
  };

  enum SingleTriggerRegion : size_t {
    kSingleTrigRegionBackward = 0,
    kSingleTrigRegionBarrel,
    kSingleTrigRegionForward,
    kNumSingleTrigRegion
  };

  struct TriggerRegionConfig {
    CalCollectionIndex calDetector;
    std::vector<TrkCollectionIndex> trkDetectors;
    SingleTriggerIndex calTrigger;
    SingleTriggerIndex calTrkTrigger;
  };

  const std::array<TriggerRegionConfig, kNumSingleTrigRegion> m_triggerRegionConfigs = {{
      {
          kCalEcalEndcapN,
          {kTrkBackwardMPGD},
          kSingleTrigBackEndcapECal,
          kSingleTrigBackEndcapECalTrk,
      },
      {
          kCalEcalBarrelScFi,
          {kTrkMPGDBarrel, kTrkOuterMPGDBarrel, kTrkTOFBarrel},
          kSingleTrigCentBarrelECal,
          kSingleTrigCentBarrelECalTrk,
      },
      {
          kCalEcalEndcapP,
          {kTrkForwardMPGD, kTrkTOFEndcap},
          kSingleTrigForwardEndcapECal,
          kSingleTrigForwardEndcapECalTrk,
      },
  }};

  enum CombineTriggerIndex : size_t {
    kCombTrigECalTrkAndB0Trk = 0,
    kCombTrigECalTrkAndZDCEcal,
    kCombTrigECalAndB0Trk,
    kCombTrigECalAndZDCEcal,
    kCombTrigECalTrk,
    kCombTrigECal,
    kNumOfCombineTrig
  };

  using trkCollNames = std::array<std::string, kTrkCollectionTypeSize>;
  std::array<trkCollNames, kTrkCollectionSize> m_trkCollNames = {{
      {
          "B0TrackerRecHits",
          "B0TrackerRawHitAssociations",
          "B0TrackerRawHitLinks",
          "B0TrackerHits",
          "B0TrackerRawHits",
      },
      {
          "TOFBarrelSharedRecHits",
          "TOFBarrelSharedRawHitAssociations",
          "TOFBarrelSharedRawHitLinks",
          "TOFBarrelHits",
          "TOFBarrelSharedRawHits",
      },
      {
          "TOFEndcapSharedRecHits",
          "TOFEndcapSharedRawHitAssociations",
          "TOFEndcapSharedRawHitLinks",
          "TOFEndcapHits",
          "TOFEndcapSharedRawHits",
      },
      {
          "MPGDBarrelRecHits",
          "MPGDBarrelRawHitAssociations",
          "MPGDBarrelRawHitLinks",
          "MPGDBarrelHits",
          "MPGDBarrelRawHits",
      },
      {
          "OuterMPGDBarrelRecHits",
          "OuterMPGDBarrelRawHitAssociations",
          "OuterMPGDBarrelRawHitLinks",
          "OuterMPGDBarrelHits",
          "OuterMPGDBarrelRawHits",
      },
      {
          "BackwardMPGDEndcapRecHits",
          "BackwardMPGDEndcapRawHitAssociations",
          "BackwardMPGDEndcapRawHitLinks",
          "BackwardMPGDEndcapHits",
          "BackwardMPGDEndcapRawHits",
      },
      {
          "ForwardMPGDEndcapRecHits",
          "ForwardMPGDEndcapRawHitAssociations",
          "ForwardMPGDEndcapRawHitLinks",
          "ForwardMPGDEndcapHits",
          "ForwardMPGDEndcapRawHits",
      },
      {
          "SiBarrelVertexRecHits",
          "SiBarrelVertexRawHitAssociations",
          "SiBarrelVertexRawHitLinks",
          "VertexBarrelHits",
          "SiBarrelVertexRawHits",
      },
      {
          "SiBarrelTrackerRecHits",
          "SiBarrelRawHitAssociations",
          "SiBarrelRawHitLinks",
          "SiBarrelHits",
          "SiBarrelRawHits",
      },
      {
          "SiEndcapTrackerRecHits",
          "SiEndcapTrackerRawHitAssociations",
          "SiEndcapTrackerRawHitLinks",
          "TrackerEndcapHits",
          "SiEndcapTrackerRawHits",
      },
      {
          "TaggerTrackerRecHits",
          "TaggerTrackerRawHitAssociations",
          "TaggerTrackerRawHitLinks",
          "TaggerTrackerHits",
          "TaggerTrackerRawHits",
      },
      {
          "ForwardRomanPotRecHits",
          "ForwardRomanPotRawHitAssociations",
          "ForwardRomanPotRawHitLinks",
          "ForwardRomanPotHits",
          "ForwardRomanPotRawHits",
      },
      {
          "ForwardOffMTrackerRecHits",
          "ForwardOffMTrackerRawHitAssociations",
          "ForwardOffMTrackerRawHitLinks",
          "ForwardOffMTrackerHits",
          "ForwardOffMTrackerRawHits",
      },
  }};

  using richCollNames = std::array<std::string, kRichCollectionTypeSize>;
  std::array<richCollNames, kRichCollectionSize> m_richCollNames = {{
      {
          "RICHEndcapNRawHitsAssociations",
          "RICHEndcapNRawHitsLinks",
          "PFRICHHits",
          "RICHEndcapNRawHits",
      },
      {
          "DIRCRawHitsAssociations",
          "DIRCRawHitsLinks",
          "DIRCBarHits",
          "DIRCRawHits",
      },
      {
          "DRICHRawHitsAssociations",
          "DRICHRawHitsLinks",
          "DRICHHits",
          "DRICHRawHits",
      },
  }};

  using calCollNames                          = std::array<std::string, kCalCollectionTypeSize>;
  std::array<calCollNames, 12> m_calCollNames = {{
      {
          "B0ECalRecHits",
          "B0ECalRawHitAssociations",
          "B0ECalRawHitLinks",
          "B0ECalHits",
          "B0ECalRawHits",
      },
      {
          "EcalBarrelImagingRecHits",
          "EcalBarrelImagingRawHitAssociations",
          "EcalBarrelImagingRawHitLinks",
          "EcalBarrelImagingHits",
          "EcalBarrelImagingRawHits",
      },
      {
          "EcalBarrelScFiRecHits",
          "EcalBarrelScFiRawHitAssociations",
          "EcalBarrelScFiRawHitLinks",
          "EcalBarrelScFiHits",
          "EcalBarrelScFiRawHits",
      },
      {
          "EcalEndcapNRecHits",
          "EcalEndcapNRawHitAssociations",
          "EcalEndcapNRawHitLinks",
          "EcalEndcapNHits",
          "EcalEndcapNRawHits",
      },
      {
          "EcalEndcapPRecHits",
          "EcalEndcapPRawHitAssociations",
          "EcalEndcapPRawHitLinks",
          "EcalEndcapPHits",
          "EcalEndcapPRawHits",
      },
      {
          "EcalFarForwardZDCRecHits",
          "EcalFarForwardZDCRawHitAssociations",
          "EcalFarForwardZDCRawHitLinks",
          "EcalFarForwardZDCHits",
          "EcalFarForwardZDCRawHits",
      },
      {
          "EcalLumiSpecRecHits",
          "EcalLumiSpecRawHitAssociations",
          "EcalLumiSpecRawHitLinks",
          "EcalLumiSpecHits",
          "EcalLumiSpecRawHits",
      },
      {
          "HcalBarrelRecHits",
          "HcalBarrelRawHitAssociations",
          "HcalBarrelRawHitLinks",
          "HcalBarrelHits",
          "HcalBarrelRawHits",
      },
      {
          "HcalEndcapNRecHits",
          "HcalEndcapNRawHitAssociations",
          "HcalEndcapNRawHitLinks",
          "HcalEndcapNHits",
          "HcalEndcapNRawHits",
      },
      {
          "HcalEndcapPInsertRecHits",
          "HcalEndcapPInsertRawHitAssociations",
          "HcalEndcapPInsertRawHitLinks",
          "HcalEndcapPInsertHits",
          "HcalEndcapPInsertRawHits",
      },
      {
          "HcalFarForwardZDCRecHits",
          "HcalFarForwardZDCRawHitAssociations",
          "HcalFarForwardZDCRawHitLinks",
          "HcalFarForwardZDCHits",
          "HcalFarForwardZDCRawHits",
      },
      {
          "LFHCALRecHits",
          "LFHCALRawHitAssociations",
          "LFHCALRawHitLinks",
          "LFHCALHits",
          "LFHCALRawHits",
      },
  }};

  std::vector<std::string> getTrkCollectionNames(TrkCollectionType type) const {
    std::vector<std::string> names;
    names.reserve(m_trkCollNames.size());
    for (const auto& collections : m_trkCollNames) {
      names.push_back(collections[type]);
    }
    return names;
  }

  std::vector<std::string> getRichCollectionNames(RichCollectionType type) const {
    std::vector<std::string> names;
    names.reserve(m_richCollNames.size());
    for (const auto& collections : m_richCollNames) {
      names.push_back(collections[type]);
    }
    return names;
  }

  std::vector<std::string> getCalCollectionNames(CalCollectionType type) const {
    std::vector<std::string> names;
    names.reserve(m_calCollNames.size());
    for (const auto& collections : m_calCollNames) {
      names.push_back(collections[type]);
    }
    return names;
  }

  std::vector<std::string> getCalContributionCollectionNames() const {
    std::vector<std::string> names;
    names.reserve(m_calCollNames.size());
    for (const auto& collections : m_calCollNames) {
      names.push_back(collections[kSimCalorimeterHit] + "Contributions");
    }
    return names;
  }

  PodioInput<edm4hep::EventHeader> m_eventHeader_inCol{
      this, {.name = "EventHeader", .is_optional = true}};
  PodioOutput<edm4hep::EventHeader> m_eventHeader_outCol{this, "EventHeader"};

  PodioInput<edm4hep::MCParticle> m_mcParticles_inCol{this, {.name = "MCParticles"}};
  PodioOutput<edm4hep::MCParticle> m_mcParticles_outCol{this, "MCParticles"};

  // tracker collections
  VariadicPodioInput<edm4eic::TrackerHit> m_trackerHits_inCols{
      this, {.names = getTrkCollectionNames(kTrackerHit), .is_optional = true}};
  VariadicPodioOutput<edm4eic::TrackerHit> m_trackerHits_outCols{
      this, getTrkCollectionNames(kTrackerHit)};

  VariadicPodioInput<edm4eic::MCRecoTrackerHitAssociation> m_trackerHitsAsso_inCols{
      this, {.names = getTrkCollectionNames(kTrackerHitAssociation), .is_optional = true}};
  VariadicPodioOutput<edm4eic::MCRecoTrackerHitAssociation> m_trackerHitsAsso_outCols{
      this, getTrkCollectionNames(kTrackerHitAssociation)};

  // VariadicPodioOutput<edm4eic::MCRecoTrackerHitLink> m_recoTrackerHitLinks_inCols{
  //     this, getTrkCollectionNames(kTrackerHitLink)};
  VariadicPodioOutput<edm4eic::MCRecoTrackerHitLink> m_recoTrackerHitLinks_outCols{
      this, getTrkCollectionNames(kTrackerHitLink)};

  VariadicPodioOutput<edm4hep::SimTrackerHit> m_simTrackerHits_outCols{
      this, getTrkCollectionNames(kSimTrackerHit)};

  VariadicPodioInput<edm4eic::RawTrackerHit> m_rawTrackerHit_inCols{
      this, {.names = getTrkCollectionNames(kRawTrackerHit), .is_optional = true}};
  VariadicPodioOutput<edm4eic::RawTrackerHit> m_rawTrackerHit_outCols{
      this, getTrkCollectionNames(kRawTrackerHit)};

  // RICH collections
  VariadicPodioInput<edm4eic::MCRecoTrackerHitAssociation> m_richHitsAsso_inCols{
      this, {.names = getRichCollectionNames(kRichRawHitAssociation), .is_optional = true}};
  VariadicPodioOutput<edm4eic::MCRecoTrackerHitAssociation> m_richHitsAsso_outCols{
      this, getRichCollectionNames(kRichRawHitAssociation)};

  VariadicPodioOutput<edm4eic::MCRecoTrackerHitLink> m_richHitLinks_outCols{
      this, getRichCollectionNames(kRichRawHitLink)};

  VariadicPodioOutput<edm4hep::SimTrackerHit> m_richSimHits_outCols{
      this, getRichCollectionNames(kRichSimTrackerHit)};

  VariadicPodioInput<edm4eic::RawTrackerHit> m_richRawHits_inCols{
      this, {.names = getRichCollectionNames(kRichRawTrackerHit), .is_optional = true}};
  VariadicPodioOutput<edm4eic::RawTrackerHit> m_richRawHits_outCols{
      this, getRichCollectionNames(kRichRawTrackerHit)};

  // calorimeter collections
  VariadicPodioInput<edm4hep::RawCalorimeterHit> m_rawCalorimeterHit_inCols{
      this, {.names = getCalCollectionNames(kRawCalorimeterHit), .is_optional = true}};
  VariadicPodioOutput<edm4hep::RawCalorimeterHit> m_rawCalorimeterHit_outCols{
      this, getCalCollectionNames(kRawCalorimeterHit)};

  VariadicPodioOutput<edm4eic::MCRecoCalorimeterHitLink> m_mcRecoCalorimeterHitLink_outCols{
      this, getCalCollectionNames(kCalorimeterHitLink)};
  VariadicPodioOutput<edm4hep::SimCalorimeterHit> m_simCalorimeterHit_outCols{
      this, getCalCollectionNames(kSimCalorimeterHit)};
  VariadicPodioOutput<edm4hep::CaloHitContribution> m_caloHitContribution_outCols{
      this, getCalContributionCollectionNames()};

  VariadicPodioInput<edm4eic::CalorimeterHit> m_calorimeterHit_inCols{
      this, {.names = getCalCollectionNames(kCalorimeterHit), .is_optional = true}};
  VariadicPodioOutput<edm4eic::CalorimeterHit> m_calorimeterHit_outCols{
      this, getCalCollectionNames(kCalorimeterHit)};

  VariadicPodioInput<edm4eic::MCRecoCalorimeterHitAssociation>
      m_mcRecoCalorimeterHitAssociation_inCols{
          this, {.names = getCalCollectionNames(kCalorimeterHitAssociation), .is_optional = true}};
  VariadicPodioOutput<edm4eic::MCRecoCalorimeterHitAssociation>
      m_mcRecoCalorimeterHitAssociation_outCols{this,
                                                getCalCollectionNames(kCalorimeterHitAssociation)};

  PodioOutput<edm4hep::EventHeader> m_eventHeaderPhy_outCols{this, "EventHeader_PHY"};
  PodioOutput<edm4hep::EventHeader> m_eventHeaderBkg_outCols{this, "EventHeader_BKG"};

  TimeframeSplitter();

  std::vector<std::tuple<size_t, const edm4eic::TrackerHitCollection*, size_t>>
      m_hitStartIndices_simTracker;
  std::vector<std::tuple<size_t, const edm4hep::SimCalorimeterHitCollection*, size_t>>
      m_hitStartIndices_simCalorimeter;

  // == Global Variables =======================
  bool bInitialLoop = true;

  int m_multiTriggerThreshold[4] = {1, 4, 20, 20};
  size_t iniTrkHitPoint[15]      = {0}; // B0Trk,
  size_t iniCalHitPoint[15]      = {0}; // B0Trk,
  bool m_bDetLastHits[10] = {false, false, false, false, false, false, false, false, false, false};

  bool m_bOnceTriggered        = false;
  bool m_bScanedAllTimeWindows = false;

  unsigned int targetDetId                  = 0;
  size_t iTimeSlice                         = 0;
  std::vector<double> m_vPhysCollisionTimes = {};
  // == Global Variables =======================

  struct TimeWindowSummary {
    size_t count       = 0;
    double timeSum     = 0.0;
    size_t nextStartID = 0;

    double average_time() const { return count == 0 ? 0.0 : timeSum / count; }
  };

  using TrackerAssociationIndex     = std::unordered_map<std::uint64_t, std::vector<size_t>>;
  using CalorimeterAssociationIndex = std::unordered_map<std::uint64_t, std::vector<size_t>>;

  std::vector<TrackerAssociationIndex> m_trkAssoIds;
  std::vector<TrackerAssociationIndex> m_richAssoIds;
  std::vector<CalorimeterAssociationIndex> m_calAssoIds;

  static std::uint64_t objIdKey(const podio::ObjectID& object_id);

  static TrackerAssociationIndex
  buildTrkAssoId(const edm4eic::MCRecoTrackerHitAssociationCollection* associations);

  static CalorimeterAssociationIndex
  buildCalAssoId(const edm4eic::MCRecoCalorimeterHitAssociationCollection* associations);

  static bool overlapsTimeWindow(double hitTime, double resolution, double window_start,
                                 double window_end);
  static bool judgeOverTimeWindow(double hitTime, double resolution, double window_end);

  static bool isValidEtaPhiBin(int etaBin, int phiBin);

  static bool judgeHitInTimeSlice(double hitTime, double timeResolution, double timeslice_start,
                                  double timeslice_end);

  template <typename HitT> double timeOfFlightCorrectedTime(const HitT& hit) {
    const auto& position = hit.getPosition();
    const double radius  = std::sqrt(position[0] * position[0] + position[1] * position[1] +
                                     position[2] * position[2]);

    return hit.getTime() - radius * refInverseVelocity();
  }

  template <typename HitT> inline void etaPhiCalc(const HitT& hit, double& hitEta, double& hitPhi) {
    const double hitX = hit.getPosition()[0];
    const double hitY = hit.getPosition()[1];
    const double hitZ = hit.getPosition()[2];
    const double hitR = std::sqrt(hitX * hitX + hitY * hitY + hitZ * hitZ);
    if (hitR <= 0.0) {
      hitEta = 0.0;
      hitPhi = 0.0;
      return;
    }

    const double cosTheta = std::clamp(hitZ / hitR, -1.0, 1.0);
    const double hitTheta = std::acos(cosTheta);

    hitEta = -std::log(std::tan(hitTheta / 2.0));
    hitPhi = std::atan2(hitY, hitX);
  }

  static std::pair<int, int> etaPhiBins(double hitEta, double hitPhi, double etaMin, double etaMax,
                                        int bShift);

  template <typename CollectionT, typename BinFunc>
  void fillEtaPhiGrids(const CollectionT* hits, size_t& iniHitID, double timeResolution,
                       double timeSliceStart, double timeSliceEnd, EtaPhiGrid& grid,
                       EtaPhiGrid& gridShifted, EtaPhiTimeGrid& gridTime,
                       EtaPhiTimeGrid& gridShiftedTime, BinFunc binFunc) {
    if (hits == nullptr)
      return;

    const size_t hitCount = hits->size();
    for (size_t iHit = iniHitID; iHit < hitCount; ++iHit) {
      const auto& hit   = hits->at(iHit);
      const double hitT = timeOfFlightCorrectedTime(hit);
      if (hitT - timeResolution > timeSliceEnd) {
        iniHitID = iHit;
        break;
      }
      if (!judgeHitInTimeSlice(hitT, timeResolution, timeSliceStart, timeSliceEnd))
        continue;

      double hitEta = 0.0;
      double hitPhi = 0.0;
      etaPhiCalc(hit, hitEta, hitPhi);

      const auto [eta0, phi0] = binFunc(hitEta, hitPhi, 0);
      const auto [eta1, phi1] = binFunc(hitEta, hitPhi, 1);
      if (isValidEtaPhiBin(eta0, phi0)) {
        grid[eta0][phi0]++;
        gridTime[eta0][phi0] += hitT;
      }
      if (isValidEtaPhiBin(eta1, phi1)) {
        gridShifted[eta1][phi1]++;
        gridShiftedTime[eta1][phi1] += hitT;
      }
    }
  }

  template <typename CollectionT, typename BinFunc>
  void fillEtaPhiGridsMatched(const CollectionT* collection, size_t& iniHitID,
                              double timeResolution, double timeSliceStart, double timeSliceEnd,
                              const EtaPhiGrid& baseGrid, const EtaPhiGrid& baseGridShifted,
                              EtaPhiGrid& compGrid, EtaPhiGrid& compGridShifted, int baseThreshold,
                              EtaPhiTimeGrid& compGridTime, EtaPhiTimeGrid& compGridShiftedTime,
                              BinFunc binFunc) {
    if (collection == nullptr)
      return;

    const size_t hitCount = collection->size();
    for (size_t iHit = iniHitID; iHit < hitCount; ++iHit) {
      const auto& hit   = collection->at(iHit);
      const double hitT = timeOfFlightCorrectedTime(hit);
      if (hitT - timeResolution > timeSliceEnd) {
        iniHitID = iHit;
        break;
      }
      if (!judgeHitInTimeSlice(hitT, timeResolution, timeSliceStart, timeSliceEnd))
        continue;

      double hitEta = 0.0;
      double hitPhi = 0.0;
      etaPhiCalc(collection->at(iHit), hitEta, hitPhi);

      const auto [eta0, phi0] = binFunc(hitEta, hitPhi, 0);
      const auto [eta1, phi1] = binFunc(hitEta, hitPhi, 1);
      if (isValidEtaPhiBin(eta0, phi0) && baseGrid[eta0][phi0] >= baseThreshold) {
        compGrid[eta0][phi0]++;
        compGridTime[eta0][phi0] += hitT;
      }
      if (isValidEtaPhiBin(eta1, phi1) && baseGridShifted[eta1][phi1] >= baseThreshold) {
        compGridShifted[eta1][phi1]++;
        compGridShiftedTime[eta1][phi1] += hitT;
      }
    }
  }

  static size_t countGridCellsWithMultiplicity(const EtaPhiGrid& grid0,
                                               const EtaPhiGrid& gridShifted,
                                               const EtaPhiTimeGrid& gridTime0,
                                               const EtaPhiTimeGrid& gridShiftedTime, int threshold,
                                               double& averageTime);

  static double averageSelectedTriggerTime(const std::array<double, 8>& values,
                                           const std::array<double, 8>& times,
                                           std::initializer_list<size_t> indices,
                                           double fallbackTime);

  double trkTimeResolution(TrkCollectionIndex detectorID);
  double calTimeResolution(CalCollectionIndex detectorID);

  template <typename CollectionT>
  TimeWindowSummary countHitsInTimeWindow(const CollectionT* collection, size_t startHitID,
                                          double resolution, double window_start,
                                          double window_end) {
    TimeWindowSummary summary;
    summary.nextStartID = startHitID;
    if (collection == nullptr)
      return summary;

    for (size_t i = startHitID; i < collection->size(); ++i) {
      const auto& hit      = collection->at(i);
      const double hitTime = timeOfFlightCorrectedTime(hit);
      if (judgeOverTimeWindow(hitTime, resolution, window_end))
        break;

      // Drop hits which cannot overlap the next window (which begins at window_end)
      if (hitTime + resolution < window_end) {
        summary.nextStartID = i + 1;
      }

      if (overlapsTimeWindow(hitTime, resolution, window_start, window_end)) {
        ++summary.count;
        summary.timeSum += hitTime;
      }
    }
    return summary;
  }

  template <typename TrackerHitOutputT, typename RawHitOutputT, typename AssociationOutputT>
  static void
  copyTrkHitWithRelations(const edm4eic::TrackerHit& trackerHit,
                          const edm4eic::MCRecoTrackerHitAssociationCollection* associations,
                          const TrackerAssociationIndex& association_index,
                          TrackerHitOutputT& trackerHits_out, RawHitOutputT& rawHits_out,
                          AssociationOutputT& associations_out,
                          std::unique_ptr<edm4hep::SimTrackerHitCollection>& simHits_out,
                          std::unique_ptr<edm4eic::MCRecoTrackerHitLinkCollection>& links_out,
                          std::unique_ptr<edm4hep::MCParticleCollection>& mc_particles_out) {

    auto trackerHitCopied = trackerHit.clone();
    trackerHitCopied.setRawHit(edm4eic::RawTrackerHit());

    if (associations == nullptr || !trackerHit.getRawHit().isAvailable()) {
      trackerHits_out->push_back(trackerHitCopied);
      return;
    }

    const auto rawHitId = trackerHit.getRawHit().getObjectID();

    auto rawHitCopied = trackerHit.getRawHit().clone();
    rawHits_out->push_back(rawHitCopied);
    trackerHitCopied.setRawHit(rawHitCopied);

    const auto assocIterCal = association_index.find(objIdKey(rawHitId));

    if (assocIterCal != association_index.end()) {
      for (const size_t index : assocIterCal->second) {
        const auto association = associations->at(index);

        if (!association.getSimHit().isAvailable())
          continue;

        const auto simHit = association.getSimHit();
        auto simHitCopied = simHit.clone(false);

        if (simHit.getParticle().isAvailable()) {
          const auto mcPIndex = simHit.getParticle().getObjectID();

          if (mcPIndex.index >= 0 &&
              static_cast<size_t>(mcPIndex.index) < mc_particles_out->size()) {
            simHitCopied.setParticle((*mc_particles_out)[mcPIndex.index]);
          }
        }

        simHits_out->push_back(simHitCopied);

        auto copiedAsso = associations_out->create();
        copiedAsso.setWeight(association.getWeight());
        copiedAsso.setRawHit(rawHitCopied);
        copiedAsso.setSimHit(simHitCopied);

        auto copiedLink = links_out->create();
        copiedLink.setWeight(association.getWeight());
        copiedLink.setFrom(rawHitCopied);
        copiedLink.setTo(simHitCopied);
      }
    }

    trackerHits_out->push_back(trackerHitCopied);
  }

  Result Unfold(const JEvent& parent, JEvent& child, int child_idx) override;
};
