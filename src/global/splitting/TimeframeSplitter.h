// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <algorithm>
#include <array>
#include <initializer_list>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <cstdint>
#include <unordered_map>

#include "TMath.h"

#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>

#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociation.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>

#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <JANA/JEventUnfolder.h>

struct TimeframeSplitter : public JEventUnfolder {

  Parameter<float> timeframe_width{this, "timeframe_width", 2000.0,
                                   "Width of each timeframe in ns"};
  Parameter<float> timesplit_width{this, "timesplit_width", 20.0, "Width of each timeslice in ns"};
  Parameter<float> timeResolution_SiMaps{this, "timeResolution_Silicon", 2000.0,
                                         "time resolution of Silicon detector in ns"};
  Parameter<float> timeResolution_MPGD{this, "timeResolution_MPGD", 30.0,
                                       "time resolution of MPGD detector in ns"};
  Parameter<float> timeResolution_ACLGad{this, "timeResolution_TOF", 20.0,
                                         "time resolution of TOF detector in ns"};
  Parameter<float> timeResolution_EMCal{this, "timeResolution_EMCal", 20.0,
                                        "time resolution of EMCal detector in ns"};
  bool m_use_timeframe = false; // Use timeframes to split events, or use timeslices

  Int_t m_OrigTFCount   = 0; //QA
  Int_t m_NewEventCount = 0; //QA
  Int_t m_PhysCount     = 0; //QA

  size_t m_event_number_ts   = 0;    // Event number for the current timeslice
  size_t m_event_number_orig = 0;    // Event number for the current timeslice
  std::vector<Int_t> m_vTargetEvent; // List of original event numbers for each timeslice

  static constexpr Int_t kEtaPhiBins       = 10;
  static constexpr Int_t kInvalidEtaPhiBin = -1;

  using EtaPhiGrid       = std::array<std::array<Int_t, kEtaPhiBins>, kEtaPhiBins>;
  using EtaPhiTimeGrid   = std::array<std::array<Double_t, kEtaPhiBins>, kEtaPhiBins>;
  using EtaPhiEnergyGrid = std::array<std::array<Double_t, kEtaPhiBins>, kEtaPhiBins>;

  enum TrkCollectionIndex : size_t {
    kTrkB0                 = 0,
    kTrkTOFBarrel          = 1,
    kTrkTOFEndcap          = 2,
    kTrkMPGDBarrel         = 3,
    kTrkOuterMPGDBarrel    = 4,
    kTrkBackwardMPGD       = 5,
    kTrkForwardMPGD        = 6,
    kTrkSiBarrelVertex     = 7,
    kTrkSiBarrel           = 8,
    kTrkSiEndcap           = 9,
    kTrkTagger             = 10,
    kTrkForwardRomanPot    = 11,
    kTrkForwardOffMTracker = 12
  };

  enum CalRecCollectionIndex : size_t {
    kCalB0ECal      = 0,
    kCalBarrelImg   = 1,
    kCalBarrelScifi = 2,
    kCalEndcapN     = 3,
    kCalEndcapP     = 4,
    kCalZDC         = 5,
    kCalLumi        = 6
  };

  std::vector<std::string> m_trackerhit_collection_names = {
      "B0TrackerRecHits_aligned",         "TOFBarrelRecHits_aligned",
      "TOFEndcapRecHits_aligned",         "MPGDBarrelRecHits_aligned",
      "OuterMPGDBarrelRecHits_aligned",   "BackwardMPGDEndcapRecHits_aligned",
      "ForwardMPGDEndcapRecHits_aligned", "SiBarrelVertexRecHits_aligned",
      "SiBarrelTrackerRecHits_aligned",   "SiEndcapTrackerRecHits_aligned",
      "TaggerTrackerRecHits_aligned",     "ForwardRomanPotRecHits_aligned",
      "ForwardOffMTrackerRecHits_aligned"};

  // "RICHEndcapNRecHits_aligned"
  // "DIRCBarRecHits_aligned",
  // "DRICHRecHits_aligned",

  std::vector<std::string> m_trackerhit_collection_names_out = {
      "B0TrackerRecHits",         "TOFBarrelRecHits",       "TOFEndcapRecHits",
      "MPGDBarrelRecHits",        "OuterMPGDBarrelRecHits", "BackwardMPGDEndcapRecHits",
      "ForwardMPGDEndcapRecHits", "SiBarrelVertexRecHits",  "SiBarrelTrackerRecHits",
      "SiEndcapTrackerRecHits",   "TaggerTrackerRecHits",   "ForwardRomanPotRecHits",
      "ForwardOffMTrackerRecHits"};

  // "RICHEndcapNRecHits"
  // "DIRCBarRecHits",
  // "DRICHRecHits",

  std::vector<std::string> m_simtrackerhitAsso_collection_names = {
      "B0TrackerRawHitAssociations",         "TOFBarrelRawHitAssociations",
      "TOFEndcapRawHitAssociations",         "MPGDBarrelRawHitAssociations",
      "OuterMPGDBarrelRawHitAssociations",   "BackwardMPGDEndcapRawHitAssociations",
      "ForwardMPGDEndcapRawHitAssociations", "SiBarrelVertexRawHitAssociations",
      "SiBarrelRawHitAssociations",          "SiEndcapTrackerRawHitAssociations",
      "TaggerTrackerRawHitAssociations",     "ForwardRomanPotRawHitAssociations",
      "ForwardOffMTrackerRawHitAssociations"};

  //   "RICHEndcapNRawHitAssociations"
  //   "DIRCBarRawHitsAssociations",
  //   "DRICHRawHitAssociations",

  std::vector<std::string> m_simtrackerhitAsso_collection_names_out = {
      "B0TrackerRawHitAssociations",         "TOFBarrelRawHitAssociations",
      "TOFEndcapRawHitAssociations",         "MPGDBarrelRawHitAssociations",
      "OuterMPGDBarrelRawHitAssociations",   "BackwardMPGDEndcapRawHitAssociations",
      "ForwardMPGDEndcapRawHitAssociations", "SiBarrelVertexRawHitAssociations",
      "SiBarrelRawHitAssociations",          "SiEndcapTrackerRawHitAssociations",
      "TaggerTrackerRawHitAssociations",     "ForwardRomanPotRawHitAssociations",
      "ForwardOffMTrackerRawHitAssociations"};
  // "RICHEndcapNRawHitsAssociations"
  // "DIRCBarRawHitAssociations",
  // "DRICHRawHitsAssociations",

  std::vector<std::string> m_rawhitlink_collection_names = {
      "B0TrackerRawHitLinks",         "TOFBarrelRawHitLinks",       "TOFEndcapRawHitLinks",
      "MPGDBarrelRawHitLinks",        "OuterMPGDBarrelRawHitLinks", "BackwardMPGDEndcapRawHitLinks",
      "ForwardMPGDEndcapRawHitLinks", "SiBarrelVertexRawHitLinks",  "SiBarrelRawHitLinks",
      "SiEndcapTrackerRawHitLinks",   "TaggerTrackerRawHitLinks",   "ForwardRomanPotRawHitLinks",
      "ForwardOffMTrackerRawHitLinks"};
  //   "RICHEndcapNRawHitsLinks"
  //   "DIRCBarRawHitLinks",
  //   "DRICHRawHitLinks",

  std::vector<std::string> m_rawhitlink_collection_names_out = {
      "B0TrackerRawHitLinks",         "TOFBarrelRawHitLinks",       "TOFEndcapRawHitLinks",
      "MPGDBarrelRawHitLinks",        "OuterMPGDBarrelRawHitLinks", "BackwardMPGDEndcapRawHitLinks",
      "ForwardMPGDEndcapRawHitLinks", "SiBarrelVertexRawHitLinks",  "SiBarrelRawHitLinks",
      "SiEndcapTrackerRawHitLinks",   "TaggerTrackerRawHitLinks",   "ForwardRomanPotRawHitLinks",
      "ForwardOffMTrackerRawHitLinks"};
  //   "RICHEndcapNRawHitsLinks"
  //   "DIRCBarRawHitLinks",
  //   "DRICHRawHitsLinks",

  std::vector<std::string> m_simtrackerhit_collection_names_out = {
      "B0TrackerHits",         "TOFBarrelHits",       "TOFEndcapHits",
      "MPGDBarrelHits",        "OuterMPGDBarrelHits", "BackwardMPGDEndcapHits",
      "ForwardMPGDEndcapHits", "VertexBarrelHits",    "SiBarrelHits",
      "TrackerEndcapHits",     "TaggerTrackerHits",   "ForwardRomanPotHits",
      "ForwardOffMTrackerHits"};

  std::vector<std::string> m_rawhit_collection_names = {
      "B0TrackerRawHits",         "TOFBarrelRawHits",       "TOFEndcapRawHits",
      "MPGDBarrelRawHits",        "OuterMPGDBarrelRawHits", "BackwardMPGDEndcapRawHits",
      "ForwardMPGDEndcapRawHits", "SiBarrelVertexRawHits",  "SiBarrelRawHits",
      "SiEndcapTrackerRawHits",   "TaggerTrackerRawHits",   "ForwardRomanPotRawHits",
      "ForwardOffMTrackerRawHits"};
  // "RICHEndcapNRawHits"
  // "DIRCBarRawHits",
  // "DRICHRawHits",

  std::vector<std::string> m_rawhit_collection_names_out = {
      "B0TrackerRawHits",         "TOFBarrelRawHits",       "TOFEndcapRawHits",
      "MPGDBarrelRawHits",        "OuterMPGDBarrelRawHits", "BackwardMPGDEndcapRawHits",
      "ForwardMPGDEndcapRawHits", "SiBarrelVertexRawHits",  "SiBarrelRawHits",
      "SiEndcapTrackerRawHits",   "TaggerTrackerRawHits",   "ForwardRomanPotRawHits",
      "ForwardOffMTrackerRawHits"};
  // "ForwardOffMTrackerRawHits",
  // "RICHEndcapNRawHits"
  // "DIRCBarRawHits",
  // "DRICHRawHits",

  std::vector<std::string> m_calorawhit_collection_names_in = {"B0ECalRawHits",
                                                               "EcalBarrelImagingRawHits",
                                                               "EcalBarrelScFiRawHits",
                                                               "EcalEndcapNRawHits",
                                                               "EcalEndcapPRawHits",
                                                               "EcalFarForwardZDCRawHits",
                                                               "EcalLumiSpecRawHits",
                                                               "HcalBarrelRawHits",
                                                               "HcalEndcapNRawHits",
                                                               "HcalEndcapPInsertRawHits",
                                                               "HcalFarForwardZDCRawHits",
                                                               "LFHCALRawHits"};

  std::vector<std::string> m_calorawhit_collection_names_out = {"B0ECalRawHits",
                                                                "EcalBarrelImagingRawHits",
                                                                "EcalBarrelScFiRawHits",
                                                                "EcalEndcapNRawHits",
                                                                "EcalEndcapPRawHits",
                                                                "EcalFarForwardZDCRawHits",
                                                                "EcalLumiSpecRawHits",
                                                                "HcalBarrelRawHits",
                                                                "HcalEndcapNRawHits",
                                                                "HcalEndcapPInsertRawHits",
                                                                "HcalFarForwardZDCRawHits",
                                                                "LFHCALRawHits"};

  std::vector<std::string> m_calorawhitlink_collection_names = {"B0ECalRawHitLinks",
                                                                "EcalBarrelImagingRawHitLinks",
                                                                "EcalBarrelScFiRawHitLinks",
                                                                "EcalEndcapNRawHitLinks",
                                                                "EcalEndcapPRawHitLinks",
                                                                "EcalFarForwardZDCRawHitLinks",
                                                                "EcalLumiSpecRawHitLinks",
                                                                "HcalBarrelRawHitLinks",
                                                                "HcalEndcapNRawHitLinks",
                                                                "HcalEndcapPInsertRawHitLinks",
                                                                "HcalFarForwardZDCRawHitLinks",
                                                                "LFHCALRawHitLinks"};

  std::vector<std::string> m_simcalorimeterhit_collection_names_out = {
      "B0ECalHits",      "EcalBarrelImagingHits", "EcalBarrelScFiHits",    "EcalEndcapNHits",
      "EcalEndcapPHits", "EcalFarForwardZDCHits", "EcalLumiSpecHits",      "HcalBarrelHits",
      "HcalEndcapNHits", "HcalEndcapPInsertHits", "HcalFarForwardZDCHits", "LFHCALHits"};

  std::vector<std::string> m_calorechit_collection_names_in = {"B0ECalRecHits_aligned",
                                                               "EcalBarrelImagingRecHits_aligned",
                                                               "EcalBarrelScFiRecHits_aligned",
                                                               "EcalEndcapNRecHits_aligned",
                                                               "EcalEndcapPRecHits_aligned",
                                                               "EcalFarForwardZDCRecHits_aligned",
                                                               "EcalLumiSpecRecHits_aligned",
                                                               "HcalBarrelRecHits_aligned",
                                                               "HcalEndcapNRecHits_aligned",
                                                               "HcalEndcapPInsertRecHits_aligned",
                                                               "HcalFarForwardZDCRecHits_aligned",
                                                               "LFHCALRecHits_aligned"};

  std::vector<std::string> m_calorechit_collection_names_out = {"B0ECalRecHits",
                                                                "EcalBarrelImagingRecHits",
                                                                "EcalBarrelScFiRecHits",
                                                                "EcalEndcapNRecHits",
                                                                "EcalEndcapPRecHits",
                                                                "EcalFarForwardZDCRecHits",
                                                                "EcalLumiSpecRecHits",
                                                                "HcalBarrelRecHits",
                                                                "HcalEndcapNRecHits",
                                                                "HcalEndcapPInsertRecHits",
                                                                "HcalFarForwardZDCRecHits",
                                                                "LFHCALRecHits"};

  std::vector<std::string> m_calorechitassociation_collection_names_in = {
      "B0ECalRawHitAssociations",
      "EcalBarrelImagingRawHitAssociations",
      "EcalBarrelScFiRawHitAssociations",
      "EcalEndcapNRawHitAssociations",
      "EcalEndcapPRawHitAssociations",
      "EcalFarForwardZDCRawHitAssociations",
      "EcalLumiSpecRawHitAssociations",
      "HcalBarrelRawHitAssociations",
      "HcalEndcapNRawHitAssociations",
      "HcalEndcapPInsertRawHitAssociations",
      "HcalFarForwardZDCRawHitAssociations",
      "LFHCALRawHitAssociations"};

  std::vector<std::string> m_calorechitassociation_collection_names_out = {
      "B0ECalRawHitAssociations",
      "EcalBarrelImagingRawHitAssociations",
      "EcalBarrelScFiRawHitAssociations",
      "EcalEndcapNRawHitAssociations",
      "EcalEndcapPRawHitAssociations",
      "EcalFarForwardZDCRawHitAssociations",
      "EcalLumiSpecRawHitAssociations",
      "HcalBarrelRawHitAssociations",
      "HcalEndcapNRawHitAssociations",
      "HcalEndcapPInsertRawHitAssociations",
      "HcalFarForwardZDCRawHitAssociations",
      "LFHCALRawHitAssociations"};

  PodioInput<edm4hep::EventHeader> m_event_header_in{this,
                                                     {.name = "EventHeader", .is_optional = true}};
  PodioOutput<edm4hep::EventHeader> m_event_header_out{this, "EventHeader"};

  PodioInput<edm4hep::MCParticle> m_mcparticles_in{this, {.name = "MCParticles"}};
  PodioOutput<edm4hep::MCParticle> m_mcparticles_out{this, "MCParticles"};

  VariadicPodioInput<edm4eic::TrackerHit> m_trackerhits_in{
      this, {.names = m_trackerhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4eic::TrackerHit> m_trackerhits_out{this,
                                                             m_trackerhit_collection_names_out};

  VariadicPodioInput<edm4eic::MCRecoTrackerHitAssociation> m_trackerhitsAsso_in{
      this, {.names = m_simtrackerhitAsso_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4eic::MCRecoTrackerHitAssociation> m_trackerhitsAsso_out{
      this, m_simtrackerhitAsso_collection_names_out};

  VariadicPodioOutput<edm4eic::MCRecoTrackerHitLink> m_rawhitlinks_out{
      this, m_rawhitlink_collection_names_out};
  VariadicPodioOutput<edm4hep::SimTrackerHit> m_simtrackerhits_out{
      this, m_simtrackerhit_collection_names_out};

  VariadicPodioInput<edm4eic::RawTrackerHit> m_rawhit_in{
      this, {.names = m_rawhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4eic::RawTrackerHit> m_rawhit_out{this, m_rawhit_collection_names_out};

  VariadicPodioInput<edm4hep::RawCalorimeterHit> m_calorawhit_in{
      this, {.names = m_calorawhit_collection_names_in, .is_optional = true}};
  VariadicPodioOutput<edm4hep::RawCalorimeterHit> m_calorawhit_out{
      this, m_calorawhit_collection_names_out};

  VariadicPodioOutput<edm4eic::MCRecoCalorimeterHitLink> m_calorawhitlinks_out{
      this, m_calorawhitlink_collection_names};
  VariadicPodioOutput<edm4hep::SimCalorimeterHit> m_simcalorimeterhits_out{
      this, m_simcalorimeterhit_collection_names_out};

  VariadicPodioInput<edm4eic::CalorimeterHit> m_calorechit_in{
      this, {.names = m_calorechit_collection_names_in, .is_optional = true}};
  VariadicPodioOutput<edm4eic::CalorimeterHit> m_calorechit_out{this,
                                                                m_calorechit_collection_names_out};

  VariadicPodioInput<edm4eic::MCRecoCalorimeterHitAssociation> m_calorechitassociation_in{
      this, {.names = m_calorechitassociation_collection_names_in, .is_optional = true}};
  VariadicPodioOutput<edm4eic::MCRecoCalorimeterHitAssociation> m_calorechitassociation_out{
      this, m_calorechitassociation_collection_names_out};

  PodioOutput<edm4hep::EventHeader> m_event_header_phy_out{this, "EventHeader_PHY"};
  PodioOutput<edm4hep::EventHeader> m_event_header_bkg_out{this, "EventHeader_BKG"};

  // For QA
  PodioOutput<edm4hep::EventHeader> m_ecalhitsintower_phy_out{this, "ECalHitsInTower_PHY"};
  PodioOutput<edm4hep::EventHeader> m_ecalhitsintower_bkg_out{this, "ECalHitsInTower_BKG"};

  PodioOutput<edm4hep::EventHeader> m_ecaltowers_phy_out{this, "ECalTowers_PHY"};
  PodioOutput<edm4hep::EventHeader> m_ecaltowers_bkg_out{this, "ECalTowers_BKG"};

  TimeframeSplitter();

  std::vector<std::tuple<size_t, const edm4eic::TrackerHitCollection*, size_t>>
      m_hitStartIndices_simTracker;
  std::vector<std::tuple<size_t, const edm4hep::SimCalorimeterHitCollection*, size_t>>
      m_hitStartIndices_simCalorimeter;

  // == Global Variables =======================
  bool bInitialLoop = true;

  Int_t m_multiTriggerThreshold[4] = {1, 4, 20, 20};
  size_t iniTrkHitPoint[15]        = {0}; // B0Trk,
  size_t iniCalHitPoint[15]        = {0}; // B0Trk,
  bool m_bDetLastHits[10] = {false, false, false, false, false, false, false, false, false, false};

  bool m_bOnceTriggered        = false;
  bool m_bScanedAllTimeWindows = false;

  Int_t targetDetId                     = 0;
  size_t iTimeSlice                     = 0;
  std::vector<Double_t> m_vPhysCooTimes = {};
  // == Global Variables =======================

  struct TimeWindowSummary {
    size_t count            = 0;
    Double_t time_sum       = 0.0;
    size_t next_start_index = 0;

    Double_t average_time() const { return count == 0 ? 0.0 : time_sum / count; }
  };

  using TrackerAssociationIndex     = std::unordered_map<std::uint64_t, std::vector<size_t>>;
  using CalorimeterAssociationIndex = std::unordered_map<std::uint64_t, std::vector<size_t>>;

  std::vector<TrackerAssociationIndex> m_tracker_association_indices;
  std::vector<CalorimeterAssociationIndex> m_calorimeter_association_indices;

  static std::uint64_t object_id_key(const podio::ObjectID& object_id);

  static TrackerAssociationIndex
  buildTrkAssoId(const edm4eic::MCRecoTrackerHitAssociationCollection* associations);

  static CalorimeterAssociationIndex
  buildCalAssoId(const edm4eic::MCRecoCalorimeterHitAssociationCollection* associations);

  static bool overlaps_time_window(Double_t hitTime, Double_t resolution, Double_t window_start,
                                   Double_t window_end);
  static bool is_after_time_window(Double_t hitTime, Double_t resolution, Double_t window_end);

  static bool isValidEtaPhiBin(Int_t etaBin, Int_t phiBin);

  static bool is_hit_in_time_slice(Double_t hitTime, Double_t time_resolution,
                                   Double_t time_slice_start, Double_t time_slice_end);

  template <typename HitT>
  inline void etaPhiCalc(const HitT& hit, Double_t& hitEta, Double_t& hitPhi) {
    const Double_t hitX = hit.getPosition()[0];
    const Double_t hitY = hit.getPosition()[1];
    const Double_t hitZ = hit.getPosition()[2];
    const Double_t hitR = TMath::Sqrt(hitX * hitX + hitY * hitY + hitZ * hitZ);
    if (hitR <= 0.0) {
      hitEta = 0.0;
      hitPhi = 0.0;
      return;
    }
    const Double_t cosTheta = std::clamp(hitZ / hitR, -1.0, 1.0);
    const Double_t hitTheta = TMath::ACos(cosTheta);
    hitEta                  = -TMath::Log(TMath::Tan(hitTheta / 2.0));
    hitPhi                  = TMath::ATan2(hitY, hitX);
  }

  static std::pair<Int_t, Int_t> etaPhiBins(Double_t hitEta, Double_t hitPhi, Double_t etaMin,
                                            Double_t etaMax, Int_t bShift);

  template <typename CollectionT, typename BinFunc>
  void fillEtaPhiGrids(const CollectionT* hits, size_t& iniHitID, Double_t timeResolution,
                       Double_t timeSliceStart, Double_t timeSliceEnd, EtaPhiGrid& grid,
                       EtaPhiGrid& gridShifted, EtaPhiTimeGrid& gridTime,
                       EtaPhiTimeGrid& gridShiftedTime, BinFunc binFunc) {
    if (hits == nullptr)
      return;

    const size_t hitCount = hits->size();
    for (size_t iHit = iniHitID; iHit < hitCount; ++iHit) {
      const auto& hit     = hits->at(iHit);
      const Double_t hitT = hit.getTime();
      if (hitT - timeResolution > timeSliceEnd) {
        iniHitID = iHit;
        break;
      }
      if (!is_hit_in_time_slice(hitT, timeResolution, timeSliceStart, timeSliceEnd))
        continue;

      Double_t hitEta = 0.0;
      Double_t hitPhi = 0.0;
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
                              Double_t timeResolution, Double_t timeSliceStart,
                              Double_t timeSliceEnd, const EtaPhiGrid& baseGrid,
                              const EtaPhiGrid& baseGridShifted, EtaPhiGrid& compGrid,
                              EtaPhiGrid& compGridShifted, Int_t baseThreshold,
                              EtaPhiTimeGrid& compGridTime, EtaPhiTimeGrid& compGridShiftedTime,
                              BinFunc binFunc) {
    if (collection == nullptr)
      return;

    const size_t hitCount = collection->size();
    for (size_t iHit = iniHitID; iHit < hitCount; ++iHit) {
      const Double_t hitT = collection->at(iHit).getTime();
      if (hitT - timeResolution > timeSliceEnd) {
        iniHitID = iHit;
        break;
      }
      if (!is_hit_in_time_slice(hitT, timeResolution, timeSliceStart, timeSliceEnd))
        continue;

      Double_t hitEta = 0.0;
      Double_t hitPhi = 0.0;
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
                                               const EtaPhiTimeGrid& gridShiftedTime,
                                               Int_t threshold, Double_t& averageTime);

  static Double_t averageSelectedTriggerTime(const std::array<Double_t, 8>& values,
                                             const std::array<Double_t, 8>& times,
                                             std::initializer_list<size_t> indices,
                                             Double_t fallbackTime);

  Double_t tracker_time_resolution(size_t detector_id);

  template <typename CollectionT>
  TimeWindowSummary count_hits_in_window(const CollectionT* collection, size_t start_index,
                                         Double_t resolution, Double_t window_start,
                                         Double_t window_end) const {
    TimeWindowSummary summary;
    summary.next_start_index = start_index;
    if (collection == nullptr)
      return summary;

    for (size_t i = start_index; i < collection->size(); ++i) {
      const auto& hit        = collection->at(i);
      const Double_t hitTime = hit.getTime();
      if (is_after_time_window(hitTime, resolution, window_end))
        break;
      if (overlaps_time_window(hitTime, resolution, window_start, window_end)) {
        ++summary.count;
        summary.time_sum += hitTime;
        summary.next_start_index = i;
      }
    }
    return summary;
  }

  template <typename TrackerHitOutputT, typename RawHitOutputT, typename AssociationOutputT>
  static void copy_tracker_hit_with_relations(
      const edm4eic::TrackerHit& tracker_hit,
      const edm4eic::MCRecoTrackerHitAssociationCollection* associations,
      const TrackerAssociationIndex& association_index, TrackerHitOutputT& tracker_hits_out,
      RawHitOutputT& raw_hits_out, AssociationOutputT& associations_out,
      std::unique_ptr<edm4hep::SimTrackerHitCollection>& simHits_out,
      std::unique_ptr<edm4eic::MCRecoTrackerHitLinkCollection>& links_out,
      std::unique_ptr<edm4hep::MCParticleCollection>& mc_particles_out) {

    auto trackerHitCopied = tracker_hit.clone();
    trackerHitCopied.setRawHit(edm4eic::RawTrackerHit());

    if (associations == nullptr || !tracker_hit.getRawHit().isAvailable()) {
      tracker_hits_out->push_back(trackerHitCopied);
      return;
    }

    const auto rawHitId = tracker_hit.getRawHit().getObjectID();

    auto rawHitCopied = tracker_hit.getRawHit().clone();
    raw_hits_out->push_back(rawHitCopied);
    trackerHitCopied.setRawHit(rawHitCopied);

    const auto assocIterCal = association_index.find(object_id_key(rawHitId));

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

        auto copied_association = associations_out->create();
        copied_association.setWeight(association.getWeight());
        copied_association.setRawHit(rawHitCopied);
        copied_association.setSimHit(simHitCopied);

        auto copied_link = links_out->create();
        copied_link.setWeight(association.getWeight());
        copied_link.setFrom(rawHitCopied);
        copied_link.setTo(simHitCopied);
      }
    }

    tracker_hits_out->push_back(trackerHitCopied);
  }

  static std::pair<Int_t, Int_t> backEndEtaPhiBins(Double_t hitEta, Double_t hitPhi, Int_t bShift);

  static std::pair<Int_t, Int_t> barrelEtaPhiBins(Double_t hitEta, Double_t hitPhi, Int_t bShift);

  static std::pair<Int_t, Int_t> forwardEndEtaPhiBins(Double_t hitEta, Double_t hitPhi,
                                                      Int_t bShift);

  Result Unfold(const JEvent& parent, JEvent& child, int child_idx) override;

  void thetaPhiBinCalc(edm4eic::TrackerHit hit, Int_t& thetaID1, Int_t& phiID1, Int_t& thetaID2,
                       Int_t& phiID2);
};
