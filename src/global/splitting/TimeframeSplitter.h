// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <sys/resource.h>

#include <array>

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

#include <JANA/JEventUnfolder.h>

struct TimeframeSplitter : public JEventUnfolder {


  Parameter<float> timeframe_width{this, "timeframe_width", 2000.0, "Width of each timeframe in ns"};
  Parameter<float> timesplit_width{this, "timesplit_width", 20.0, "Width of each timeslice in ns"};
  Parameter<float> timeResolution_SiMaps{this, "timeResolution_Silicon", 2000.0,
                                          "time resolution of Silicon detector in ns"};
  Parameter<float> timeResolution_MPGD{this, "timeResolution_MPGD = 10.0", 30.0,
                                       "time resolution of MPGD detector in ns"};
  Parameter<float> timeResolution_ACLGad{this, "timeResolution_TOF = 1.0", 0.03,
                                      "time resolution of TOF detector in ns"};
  Parameter<float> timeResolution_EMCal{this, "timeResolution_EMCal = 1.0", 30.0,
                                      "time resolution of EMCal detector in ns"};
  bool m_use_timeframe = false; // Use timeframes to split events, or use timeslices

  size_t m_event_number_ts   = 0;    // Event number for the current timeslice
  size_t m_event_number_orig = 0;    // Event number for the current timeslice
  std::vector<Int_t> m_vTargetEvent; // List of original event numbers for each timeslice

  std::vector<std::string> m_trackerhit_collection_names = {
      "B0TrackerRecHits_TK_aligned",
      "TOFBarrelRecHits_TK_aligned",
      "TOFEndcapRecHits_TK_aligned",
      "MPGDBarrelRecHits_TK_aligned",
      "OuterMPGDBarrelRecHits_TK_aligned",
      "BackwardMPGDEndcapRecHits_TK_aligned",
      "ForwardMPGDEndcapRecHits_TK_aligned",
      "SiBarrelVertexRecHits_TK_aligned",
      "SiBarrelTrackerRecHits_TK_aligned",
      "SiEndcapTrackerRecHits_TK_aligned"
      };
      // "TaggerTrackerRecHits_TK_aligned",
      // "DIRCBarRecHits_TK_aligned",
      // "DRICHRecHits_TK_aligned",
      // "ForwardOffMTrackerRecHits_TK_aligned",
      // "ForwardRomanPotRecHits_TK_aligned",
      // "LumiSpecTrackerRecHits_TK_aligned",
      // "RICHEndcapNRecHits_TK_aligned"
  std::vector<std::string> m_trackerhit_collection_names_out = {
      "B0TrackerRecHits",
      "TOFBarrelRecHits",
      "TOFEndcapRecHits",
      "MPGDBarrelRecHits",
      "OuterMPGDBarrelRecHits",
      "BackwardMPGDEndcapRecHits",
      "ForwardMPGDEndcapRecHits",
      "SiBarrelVertexRecHits",
      "SiBarrelTrackerRecHits",
      "SiEndcapTrackerRecHits"
    };
      // "TaggerTrackerRecHits",
      // "DIRCBarRecHits",
      // "DRICHRecHits",
      // "ForwardOffMTrackerRecHits",
      // "ForwardRomanPotRecHits",
      // "LumiSpecTrackerRecHits",
      // "RICHEndcapNRecHits"

  std::vector<std::string> m_simtrackerhitAsso_collection_names = {
      "B0TrackerRawHitAssociations_TK",
      "TOFBarrelRawHitAssociations_TK",
      "TOFEndcapRawHitAssociations_TK",
      "MPGDBarrelRawHitAssociations_TK",
      "OuterMPGDBarrelRawHitAssociations_TK",
      "BackwardMPGDEndcapRawHitAssociations_TK",
      "ForwardMPGDEndcapRawHitAssociations_TK",
      "SiBarrelVertexRawHitAssociations_TK",
      "SiBarrelRawHitAssociations_TK",
      "SiEndcapTrackerRawHitAssociations_TK"
    };
    // ,"TaggerTrackerRawHitAssociations_TK",
    //   "DIRCBarRawHitsAssociations_TK",
    //   "DRICHRawHitAssociations_TK",
    //   "ForwardOffMTrackerRawHitAssociations_TK",
    //   "ForwardRomanPotRawHitAssociations_TK",
    //   "LumiSpecTrackerRawHitAssociations_TK",
    //   "RICHEndcapNRawHitAssociations_TK"
  std::vector<std::string> m_simtrackerhitAsso_collection_names_out = {
      "B0TrackerRawHitAssociations",
      "TOFBarrelRawHitAssociations",
      "TOFEndcapRawHitAssociations",
      "MPGDBarrelRawHitAssociations",
      "OuterMPGDBarrelRawHitAssociations",
      "BackwardMPGDEndcapRawHitAssociations",
      "ForwardMPGDEndcapRawHitAssociations",
      "SiBarrelVertexRawHitAssociations",
      "SiBarrelRawHitAssociations",
      "SiEndcapTrackerRawHitAssociations"
    };
      // "TaggerTrackerRawHitAssociations",
      // "DIRCBarRawHitAssociations",
      // "DRICHRawHitsAssociations",
      // "ForwardOffMTrackerRawHitAssociations",
      // "ForwardRomanPotRawHitAssociations",
      // "LumiSpecTrackerRawHitAssociations",
      // "RICHEndcapNRawHitsAssociations"

  std::vector<std::string> m_rawhitlink_collection_names = {
    "B0TrackerRawHitLinks_TK",
    "TOFBarrelRawHitLinks_TK",
    "TOFEndcapRawHitLinks_TK",
    "MPGDBarrelRawHitLinks_TK",
    "OuterMPGDBarrelRawHitLinks_TK",
    "BackwardMPGDEndcapRawHitLinks_TK",
    "ForwardMPGDEndcapRawHitLinks_TK",
    "SiBarrelVertexRawHitLinks_TK",
    "SiBarrelRawHitLinks_TK",
    "SiEndcapTrackerRawHitLinks_TK"
  };
  // "TaggerTrackerRawHitLinks_TK",
  //   "DIRCBarRawHitLinks_TK",
  //   "DRICHRawHitLinks_TK",
  //   "ForwardOffMTrackerRawHitLinks_TK",
  //   "ForwardRomanPotRawHitLinks_TK",
  //   "LumiSpecTrackerRawHitLinks_TK",
  //   "RICHEndcapNRawHitsLinks_TK"

  std::vector<std::string> m_rawhitlink_collection_names_out = {
      "B0TrackerRawHitLinks",
      "TOFBarrelRawHitLinks",
      "TOFEndcapRawHitLinks",
      "MPGDBarrelRawHitLinks",
      "OuterMPGDBarrelRawHitLinks",
      "BackwardMPGDEndcapRawHitLinks",
      "ForwardMPGDEndcapRawHitLinks",
      "SiBarrelVertexRawHitLinks",
      "SiBarrelRawHitLinks",
      "SiEndcapTrackerRawHitLinks"
    };
    // "TaggerTrackerRawHitLinks",
    //   "DIRCBarRawHitLinks",
    //   "DRICHRawHitsLinks",
    //   "ForwardOffMTrackerRawHitLinks",
    //   "ForwardRomanPotRawHitLinks",
    //   "LumiSpecTrackerRawHitLinks",
    //   "RICHEndcapNRawHitsLinks"

  std::vector<std::string> m_rawhit_collection_names = {
      "B0TrackerRawHits_TK",
      "TOFBarrelRawHits_TK",
      "TOFEndcapRawHits_TK",
      "MPGDBarrelRawHits_TK",
      "OuterMPGDBarrelRawHits_TK",
      "BackwardMPGDEndcapRawHits_TK",
      "ForwardMPGDEndcapRawHits_TK",
      "SiBarrelVertexRawHits_TK",
      "SiBarrelRawHits_TK",
      "SiEndcapTrackerRawHits_TK"
  };
    // "TaggerTrackerRawHits_TK",
    // "DIRCBarRawHits_TK",
    // "DRICHRawHits_TK",
    // "ForwardOffMTrackerRawHits_TK",
    // "ForwardRomanPotRawHits_TK",
    // "LumiSpecTrackerRawHits_TK",
    // "RICHEndcapNRawHits_TK"

  std::vector<std::string> m_rawhit_collection_names_out = {
      "B0TrackerRawHits",
      "TOFBarrelRawHits",
      "TOFEndcapRawHits",
      "MPGDBarrelRawHits",
      "OuterMPGDBarrelRawHits",
      "BackwardMPGDEndcapRawHits",
      "ForwardMPGDEndcapRawHits",
      "SiBarrelVertexRawHits",
      "SiBarrelRawHits",
      "SiEndcapTrackerRawHits"
    };
// "TaggerTrackerRawHits",
//       "DIRCBarRawHits",
//       "DRICHRawHits",
//       "ForwardOffMTrackerRawHits",
//       "ForwardRomanPotRawHits",
//       "LumiSpecTrackerRawHits",
//       "RICHEndcapNRawHits"


  std::vector<std::string> m_calorawhit_collection_names_in = {
      "B0ECalRawHits_TK_aligned",
      "EcalBarrelImagingRawHits_TK_aligned",
      "EcalBarrelScFiRawHits_TK_aligned",
      "EcalEndcapNRawHits_TK_aligned",
      "EcalEndcapPRawHits_TK_aligned",
      "EcalFarForwardZDCRawHits_TK_aligned",
      "EcalLumiSpecRawHits_TK_aligned"
  };
      // "HcalBarrelRawHits_TK_aligned",
      // "HcalEndcapNRawHits_TK_aligned",
      // "HcalEndcapPInsertRawHits_TK_aligned",
      // "HcalFarForwardZDCRawHits_TK_aligned",
      // "LFHCALRawHits_TK_aligned"

  std::vector<std::string> m_calorawhit_collection_names_out = {
      "B0ECalRawHits",
      "EcalBarrelImagingRawHits",
      "EcalBarrelScFiRawHits",
      "EcalEndcapNRawHits",
      "EcalEndcapPRawHits",
      "EcalFarForwardZDCRawHits",
      "EcalLumiSpecRawHits"
  };
      //   "HcalBarrelRawHits",
      // "HcalEndcapNRawHits",
      // "HcalEndcapPInsertRawHits",
      // "HcalFarForwardZDCRawHits",
      // "LFHCALRawHits"


  std::vector<std::string> m_calorechit_collection_names_in = {
      "B0ECalRecHits_TK_aligned",
      "EcalBarrelImagingRecHits_TK_aligned",
      "EcalBarrelScFiRecHits_TK_aligned",
      "EcalEndcapNRecHits_TK_aligned",
      "EcalEndcapPRecHits_TK_aligned",
      "EcalFarForwardZDCRecHits_TK_aligned",
      "EcalLumiSpecRecHits_TK_aligned"
  };
      // "HcalBarrelRecHits_TK_aligned",
      // "HcalEndcapNRecHits_TK_aligned",
      // "HcalEndcapPInsertRecHits_TK_aligned",
      // "HcalFarForwardZDCRecHits_TK_aligned",
      // "LFHCALRecHits_TK_aligned"

  std::vector<std::string> m_calorechit_collection_names_out = {
      "B0ECalRecHits",
      "EcalBarrelImagingRecHits",
      "EcalBarrelScFiRecHits",
      "EcalEndcapNRecHits",
      "EcalEndcapPRecHits",
      "EcalFarForwardZDCRecHits",
      "EcalLumiSpecRecHits"
  };
      //   "HcalBarrelRecHits",
      // "HcalEndcapNRecHits",
      // "HcalEndcapPInsertRecHits",
      // "HcalFarForwardZDCRecHits",
      // "LFHCALRecHits"

  std::vector<std::string> m_calorechitassociation_collection_names_in = {
      "B0ECalRawHitAssociations_TK",
      "EcalBarrelImagingRawHitAssociations_TK",
      "EcalBarrelScFiRawHitAssociations_TK",
      "EcalEndcapNRawHitAssociations_TK",
      "EcalEndcapPRawHitAssociations_TK",
      "EcalFarForwardZDCRawHitAssociations_TK",
      "EcalLumiSpecRawHitAssociations_TK"
  };
  std::vector<std::string> m_calorechitassociation_collection_names_out = {
      "B0ECalRawHitAssociations",
      "EcalBarrelImagingRawHitAssociations",
      "EcalBarrelScFiRawHitAssociations",
      "EcalEndcapNRawHitAssociations",
      "EcalEndcapPRawHitAssociations",
      "EcalFarForwardZDCRawHitAssociations",
      "EcalLumiSpecRawHitAssociations"
  };


  // std::vector<std::string> m_simcalorimeterhit_collection_names = {
  //     "B0ECalHits",      "EcalBarrelImagingHits", "EcalBarrelScFiHits",    "EcalEndcapNHits",
  //     "EcalEndcapPHits", "EcalEndcapPInsertHits", "EcalFarForwardZDCHits", "EcalLumiSpecHits",
  //     "HcalBarrelHits",  "HcalEndcapNHits",       "HcalEndcapPInsertHits", "HcalFarForwardZDCHits",
  //     "LFHCALHits",      "LumiDirectPCALHits"};

  // std::vector<std::string> m_calohitcontribution_collection_names = {
  //     "B0ECalHitsContributions",
  //     "EcalBarrelImagingHitsContributions",
  //     "EcalBarrelScFiHitsContributions",
  //     "EcalEndcapNHitsContributions",
  //     "EcalEndcapPHitsContributions",
  //     "EcalEndcapPInsertHitsContributions",
  //     "EcalLumiSpecHitsContributions",
  //     "EcalFarForwardZDCHitsContributions",
  //     "HcalBarrelHitsContributions",
  //     "HcalEndcapNHitsContributions",
  //     "HcalEndcapPInsertHitsContributions",
  //     "HcalFarForwardZDCHitsContributions",
  //     "LFHCALHitsContributions",
  //     "LumiDirectPCALHitsContributions"};

   std::vector<std::string> m_calocluster_collection_names_in = {
      "B0ECalClusters_TK_aligned",
      "EcalBarrelClusters_TK_aligned",
      "EcalEndcapNClusters_TK_aligned",
      "EcalEndcapPClusters_TK_aligned",
    };
      //     "EcalFarForwardZDCClusters_TK_aligned",
      // "EcalLumiSpecClusters_TK_aligned",
      // "HcalBarrelClusters_TK_aligned",
      // "HcalEndcapNClusters_TK_aligned",
      // "HcalEndcapPInsertClusters_TK_aligned",
      // "HcalFarForwardZDCClusters_TK_aligned",
      // "LFHCALClusters_TK_aligned",
      // "EcalBarrelImagingClusters_TK_aligned",
      // "EcalBarrelScFiClusters_TK_aligned",
      // "EcalEndcapNImagingClusters_TK_aligned",
      // "EcalEndcapPImagingClusters_TK_aligned",
      // "EcalFarForwardZDCImagingClusters_TK_aligned",
      // "EcalLumiSpecImagingClusters_TK_aligned"

     std::vector<std::string> m_calocluster_collection_names_out = {
      "B0ECalClusters",
      "EcalBarrelClusters",
      "EcalEndcapNClusters",
      "EcalEndcapPClusters",
    };
      // "EcalFarForwardZDCClusters",
      // "EcalLumiSpecClusters",
      // "HcalBarrelClusters",
      // "HcalEndcapNClusters",
      // "HcalEndcapPInsertClusters",
      // "HcalFarForwardZDCClusters",
      // "LFHCALClusters",
      // "EcalBarrelImagingClusters",
      // "EcalBarrelScFiClusters",
      // "EcalEndcapNImagingClusters",
      // "EcalEndcapPImagingClusters",
      // "EcalFarForwardZDCImagingClusters",
      // "EcalLumiSpecImagingClusters"

  std::vector<std::string> m_caloclusterassociation_collection_names_in = {
      "B0ECalClusterAssociations_TK",
      "EcalBarrelClusterAssociations_TK",
      "EcalEndcapNClusterAssociations_TK",
      "EcalEndcapPClusterAssociations_TK",
    };
      // "HcalBarrelClusterAssociations_TK",
      // "HcalEndcapNClusterAssociations_TK",
      // "HcalEndcapPInsertClusterAssociations_TK",
      // "HcalFarForwardZDCClusterAssociations_TK",
      // "LFHCALClusterAssociations_TK",
      // "EcalBarrelImagingClusterAssociations_TK",
      // "EcalBarrelScFiClusterAssociations_TK",

    std::vector<std::string> m_caloclusterassociation_collection_names_out = {
      "B0ECalClusterAssociations",
      "EcalBarrelClusterAssociations",
      "EcalEndcapNClusterAssociations",
      "EcalEndcapPClusterAssociations",
    };
      //     "HcalBarrelClusterAssociations",
      // "HcalEndcapNClusterAssociations",
      // "HcalEndcapPInsertClusterAssociations",
      // "HcalFarForwardZDCClusterAssociations",
      // "LFHCALClusterAssociations",
      // "EcalBarrelImagingClusterAssociations",
      // "EcalBarrelScFiClusterAssociations",

  PodioInput<edm4hep::EventHeader> m_event_header_in{this,
                                                     {.name = "EventHeader", .is_optional = true}};
  PodioOutput<edm4hep::EventHeader> m_event_header_out{this, "EventHeader"};

  PodioInput<edm4hep::MCParticle> m_mcparticles_in{this, {.name = "MCParticles"}};
  PodioOutput<edm4hep::MCParticle> m_mcparticles_out{this, "MCParticles"};

  VariadicPodioInput<edm4eic::TrackerHit> m_trackerhits_in{
      this, {.names = m_trackerhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4eic::TrackerHit> m_trackerhits_out{
      this, m_trackerhit_collection_names_out};

  VariadicPodioInput<edm4eic::MCRecoTrackerHitAssociation> m_trackerhitsAsso_in{
      this, {.names = m_simtrackerhitAsso_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4eic::MCRecoTrackerHitAssociation> m_trackerhitsAsso_out{
      this, m_simtrackerhitAsso_collection_names_out};

  VariadicPodioInput<podio::Link<edm4eic::RawTrackerHit, edm4hep::SimTrackerHit>> m_rawhitlinks_in{
      this, {.names = m_rawhitlink_collection_names, .is_optional = true}};
  VariadicPodioOutput<podio::Link<edm4eic::RawTrackerHit, edm4hep::SimTrackerHit>>
      m_rawhitlinks_out{this, m_rawhitlink_collection_names_out};

  VariadicPodioInput<edm4eic::RawTrackerHit> m_rawhit_in{
      this, {.names = m_rawhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4eic::RawTrackerHit> m_rawhit_out{this, m_rawhit_collection_names_out};

  // VariadicPodioInput<edm4hep::SimCalorimeterHit> m_simcalorimeterhits_in{
  //     this, {.names = m_simcalorimeterhit_collection_names, .is_optional = true}};
  // VariadicPodioOutput<edm4hep::SimCalorimeterHit> m_simcalorimeterhits_out{
  //     this, m_simcalorimeterhit_collection_names};

  // VariadicPodioInput<edm4hep::CaloHitContribution> m_calohitcontributions_in{
  //     this, {.names = m_calohitcontribution_collection_names, .is_optional = true}};
  // VariadicPodioOutput<edm4hep::CaloHitContribution> m_calohitcontributions_out{
  //     this, m_calohitcontribution_collection_names};

  VariadicPodioInput<edm4hep::RawCalorimeterHit> m_calorawhit_in{
      this, {.names = m_calorawhit_collection_names_in, .is_optional = true}};
  VariadicPodioOutput<edm4hep::RawCalorimeterHit> m_calorawhit_out{
      this, m_calorawhit_collection_names_out};

  VariadicPodioInput<edm4eic::CalorimeterHit> m_calorechit_in{
      this, {.names = m_calorechit_collection_names_in, .is_optional = true}};
  VariadicPodioOutput<edm4eic::CalorimeterHit> m_calorechit_out{
      this, m_calorechit_collection_names_out};

  VariadicPodioInput<edm4eic::MCRecoCalorimeterHitAssociation> m_calorechitassociation_in{
      this, {.names = m_calorechitassociation_collection_names_in, .is_optional = true}};
  VariadicPodioOutput<edm4eic::MCRecoCalorimeterHitAssociation> m_calorechitassociation_out{
      this, m_calorechitassociation_collection_names_out};

  VariadicPodioInput<edm4eic::Cluster> m_calocluster_in{
      this, {.names = m_calocluster_collection_names_in, .is_optional = true}};
  VariadicPodioOutput<edm4eic::Cluster> m_calocluster_out{
      this, m_calocluster_collection_names_out};

  VariadicPodioInput<edm4eic::MCRecoClusterParticleAssociation> m_caloclusterassociation_in{
      this, {.names = m_caloclusterassociation_collection_names_in, .is_optional = true}};
  VariadicPodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_caloclusterassociation_out{
      this, m_caloclusterassociation_collection_names_out};

  PodioOutput<edm4hep::EventHeader> m_event_header_ts_out{this, "EventHeader_TS"};
  PodioOutput<edm4hep::EventHeader> m_event_header_phy_out{this, "EventHeader_PHY"};
  PodioOutput<edm4hep::EventHeader> m_event_header_bkg_out{this, "EventHeader_BKG"};

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
  size_t m_triggerDetSize = 10; // Number of detectors used for triggering
  // Int_t m_detId[10] = {12, 13, 1, 4, 8, 9, 11, 14, 15, 16}; // TOF and MPGD, Silicon excluded
  Int_t m_detId[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; // TOF and MPGD, Silicon excluded

  bool bInitialLoop = true;
  std::vector<std::vector<unsigned int>> m_vOrigHitId;
  std::vector<std::vector<unsigned int>> m_vSameTSHitId;

  Int_t m_multiTriggerThreshold[4] = {1, 4, 20, 20};
  size_t iniTrkHitPoint[15] = {0}; // B0Trk,
  size_t iniCalHitPoint[15] = {0}; // B0Trk,
  bool m_bDetLastHits[10]        = {false, false, false, false, false, false, false, false, false, false};

  bool m_bOnceTriggered        = false;
  bool m_bScanedAllTimeWindows = false;

  Int_t targetDetId                     = 0;
  size_t iTimeSlice                     = 0;
  std::vector<Double_t> m_vPhysCooTimes = {};
  // == Global Variables =======================

  struct TimeWindowSummary {
    size_t count = 0;
    Double_t time_sum = 0.0;
    size_t next_start_index = 0;

    Double_t average_time() const {
      return count == 0 ? 0.0 : time_sum / count;
    }
  };

  static bool overlaps_time_window(Double_t hit_time, Double_t resolution,\
    Double_t window_start,Double_t window_end) {
    return hit_time + resolution > window_start && hit_time - resolution < window_end;
  }
  static bool is_after_time_window(Double_t hit_time, Double_t resolution, Double_t window_end) {
    return hit_time - resolution >= window_end;
  }


  Double_t tracker_time_resolution(size_t detector_id) {
    if (detector_id < 3) return timeResolution_ACLGad();
    if (detector_id < 7) return timeResolution_MPGD();
    return timeResolution_SiMaps();
  }

  template <typename CollectionT>
  TimeWindowSummary count_hits_in_window(const CollectionT* collection,\
    size_t start_index, Double_t resolution,\
    Double_t window_start, Double_t window_end) const {
    TimeWindowSummary summary;
    summary.next_start_index = start_index;
    if (collection == nullptr) return summary;

    bool found_next_start = false;
    for (size_t i = start_index; i < collection->size(); ++i) {
      const auto& hit = collection->at(i);
      const Double_t hit_time = hit.getTime();

      if (is_after_time_window(hit_time, resolution, window_end)) {
        summary.next_start_index = i;
        break;
      }
      if (overlaps_time_window(hit_time, resolution, window_start, window_end)) {
        ++summary.count;
        summary.time_sum += hit_time;
      }
    }
    return summary;
  }

  Result Unfold(const JEvent& parent, JEvent& child, int child_idx) override {
    std::cout << " <><><><> TimeframeSplitter: timeslice " << child_idx << " of timeframe "
              << parent.GetEventNumber() << ", targetDetID: " << targetDetId << " <><><><<><>"
              << std::endl;

    float m_timeframe_width = timeframe_width();
    float m_timesplit_width = timesplit_width();

    Bool_t m_bTrigger = false;

    const auto trackerHitCollsIn = m_trackerhits_in();
    const auto caloRecHitCollsIn = m_calorechit_in();
    const auto trkAssoCollsIn = m_trackerhitsAsso_in();
    const auto calrecAssoCollsIn = m_calorechitassociation_in();

    // == s == Register hits of TOF and MPGD detectors in the time slice ==================
    if (child_idx == 0) {
      // == s == For MC Trigger Efficiency Estimation ~~~~~~~~
      m_vPhysCooTimes.clear();

      Double_t prevMCTime = -9999.0; // temp check mc particle times
      for (const auto& mcparticle : *m_mcparticles_in) {
        // if (mcparticle.parents_begin() != 0)
        // if(mcparticle.parents_size() > 0) continue;

        if(mcparticle.getGeneratorStatus() != 1) continue;
        if(std::abs(prevMCTime - mcparticle.getTime()) < 50.) continue;
        Double_t mcCollTime = mcparticle.getTime();
        m_vPhysCooTimes.push_back(mcCollTime);
        prevMCTime = mcCollTime;
      }
      std::sort(m_vPhysCooTimes.begin(), m_vPhysCooTimes.end());
      auto last = std::unique(m_vPhysCooTimes.begin(), m_vPhysCooTimes.end());
      m_vPhysCooTimes.erase(last, m_vPhysCooTimes.end());
      for(size_t iPhysT = 0; iPhysT < m_vPhysCooTimes.size(); ++iPhysT) {
        Double_t physCollTime = m_vPhysCooTimes[iPhysT];
        Double_t tsTime = iPhysT * timesplit_width();
        // std::cout << "111<><>><><<><><<><<><><><> TF:TS = " << parent.GetEventNumber() << " : " << child_idx << ", physCollTime: " << physCollTime << ", tsTime: " << tsTime << std::endl;
      }
      
      // == e == For MC Trigger Efficiency Estimation ~~~~~~~~
    }
    // == e == Register hits of TOF and MPGD detectors in the time slice ==================
    

    // == s == Time frame scan loop ==========================================================
    Double_t timesliceT0 = -999.0;
    Bool_t bTimesliceTrigger = false;

    Bool_t bMutipliTriggers[4] = {false, false, false, false};
    Double_t multipliTrigTime[4] = {0.0, 0.0, 0.0, 0.0};

      
    // Double_t tsTimeS = child_idx * m_timesplit_width;  
    // Double_t tsTimeE = (child_idx + 1) * m_timesplit_width;
    Double_t tsTimeS = iTimeSlice * m_timesplit_width;
    Double_t tsTimeE = (iTimeSlice + 1) * m_timesplit_width;
    while(1){
      tsTimeS = iTimeSlice * m_timesplit_width;
      tsTimeE = (iTimeSlice + 1) * m_timesplit_width;
      if(tsTimeE > m_timeframe_width) break;
      iTimeSlice++;
      
      // == s == Multiplisity shreshold Triggers =======================================


      std::array<size_t, 4> multiHits = {0, 0, 0, 0};
      // 111 s 111 Trigger 1 : B0 ##############################################
      const auto hitsB0 = count_hits_in_window(trackerHitCollsIn.at(0), iniTrkHitPoint[0], timeResolution_ACLGad(), tsTimeS, tsTimeE);
      iniTrkHitPoint[0] = hitsB0.next_start_index;
      multiHits[0] = hitsB0.count;
      if(hitsB0.count > 0) multipliTrigTime[0] = hitsB0.average_time();
      if(multiHits[0] > m_multiTriggerThreshold[0]) bMutipliTriggers[0] = true;
      // 111 s 111 Trigger 1 : B0 ##############################################

      // 222 s 222 Trigger 2 : Endcap TOF & MPGD ##################################
      const auto hitsETOF = count_hits_in_window(trackerHitCollsIn.at(2), iniTrkHitPoint[2],\
                                                timeResolution_ACLGad(), tsTimeS, tsTimeE);
      iniTrkHitPoint[2] = hitsETOF.next_start_index;
      auto hitsEMPGD = count_hits_in_window(trackerHitCollsIn.at(5), iniTrkHitPoint[5],\
                                                timeResolution_MPGD(), tsTimeS, tsTimeE);
      iniTrkHitPoint[5] = hitsEMPGD.next_start_index;
      const auto hitsForwardEMPGD = count_hits_in_window(trackerHitCollsIn.at(6), iniTrkHitPoint[6],\
                                                         timeResolution_MPGD(), tsTimeS, tsTimeE);
      iniTrkHitPoint[6] = hitsForwardEMPGD.next_start_index;
      hitsEMPGD.count += hitsForwardEMPGD.count;
      hitsEMPGD.time_sum += hitsForwardEMPGD.time_sum;
      multiHits[1] = hitsETOF.count + hitsEMPGD.count;
      if(hitsEMPGD.count > 0) multipliTrigTime[1] = hitsEMPGD.average_time();
      if(multiHits[1] > m_multiTriggerThreshold[1]) bMutipliTriggers[1] = true;
      // 222 e 222 Trigger 2 : Endcap TOF & MPGD ##################################

      // 333 s 333 Trigger 3 : Endcap ECal ##################################4
      const auto hitsEPECal = count_hits_in_window(caloRecHitCollsIn.at(4),\
        iniCalHitPoint[4], timeResolution_EMCal(), tsTimeS, tsTimeE);
      iniCalHitPoint[4] = hitsEPECal.next_start_index;
      multiHits[2] = hitsEPECal.count;
      if(hitsEPECal.count > 0) multipliTrigTime[2] = hitsEPECal.average_time();
      if(multiHits[2] > m_multiTriggerThreshold[2]) bMutipliTriggers[2] = true;
      // 333 e 333 Trigger 3 : Endcap ECal ##################################

      // 444 s 444 Trigger 4 : ZDC ECal ##################################5
      const auto hitsZDCECal = count_hits_in_window(caloRecHitCollsIn.at(5), \
        iniCalHitPoint[5], timeResolution_EMCal(), tsTimeS, tsTimeE);
      iniCalHitPoint[5] = hitsZDCECal.next_start_index;
      multiHits[3] = hitsZDCECal.count;
      if(hitsZDCECal.count > 0) multipliTrigTime[3] = hitsZDCECal.average_time();
      if(multiHits[3] > m_multiTriggerThreshold[3]) bMutipliTriggers[3] = true;
      // 444 e 444 Trigger 4 : ZDC ECal ##################################

      if(!bMutipliTriggers[0]&&!bMutipliTriggers[1]&&!bMutipliTriggers[2]&&!bMutipliTriggers[3]) continue;
      // == e == Multiplisity shreshold Triggers =======================================

      // == s == Geometrical Coincidence Triggers =====================================
      // ===  Geometrical Coincidence ===
      // == e == Geometrical Coincidence Triggers =====================================

      for(size_t iTrig = 0; iTrig < 4; ++iTrig){
        std::cout << "Trig" << iTrig << ":: numOfHits:numOfTrhe:TreResult = " << multiHits[iTrig] << ":" << m_multiTriggerThreshold[iTrig] << ":" <<  bMutipliTriggers[iTrig] << std::endl;
      }

      if(bMutipliTriggers[0] || bMutipliTriggers[1] || bMutipliTriggers[2] || bMutipliTriggers[3]) bTimesliceTrigger = true; // ???? temporary, need to be removed after geometrical coincidence trigger is implemented
      if(bTimesliceTrigger) break;
    }
    // == e == Time frame scan loop ==========================================================

    m_bTrigger = bTimesliceTrigger;
    if(bTimesliceTrigger){
      m_bOnceTriggered = true;
      // For now, a one-to-one relationship between timeslices and events
      child.SetEventNumber(parent.GetEventNumber());
      child.SetRunNumber(parent.GetRunNumber());

      std::vector<Int_t> regisMcPIDs = {}; // QA MC particle IDs

      Double_t tsTime = 0.; // ??? temporary, need to be removed after geometrical coincidence trigger is implemented
      if(multipliTrigTime[0] != 0.) tsTime = multipliTrigTime[0];
      else if(multipliTrigTime[1] != 0.) tsTime = multipliTrigTime[1];
      else if(multipliTrigTime[2] != 0.) tsTime = multipliTrigTime[2];
      else if(multipliTrigTime[3] != 0.) tsTime = multipliTrigTime[3];
      
      // == s == Registrer Tracker Hits =======================================================
      for (size_t trkDetID = 0; trkDetID < trackerHitCollsIn.size(); ++trkDetID) {
        const auto* trkCollIn = trackerHitCollsIn.at(trkDetID);
        if (trkCollIn == nullptr) continue;
        auto& trkCollOut  = m_trackerhits_out().at(trkDetID);

        const Double_t detTimeReso = tracker_time_resolution(trkDetID);

        const auto* trkAssoCollIn = trkAssoCollsIn.at(trkDetID);
        auto& rawCollOut  = m_rawhit_out().at(trkDetID);

        for (size_t iHit = 0; iHit < trkCollIn->size(); ++iHit) {
          const auto& trkHit = trkCollIn->at(iHit);

          Double_t hitT = trkHit.getTime();
          if(hitT - detTimeReso > tsTime + 30.){
            iniTrkHitPoint[trkDetID] = iHit;
            continue;
          }
          
          if(overlaps_time_window(hitT, detTimeReso, tsTime - 10., tsTime + 30.)){
            auto copiedTrkHit = trkHit.clone();
            copiedTrkHit.setRawHit(edm4eic::RawTrackerHit());
            trkCollOut->push_back(copiedTrkHit);
            
            // == s == For QA relation valuables QAQAQAQAQAQAQAQAQAQAQAQAQAQAQA
            if (trkAssoCollIn == nullptr) continue;
            auto rawHitFromRec = trkHit.getRawHit();
            auto rawHitID = rawHitFromRec.getObjectID();
            for (const auto& assoc : *trkAssoCollIn) {
                auto rawHitFromAssoc = assoc.getRawHit();
                auto assocRawID = rawHitFromAssoc.getObjectID();
                if(rawHitID.index == assocRawID.index && rawHitID.collectionID == assocRawID.collectionID) {
                  auto simHit = assoc.getSimHit();
                  auto relMcP = simHit.getParticle();
                  auto relMcPId = relMcP.getObjectID();
                  regisMcPIDs.push_back(relMcPId.index);
                  rawCollOut->push_back(rawHitFromAssoc.clone());

                }
            }
            // == e == For QA relation valuables QAQAQAQAQAQAQAQAQAQAQAQAQAQAQA
          }

          
        }
      }
      // == e == Registrer Tracker Hits =======================================================

      // == s == Registrer Calo Rec Hits =======================================================
      for (size_t calDetID = 0; calDetID < caloRecHitCollsIn.size(); ++calDetID) {
        const auto* caloInColl = caloRecHitCollsIn.at(calDetID);
        if (caloInColl == nullptr) continue;
        auto& caloOutColl = m_calorechit_out().at(calDetID);

        const auto* caloInCollAsso = calrecAssoCollsIn.at(calDetID);
        if (caloInCollAsso == nullptr) continue;

        for (size_t iCalHit = 0; iCalHit < caloInColl->size(); ++iCalHit) {
          const auto& caloHit = caloInColl->at(iCalHit);

          Double_t detTimeReso = timeResolution_EMCal();

          Double_t hitT = caloHit.getTime();
          if(overlaps_time_window(hitT, detTimeReso, tsTime - 10., tsTime + 30.)){
            auto copiedCaloHit = caloHit.clone();
            copiedCaloHit.setRawHit(edm4hep::RawCalorimeterHit());
            caloOutColl->push_back(copiedCaloHit);

            // == s == For QA relation valuables QAQAQAQAQAQAQAQAQAQAQAQAQAQAQA
            // const auto& rawCollOut = m_rawhit_out().at(calDetID);
            const auto* calrecAssoCollIn  = calrecAssoCollsIn.at(calDetID);
            auto& rawCollOut  = m_calorawhit_out().at(calDetID);

            // auto& linkCollIn = m_rawhitlinks_in().at(calDetID);
            // auto& linkCollOut = m_rawhitlinks_out().at(calDetID);
            // linkCollOut->setSubsetCollection();

            auto rawHitFromRec = caloHit.getRawHit();
            auto rawHitID = rawHitFromRec.getObjectID();

            for (const auto& assoc : *calrecAssoCollIn) {
                auto rawHitFromAssoc = assoc.getRawHit();
                auto assocRawID = rawHitFromAssoc.getObjectID();
                if(rawHitID.index == assocRawID.index && rawHitID.collectionID == assocRawID.collectionID) {
                    auto simHit = assoc.getSimHit();
                    for (const auto& contrib : simHit.getContributions()) {
                        const auto& relMcP = contrib.getParticle();
                        auto relMcPId = relMcP.getObjectID();
                        regisMcPIDs.push_back(relMcPId.index);
                        rawCollOut->push_back(rawHitFromAssoc.clone());
                    }
                }
            }

            // == e == For QA relation valuables QAQAQAQAQAQAQAQAQAQAQAQAQAQAQA
          }


        }
      }
      // == e == Registrer Calo Rec Hits =======================================================

      // == s == For QA relation valuables QAQAQAQAQAQAQAQAQAQAQAQAQAQAQA
      // == s == For MC Trigger Efficiency Estimation ~~~~~~~~
      Int_t physEventWeight = 2;
      for (auto it = m_vPhysCooTimes.begin(); it != m_vPhysCooTimes.end(); ++it) {
          const Double_t physCollTime = *it;
          if ((physCollTime + 20 > tsTime - 10) && (physCollTime - 10 < tsTime + 30)) {
            physEventWeight = 1;
            m_vPhysCooTimes.erase(it);
            break;
        }
      }

      edm4hep::MutableEventHeader event_header_ts;
      event_header_ts.setRunNumber(m_event_number_ts * 10000 + child_idx);
      event_header_ts.setEventNumber(m_event_number_ts);
      event_header_ts.setTimeStamp(iTimeSlice);
      event_header_ts.setWeight(physEventWeight);
      // event_header_ts.setWeight(memoryUsage); // Just a dummy weight for now
      m_event_header_ts_out()->push_back(event_header_ts);
      m_event_number_ts++;


      if(physEventWeight == 1){
        edm4hep::MutableEventHeader event_header_bkg;
        event_header_bkg.setRunNumber(m_event_number_ts * 10000 + child_idx);
        event_header_bkg.setEventNumber(m_event_number_ts);
        event_header_bkg.setTimeStamp(iTimeSlice);
        event_header_bkg.setWeight(1);
        m_event_header_bkg_out()->push_back(event_header_bkg);

        edm4hep::MutableEventHeader event_header_phy;
        event_header_phy.setRunNumber(m_event_number_ts * 10000 + child_idx);
        event_header_phy.setEventNumber(m_event_number_ts);
        event_header_phy.setTimeStamp(iTimeSlice);
        event_header_phy.setWeight(2);
        m_event_header_phy_out()->push_back(event_header_phy);
        for(size_t iTrig = 0; iTrig < 4; ++iTrig){
          if(bMutipliTriggers[iTrig]){
            edm4hep::MutableEventHeader event_header_phy;
            event_header_phy.setRunNumber(m_event_number_ts * 10000 + child_idx);
            event_header_phy.setEventNumber(m_event_number_ts);
            event_header_phy.setTimeStamp(iTimeSlice);
            event_header_phy.setWeight(iTrig+3);
            m_event_header_phy_out()->push_back(event_header_phy);
          }
        }
      }else if(physEventWeight == 2){
        edm4hep::MutableEventHeader event_header_phy;
        event_header_phy.setRunNumber(m_event_number_ts * 10000 + child_idx);
        event_header_phy.setEventNumber(m_event_number_ts);
        event_header_phy.setTimeStamp(iTimeSlice);
        event_header_phy.setWeight(1);
        m_event_header_phy_out()->push_back(event_header_phy);

        edm4hep::MutableEventHeader event_header_bkg;
        event_header_bkg.setRunNumber(m_event_number_ts * 10000 + child_idx);
        event_header_bkg.setEventNumber(m_event_number_ts);
        event_header_bkg.setTimeStamp(iTimeSlice);
        event_header_bkg.setWeight(2);
        m_event_header_bkg_out()->push_back(event_header_bkg);
        for(size_t iTrig = 0; iTrig < 4; ++iTrig){
          if(bMutipliTriggers[iTrig]){
            edm4hep::MutableEventHeader event_header_bkg;
            event_header_bkg.setRunNumber(m_event_number_ts * 10000 + child_idx);
            event_header_bkg.setEventNumber(m_event_number_ts);
            event_header_bkg.setTimeStamp(iTimeSlice);
            event_header_bkg.setWeight(iTrig+3);
            m_event_header_bkg_out()->push_back(event_header_bkg);
          }
        }
      }

      // Insert an independent EventHeader object into the physics event.
      // A subset header would keep a reference to the parent frame collection.
      edm4hep::MutableEventHeader event_header;
      if (m_event_header_in() != nullptr && !m_event_header_in()->empty()) {
        const auto& event_header_in = m_event_header_in()->at(0);
        event_header.setRunNumber(event_header_in.getRunNumber());
        event_header.setEventNumber(event_header_in.getEventNumber());
        event_header.setTimeStamp(event_header_in.getTimeStamp());
        event_header.setWeight(event_header_in.getWeight());
      } else {
        event_header.setRunNumber(child.GetRunNumber());
        event_header.setEventNumber(child.GetEventNumber());
        event_header.setTimeStamp(iTimeSlice);
        event_header.setWeight(physEventWeight);
      }
      m_event_header_out()->push_back(event_header);
      // == e == For MC Trigger Efficiency Estimation ~~~~~~~~
      
      // == s == Register in output of MC Particles =========
      for (const auto& mcparticle : *m_mcparticles_in()) {
        m_mcparticles_out()->push_back(mcparticle.clone(true));
      }
      // == e == Register in output of MC Particles =========
      // == s == For QA relation valuables QAQAQAQAQAQAQAQAQAQAQAQAQAQAQA
    }
    
    if (tsTimeE > m_timeframe_width) m_bScanedAllTimeWindows = true;
    if (m_bScanedAllTimeWindows) {
      bInitialLoop     = true;
      m_bOnceTriggered = false;

      m_vPhysCooTimes.clear();
      std::vector<Double_t>().swap(m_vPhysCooTimes);

      m_bScanedAllTimeWindows = false;
      iTimeSlice              = 0;
      targetDetId             = 0;
      for (auto& start_point : iniTrkHitPoint) {
        start_point = 0;
      }
      for (auto& start_point : iniCalHitPoint) {
        start_point = 0;
      }

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
