// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#pragma once

#include <JANA/Components/JComponent.h>
#include <JANA/Components/JPodioOutput.h>
#include <JANA/JEventUnfolder.h>
#include <RtypesCore.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociation.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <podio/ObjectID.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <stddef.h>
#include <algorithm>
#include <array>
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
#include <optional>
#include <tuple>

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
  bool m_use_timeframe = false; // Use timeframes to split events, or use timeslices

  unsigned int m_OrigTFCount   = 0; //QA
  unsigned int m_NewEventCount = 0; //QA
  unsigned int m_PhysCount     = 0; //QA

  size_t m_eventNumber_TS = 0;              // Event number for the current timeslice
  std::vector<unsigned int> m_vTargetEvent; // List of original event numbers for each timeslice

  static constexpr double kPi = 3.14159265358979323846;

  static constexpr int kEtaPhiBins       = 10;
  static constexpr int kInvalidEtaPhiBin = -1;

  using EtaPhiGrid       = std::array<std::array<int, kEtaPhiBins>, kEtaPhiBins>;
  using EtaPhiTimeGrid   = std::array<std::array<double, kEtaPhiBins>, kEtaPhiBins>;
  using EtaPhiEnergyGrid = std::array<std::array<double, kEtaPhiBins>, kEtaPhiBins>;

  enum TrkCollectionType : size_t {
    kTrackerHitAligned = 0,
    kTrackerHit,
    kTrackerHitAssociation,
    kTrackerHitLink,
    kSimTrackerHit,
    kRawTrackerHit,
    kTrkCollectionTypeSize
  };

  enum CalCollectionType : size_t {
    kCalorimeterHitAligned = 0,
    kCalorimeterHit,
    kCalorimeterHitAssociation,
    kCalorimeterHitLink,
    kSimCalorimeterHit,
    kRawCalorimeterHit,
    kCalCollectionTypeSize
  };

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

  using trkCollNames                          = std::array<std::string, kTrkCollectionTypeSize>;
  std::array<trkCollNames, 13> m_trkCollNames = {{
      {
          "B0TrackerRecHits_aligned",
          "B0TrackerRecHits",
          "B0TrackerRawHitAssociations",
          "B0TrackerRawHitLinks",
          "B0TrackerHits",
          "B0TrackerRawHits",
      },
      {
          "TOFBarrelRecHits_aligned",
          "TOFBarrelRecHits",
          "TOFBarrelRawHitAssociations",
          "TOFBarrelRawHitLinks",
          "TOFBarrelHits",
          "TOFBarrelRawHits",
      },
      {
          "TOFEndcapRecHits_aligned",
          "TOFEndcapRecHits",
          "TOFEndcapRawHitAssociations",
          "TOFEndcapRawHitLinks",
          "TOFEndcapHits",
          "TOFEndcapRawHits",
      },
      {
          "MPGDBarrelRecHits_aligned",
          "MPGDBarrelRecHits",
          "MPGDBarrelRawHitAssociations",
          "MPGDBarrelRawHitLinks",
          "MPGDBarrelHits",
          "MPGDBarrelRawHits",
      },
      {
          "OuterMPGDBarrelRecHits_aligned",
          "OuterMPGDBarrelRecHits",
          "OuterMPGDBarrelRawHitAssociations",
          "OuterMPGDBarrelRawHitLinks",
          "OuterMPGDBarrelHits",
          "OuterMPGDBarrelRawHits",
      },
      {
          "BackwardMPGDEndcapRecHits_aligned",
          "BackwardMPGDEndcapRecHits",
          "BackwardMPGDEndcapRawHitAssociations",
          "BackwardMPGDEndcapRawHitLinks",
          "BackwardMPGDEndcapHits",
          "BackwardMPGDEndcapRawHits",
      },
      {
          "ForwardMPGDEndcapRecHits_aligned",
          "ForwardMPGDEndcapRecHits",
          "ForwardMPGDEndcapRawHitAssociations",
          "ForwardMPGDEndcapRawHitLinks",
          "ForwardMPGDEndcapHits",
          "ForwardMPGDEndcapRawHits",
      },
      {
          "SiBarrelVertexRecHits_aligned",
          "SiBarrelVertexRecHits",
          "SiBarrelVertexRawHitAssociations",
          "SiBarrelVertexRawHitLinks",
          "VertexBarrelHits",
          "SiBarrelVertexRawHits",
      },
      {
          "SiBarrelTrackerRecHits_aligned",
          "SiBarrelTrackerRecHits",
          "SiBarrelRawHitAssociations",
          "SiBarrelRawHitLinks",
          "SiBarrelHits",
          "SiBarrelRawHits",
      },
      {
          "SiEndcapTrackerRecHits_aligned",
          "SiEndcapTrackerRecHits",
          "SiEndcapTrackerRawHitAssociations",
          "SiEndcapTrackerRawHitLinks",
          "TrackerEndcapHits",
          "SiEndcapTrackerRawHits",
      },
      {
          "TaggerTrackerRecHits_aligned",
          "TaggerTrackerRecHits",
          "TaggerTrackerRawHitAssociations",
          "TaggerTrackerRawHitLinks",
          "TaggerTrackerHits",
          "TaggerTrackerRawHits",
      },
      {
          "ForwardRomanPotRecHits_aligned",
          "ForwardRomanPotRecHits",
          "ForwardRomanPotRawHitAssociations",
          "ForwardRomanPotRawHitLinks",
          "ForwardRomanPotHits",
          "ForwardRomanPotRawHits",
      },
      {
          "ForwardOffMTrackerRecHits_aligned",
          "ForwardOffMTrackerRecHits",
          "ForwardOffMTrackerRawHitAssociations",
          "ForwardOffMTrackerRawHitLinks",
          "ForwardOffMTrackerHits",
          "ForwardOffMTrackerRawHits",
      },
  }};
  // "RICHEndcapNRecHits_aligned"
  // "DIRCBarRecHits_aligned",
  // "DRICHRecHits_aligned",

  using calCollNames                          = std::array<std::string, kCalCollectionTypeSize>;
  std::array<calCollNames, 12> m_calCollNames = {{
      {
          "B0ECalRecHits_aligned",
          "B0ECalRecHits",
          "B0ECalRawHitAssociations",
          "B0ECalRawHitLinks",
          "B0ECalHits",
          "B0ECalRawHits",
      },
      {
          "EcalBarrelImagingRecHits_aligned",
          "EcalBarrelImagingRecHits",
          "EcalBarrelImagingRawHitAssociations",
          "EcalBarrelImagingRawHitLinks",
          "EcalBarrelImagingHits",
          "EcalBarrelImagingRawHits",
      },
      {
          "EcalBarrelScFiRecHits_aligned",
          "EcalBarrelScFiRecHits",
          "EcalBarrelScFiRawHitAssociations",
          "EcalBarrelScFiRawHitLinks",
          "EcalBarrelScFiHits",
          "EcalBarrelScFiRawHits",
      },
      {
          "EcalEndcapNRecHits_aligned",
          "EcalEndcapNRecHits",
          "EcalEndcapNRawHitAssociations",
          "EcalEndcapNRawHitLinks",
          "EcalEndcapNHits",
          "EcalEndcapNRawHits",
      },
      {
          "EcalEndcapPRecHits_aligned",
          "EcalEndcapPRecHits",
          "EcalEndcapPRawHitAssociations",
          "EcalEndcapPRawHitLinks",
          "EcalEndcapPHits",
          "EcalEndcapPRawHits",
      },
      {
          "EcalFarForwardZDCRecHits_aligned",
          "EcalFarForwardZDCRecHits",
          "EcalFarForwardZDCRawHitAssociations",
          "EcalFarForwardZDCRawHitLinks",
          "EcalFarForwardZDCHits",
          "EcalFarForwardZDCRawHits",
      },
      {
          "EcalLumiSpecRecHits_aligned",
          "EcalLumiSpecRecHits",
          "EcalLumiSpecRawHitAssociations",
          "EcalLumiSpecRawHitLinks",
          "EcalLumiSpecHits",
          "EcalLumiSpecRawHits",
      },
      {
          "HcalBarrelRecHits_aligned",
          "HcalBarrelRecHits",
          "HcalBarrelRawHitAssociations",
          "HcalBarrelRawHitLinks",
          "HcalBarrelHits",
          "HcalBarrelRawHits",
      },
      {
          "HcalEndcapNRecHits_aligned",
          "HcalEndcapNRecHits",
          "HcalEndcapNRawHitAssociations",
          "HcalEndcapNRawHitLinks",
          "HcalEndcapNHits",
          "HcalEndcapNRawHits",
      },
      {
          "HcalEndcapPInsertRecHits_aligned",
          "HcalEndcapPInsertRecHits",
          "HcalEndcapPInsertRawHitAssociations",
          "HcalEndcapPInsertRawHitLinks",
          "HcalEndcapPInsertHits",
          "HcalEndcapPInsertRawHits",
      },
      {
          "HcalFarForwardZDCRecHits_aligned",
          "HcalFarForwardZDCRecHits",
          "HcalFarForwardZDCRawHitAssociations",
          "HcalFarForwardZDCRawHitLinks",
          "HcalFarForwardZDCHits",
          "HcalFarForwardZDCRawHits",
      },
      {
          "LFHCALRecHits_aligned",
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

  std::vector<std::string> getCalCollectionNames(CalCollectionType type) const {
    std::vector<std::string> names;
    names.reserve(m_calCollNames.size());
    for (const auto& collections : m_calCollNames) {
      names.push_back(collections[type]);
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
      this, {.names = getTrkCollectionNames(kTrackerHitAligned), .is_optional = true}};
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

  // calorimeter collections
  VariadicPodioInput<edm4hep::RawCalorimeterHit> m_rawCalorimeterHit_inCols{
      this, {.names = getCalCollectionNames(kRawCalorimeterHit), .is_optional = true}};
  VariadicPodioOutput<edm4hep::RawCalorimeterHit> m_rawCalorimeterHit_outCols{
      this, getCalCollectionNames(kRawCalorimeterHit)};

  VariadicPodioOutput<edm4eic::MCRecoCalorimeterHitLink> m_mcRecoCalorimeterHitLink_outCols{
      this, getCalCollectionNames(kCalorimeterHitLink)};
  VariadicPodioOutput<edm4hep::SimCalorimeterHit> m_simCalorimeterHit_outCols{
      this, getCalCollectionNames(kSimCalorimeterHit)};

  VariadicPodioInput<edm4eic::CalorimeterHit> m_calorimeterHit_inCols{
      this, {.names = getCalCollectionNames(kCalorimeterHitAligned), .is_optional = true}};
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

  unsigned int m_multiTriggerThreshold[4] = {1, 4, 20, 20};
  size_t iniTrkHitPoint[15]               = {0}; // B0Trk,
  size_t iniCalHitPoint[15]               = {0}; // B0Trk,
  bool m_bDetLastHits[10] = {false, false, false, false, false, false, false, false, false, false};

  bool m_bOnceTriggered        = false;
  bool m_bScanedAllTimeWindows = false;

  unsigned int targetDetId            = 0;
  size_t iTimeSlice                   = 0;
  std::vector<double> m_vPhysCooTimes = {};
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
      const double hitT = hit.getTime();
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
                              EtaPhiGrid& compGrid, EtaPhiGrid& compGridShifted,
                              unsigned int baseThreshold, EtaPhiTimeGrid& compGridTime,
                              EtaPhiTimeGrid& compGridShiftedTime, BinFunc binFunc) {
    if (collection == nullptr)
      return;

    const size_t hitCount = collection->size();
    for (size_t iHit = iniHitID; iHit < hitCount; ++iHit) {
      const double hitT = collection->at(iHit).getTime();
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
                                               const EtaPhiTimeGrid& gridShiftedTime,
                                               unsigned int threshold, double& averageTime);

  static double averageSelectedTriggerTime(const std::array<double, 8>& values,
                                           const std::array<double, 8>& times,
                                           std::initializer_list<size_t> indices,
                                           double fallbackTime);

  double trkTimeResolution(size_t detectorID);

  template <typename CollectionT>
  TimeWindowSummary countHitsInTimeWindow(const CollectionT* collection, size_t startHitID,
                                          double resolution, double window_start,
                                          double window_end) const {
    TimeWindowSummary summary;
    summary.nextStartID = startHitID;
    if (collection == nullptr)
      return summary;

    for (size_t i = startHitID; i < collection->size(); ++i) {
      const auto& hit      = collection->at(i);
      const double hitTime = hit.getTime();
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

  static std::pair<int, int> backEndEtaPhiBins(double hitEta, double hitPhi, int bShift);

  static std::pair<int, int> barrelEtaPhiBins(double hitEta, double hitPhi, int bShift);

  static std::pair<int, int> forwardEndEtaPhiBins(double hitEta, double hitPhi, int bShift);

  Result Unfold(const JEvent& parent, JEvent& child, int child_idx) override;
};
