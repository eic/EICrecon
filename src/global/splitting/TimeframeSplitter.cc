// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#include "TimeframeSplitter.h"

#include <JANA/JEvent.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

TimeframeSplitter::TimeframeSplitter() {
  SetTypeName(NAME_OF_THIS);
  SetParentLevel(JEventLevel::Timeslice);
  SetChildLevel(JEventLevel::PhysicsEvent);
}

std::uint64_t TimeframeSplitter::objIdKey(const podio::ObjectID& objectID) {
  const auto collectionID =
      static_cast<std::uint64_t>(static_cast<std::uint32_t>(objectID.collectionID));
  const auto index = static_cast<std::uint64_t>(static_cast<std::uint32_t>(objectID.index));

  return (collectionID << 32U) | index;
}

TimeframeSplitter::TrackerAssociationIndex TimeframeSplitter::buildTrkAssoId(
    const edm4eic::MCRecoTrackerHitAssociationCollection* associations) {

  TrackerAssociationIndex index;
  if (associations == nullptr) {
    return index;
  }
  index.reserve(associations->size());

  for (size_t assoId = 0; assoId < associations->size(); ++assoId) {
    const auto association = associations->at(assoId);
    const auto rawHit      = association.getRawHit();
    if (!rawHit.isAvailable()) {
      continue;
    }
    index[objIdKey(rawHit.getObjectID())].push_back(assoId);
  }

  return index;
}

TimeframeSplitter::CalorimeterAssociationIndex TimeframeSplitter::buildCalAssoId(
    const edm4eic::MCRecoCalorimeterHitAssociationCollection* associations) {

  CalorimeterAssociationIndex index;
  if (associations == nullptr) {
    return index;
  }
  index.reserve(associations->size());

  for (size_t assoId = 0; assoId < associations->size(); ++assoId) {
    const auto association = associations->at(assoId);
    const auto rawHit      = association.getRawHit();
    if (!rawHit.isAvailable()) {
      continue;
    }
    index[objIdKey(rawHit.getObjectID())].push_back(assoId);
  }

  return index;
}

bool TimeframeSplitter::overlapsTimeWindow(double hitTime, double resolution, double window_start,
                                           double window_end) {
  return hitTime + resolution > window_start && hitTime - resolution < window_end;
}

bool TimeframeSplitter::judgeOverTimeWindow(double hitTime, double resolution, double window_end) {
  return hitTime - resolution >= window_end;
}

bool TimeframeSplitter::isValidEtaPhiBin(int etaBin, int phiBin) {
  return 0 <= etaBin && etaBin < kEtaPhiBins && 0 <= phiBin && phiBin < kEtaPhiBins;
}

bool TimeframeSplitter::judgeHitInTimeSlice(double hitTime, double timeResolution,
                                            double timeslice_start, double timeslice_end) {
  return !(hitTime + timeResolution < timeslice_start || hitTime - timeResolution > timeslice_end);
}

std::pair<int, int> TimeframeSplitter::etaPhiBins(double hitEta, double hitPhi, double etaMin,
                                                  double etaMax, int bShift) {
  const double etaBinWidth = (etaMax - etaMin) / kEtaPhiBins;
  const double phiMin      = -std::numbers::pi;
  const double phiMax      = std::numbers::pi;
  const double phiBinWidth = (phiMax - phiMin) / kEtaPhiBins;

  const double halfEtaBin    = 0.5 * etaBinWidth * bShift;
  const double halfPhiBin    = 0.5 * phiBinWidth * bShift;
  const double shiftedEtaMin = etaMin + halfEtaBin;
  const double shiftedEtaMax = etaMax + halfEtaBin;
  const double shiftedPhiMin = phiMin + halfPhiBin;
  const double shiftedPhiMax = phiMax + halfPhiBin;

  if (hitEta < shiftedEtaMin || hitEta >= shiftedEtaMax || hitPhi < shiftedPhiMin ||
      hitPhi >= shiftedPhiMax) {
    return {kInvalidEtaPhiBin, kInvalidEtaPhiBin};
  }

  const int etaBin = static_cast<int>(std::floor((hitEta - shiftedEtaMin) / etaBinWidth));
  const int phiBin = static_cast<int>(std::floor((hitPhi - shiftedPhiMin) / phiBinWidth));
  return {etaBin, phiBin};
}

size_t TimeframeSplitter::countGridCellsWithMultiplicity(const EtaPhiGrid& grid0,
                                                         const EtaPhiGrid& gridShifted,
                                                         const EtaPhiTimeGrid& gridTime0,
                                                         const EtaPhiTimeGrid& gridShiftedTime,
                                                         int threshold, double& averageTime) {
  size_t count   = 0;
  double timeSum = 0.0;
  for (size_t iEta = 0; iEta < kEtaPhiBins; ++iEta) {
    for (size_t iPhi = 0; iPhi < kEtaPhiBins; ++iPhi) {
      if (grid0[iEta][iPhi] >= threshold || gridShifted[iEta][iPhi] >= threshold) {
        count++;
        if (grid0[iEta][iPhi] >= threshold) {
          timeSum += gridTime0[iEta][iPhi] / grid0[iEta][iPhi];
        } else if (gridShifted[iEta][iPhi] >= threshold) {
          timeSum += gridShiftedTime[iEta][iPhi] / gridShifted[iEta][iPhi];
        }
      }
    }
  }
  averageTime = count > 0 ? timeSum / count : 0.0;
  return count;
}

double TimeframeSplitter::averageSelectedTriggerTime(const std::array<double, 8>& values,
                                                     const std::array<double, 8>& times,
                                                     std::initializer_list<size_t> indices,
                                                     double fallbackTime) {
  size_t count   = 0;
  double timeSum = 0.0;
  for (const size_t index : indices) {
    if (values[index] <= 0.0) {
      continue;
    }
    timeSum += times[index];
    count++;
  }
  return count > 0 ? timeSum / count : fallbackTime;
}

std::pair<int, int> TimeframeSplitter::backEndEtaPhiBins(double hitEta, double hitPhi, int bShift) {
  return etaPhiBins(hitEta, hitPhi, backwardEtaMin(), backwardEtaMax(), bShift);
}

std::pair<int, int> TimeframeSplitter::barrelEtaPhiBins(double hitEta, double hitPhi, int bShift) {
  return etaPhiBins(hitEta, hitPhi, barrelEtaMin(), barrelEtaMax(), bShift);
}

std::pair<int, int> TimeframeSplitter::forwardEndEtaPhiBins(double hitEta, double hitPhi,
                                                            int bShift) {
  return etaPhiBins(hitEta, hitPhi, forwardEtaMin(), forwardEtaMax(), bShift);
}

double TimeframeSplitter::trkTimeResolution(TrkCollectionIndex detectorID) {
  switch (detectorID) {
  case kTrkB0:
    [[fallthrough]];
  case kTrkTOFBarrel:
    [[fallthrough]];
  case kTrkTOFEndcap:
    return timeResolution_ACLGad();

  case kTrkMPGDBarrel:
    [[fallthrough]];
  case kTrkOuterMPGDBarrel:
    [[fallthrough]];
  case kTrkBackwardMPGD:
    [[fallthrough]];
  case kTrkForwardMPGD:
    return timeResolution_MPGD();

  case kTrkSiBarrelVertex:
    [[fallthrough]];
  case kTrkSiBarrel:
    [[fallthrough]];
  case kTrkSiEndcap:
    [[fallthrough]];
  case kTrkTagger:
    [[fallthrough]];
  case kTrkForwardRomanPot:
    [[fallthrough]];
  case kTrkForwardOffMTracker:
    return timeResolution_SiMaps();

  case kTrkCollectionSize:
    break;
  }

  throw std::runtime_error("Unknown tracker detector ID");
}

double TimeframeSplitter::calTimeResolution(CalCollectionIndex detectorID) {
  switch (detectorID) {
  case kCalB0ECal:
    [[fallthrough]];
  case kCalEcalBarrelImg:
    [[fallthrough]];
  case kCalEcalBarrelScFi:
    [[fallthrough]];
  case kCalEcalEndcapN:
    [[fallthrough]];
  case kCalEcalEndcapP:
    [[fallthrough]];
  case kCalEcalZDC:
    [[fallthrough]];
  case kCalEcalLumiSpec:
    return timeResolution_EMCal();

  case kCalHcalBarrel:
    [[fallthrough]];
  case kCalHcalEndcapN:
    [[fallthrough]];
  case kCalHcalEndcapPInsert:
    [[fallthrough]];
  case kCalHcalZDC:
    [[fallthrough]];
  case kCalLFHCAL:
    return timeResolution_HCal(); // TODO: use dedicated HCal resolution if needed

  case kCalCollectionSize:
    break;
  }

  throw std::runtime_error("Unknown calorimeter detector ID");
}

TimeframeSplitter::Result TimeframeSplitter::Unfold(const JEvent& parent, JEvent& child,
                                                    int child_idx) {
  const float timeframeWidth = timeframeWidth();
  const float timesplitWidth = timesplitWidth();

  bool bTrigger = false;

  // Materialize these output collections even when no timeslice triggers,
  // so the output schema is available from the first event.
  (void)eventHeader_outCol();
  (void)eventHeaderPhy_outCols();
  (void)eventHeaderBkg_outCols();

  const auto trackerHitCollsIn = trackerHits_inCols();
  const auto caloRecHitCollsIn = calorimeterHit_inCols();
  const auto trkAssoCollsIn    = trackerHitsAsso_inCols();
  const auto richRawHitCollsIn = richRawHits_inCols();
  const auto richAssoCollsIn   = richHitsAsso_inCols();
  const auto calrecAssoCollsIn = mcRecoCalorimeterHitAssociation_inCols();

  // == s == Register hits of TOF and MPGD detectors in the time slice ==================
  if (child_idx == 0) {
    // Association collections belong to the parent Timeslice and remain valid for all
    // of its children. Build each lookup once per parent instead of rebuilding the full
    // index for every triggered PhysicsEvent.
    trkAssoIds.clear();
    trkAssoIds.reserve(trkAssoCollsIn.size());
    for (const auto* associations : trkAssoCollsIn) {
      trkAssoIds.push_back(buildTrkAssoId(associations));
    }

    richAssoIds.clear();
    richAssoIds.reserve(richAssoCollsIn.size());
    for (const auto* associations : richAssoCollsIn) {
      richAssoIds.push_back(buildTrkAssoId(associations));
    }

    calAssoIds.clear();
    calAssoIds.reserve(calrecAssoCollsIn.size());
    for (const auto* associations : calrecAssoCollsIn) {
      calAssoIds.push_back(buildCalAssoId(associations));
    }

    // == s == For MC Trigger Efficiency Estimation ~~~~~~~~
    vPhysCollisionTimes.clear();

    double prevMCTime = -std::numeric_limits<double>::max(); // temp check mc particle times
    for (const auto& mcparticle : *mcParticles_inCol()) {
      if (mcparticle.getGeneratorStatus() != 1) {
        continue;
      }
      if (std::abs(prevMCTime - mcparticle.getTime()) < 50.) {
        continue;
      }
      double mcCollTime = mcparticle.getTime();
      vPhysCollisionTimes.push_back(mcCollTime);
      prevMCTime = mcCollTime;
    }
    std::sort(vPhysCollisionTimes.begin(), vPhysCollisionTimes.end());
    auto last = std::unique(vPhysCollisionTimes.begin(), vPhysCollisionTimes.end());
    vPhysCollisionTimes.erase(last, vPhysCollisionTimes.end());
    // == e == For MC Trigger Efficiency Estimation ~~~~~~~~
  }
  // == e == Register hits of TOF and MPGD detectors in the time slice ==================

  // == s == Time frame scan loop ==========================================================
  double timesliceT0     = std::numeric_limits<double>::quiet_NaN();
  bool bTimesliceTrigger = false;

  std::array<bool, kNumOfCombineTrig> bCombineTriggers{};
  std::array<double, kNumOfCombineTrig> combineTrigTime{};

  if (timesplitWidth <= 0.0F) {
    throw std::runtime_error("TimeframeSplitter: timesplitWidth must be greater than zero");
  }
  if (timeframeWidth <= 0.0F) {
    throw std::runtime_error("TimeframeSplitter: timeframeWidth must be greater than zero");
  }

  const size_t nTimeSlices = static_cast<size_t>(std::floor(timeframeWidth / timesplitWidth));
  double tsTimeS           = 0.0;
  double tsTimeE           = 0.0;

  // Scan the timeframe one time slice at a time.
  // The scan stops early when a physics trigger fires; otherwise it terminates
  // after all time slices in the timeframe have been processed.
  while (iTimeSlice < nTimeSlices) {
    tsTimeS = iTimeSlice * timesplitWidth;
    tsTimeE = (iTimeSlice + 1) * timesplitWidth;
    if (tsTimeE > timeframeWidth) {
      break;
    }
    iTimeSlice++;

    // == s == Multiplicity Single Triggers =======================================
    std::array<double, kNumOfSingleTrig> singleTrig{};
    std::array<double, kNumOfSingleTrig> singleTrigTime{};

    // Tracker-matching thresholds for each trigger region.
    const std::array<size_t, kNumSingleTrigRegion> trackerMatchThresholds = {
        backwardTrackerMatchThreshold(),
        barrelTrackerMatchThreshold(),
        forwardTrackerMatchThreshold(),
    };

    // ---------------------------------------------------------------------------
    // Backward, barrel, and forward ECal / ECal+tracker triggers
    // ---------------------------------------------------------------------------
    for (size_t iRegion = 0; iRegion < kNumSingleTrigRegion; ++iRegion) {

      const auto& config = triggerRegionConfigs.at(iRegion);

      // Select the eta-phi binning function corresponding to the trigger region.
      const auto binFunc = [this, iRegion](double eta, double phi, int shift) {
        switch (iRegion) {
        case kSingleTrigRegionBackward:
          return backEndEtaPhiBins(eta, phi, shift);

        case kSingleTrigRegionBarrel:
          return barrelEtaPhiBins(eta, phi, shift);

        case kSingleTrigRegionForward:
          return forwardEndEtaPhiBins(eta, phi, shift);

        default:
          throw std::runtime_error("Unknown single trigger region");
        }
      };

      // -------------------------------------------------------------------------
      // ECal trigger
      // -------------------------------------------------------------------------
      EtaPhiGrid calGrid{};
      EtaPhiGrid calGridShifted{};
      EtaPhiTimeGrid calTimeGrid{};
      EtaPhiTimeGrid calTimeGridShifted{};
      fillEtaPhiGrids(caloRecHitCollsIn.at(config.calDetector), iniCalHitPoint[config.calDetector],
                      calTimeResolution(config.calDetector), tsTimeS, tsTimeE, calGrid,
                      calGridShifted, calTimeGrid, calTimeGridShifted, binFunc);

      singleTrig[config.calTrigger] = countGridCellsWithMultiplicity(
          calGrid, calGridShifted, calTimeGrid, calTimeGridShifted, ecalMultiplicityThreshold(),
          singleTrigTime[config.calTrigger]);

      // -------------------------------------------------------------------------
      // ECal + tracker matching trigger
      // -------------------------------------------------------------------------
      EtaPhiGrid trkGrid{};
      EtaPhiGrid trkGridShifted{};
      EtaPhiTimeGrid trkTimeGrid{};
      EtaPhiTimeGrid trkTimeGridShifted{};
      for (const auto trkDetector : config.trkDetectors) {
        fillEtaPhiGridsMatched(trackerHitCollsIn.at(trkDetector), iniTrkHitPoint[trkDetector],
                               trkTimeResolution(trkDetector), tsTimeS, tsTimeE, calGrid,
                               calGridShifted, trkGrid, trkGridShifted,
                               trackerMatchThresholds.at(iRegion), trkTimeGrid, trkTimeGridShifted,
                               binFunc);
      }

      singleTrig[config.calTrkTrigger] = countGridCellsWithMultiplicity(
          trkGrid, trkGridShifted, trkTimeGrid, trkTimeGridShifted, trackerMultiplicityThreshold(),
          singleTrigTime[config.calTrkTrigger]);
    }

    // ---------------------------------------------------------------------------
    // B0 tracker trigger
    // ---------------------------------------------------------------------------
    const auto hitsB0 = countHitsInTimeWindow(trackerHitCollsIn.at(kTrkB0), iniTrkHitPoint[kTrkB0],
                                              trkTimeResolution(kTrkB0), tsTimeS, tsTimeE);
    iniTrkHitPoint[kTrkB0]           = hitsB0.nextStartID;
    singleTrig[kSingleTrigB0Trk]     = hitsB0.count;
    singleTrigTime[kSingleTrigB0Trk] = hitsB0.average_time();

    double totalZDCEnergy      = 0.0;
    double totalZDCEnergyTime  = 0.0;
    const auto* recHitsZDCECal = caloRecHitCollsIn.at(kCalEcalZDC);
    if (recHitsZDCECal != nullptr) {
      for (size_t iHit = iniCalHitPoint[kCalEcalZDC]; iHit < recHitsZDCECal->size(); ++iHit) {
        const auto& hit      = recHitsZDCECal->at(iHit);
        const double hitTime = timeOfFlightCorrectedTime(hit);
        if (hitTime - calTimeResolution(kCalEcalZDC) > tsTimeE) {
          break;
        }
        if (judgeHitInTimeSlice(hitTime, calTimeResolution(kCalEcalZDC), tsTimeS, tsTimeE)) {
          totalZDCEnergy += hit.getEnergy();
          totalZDCEnergyTime += hit.getEnergy() * hitTime;
          iniCalHitPoint[kCalEcalZDC] = iHit;
        }
      }
    }
    singleTrig[kSingleTrigZDCECal] = totalZDCEnergy;
    singleTrigTime[kSingleTrigZDCECal] =
        totalZDCEnergy > 0.0 ? totalZDCEnergyTime / totalZDCEnergy : 0.0;

    const double etaPhiCalTriggerSum    = singleTrig[kSingleTrigBackEndcapECal] +
                                          singleTrig[kSingleTrigCentBarrelECal] +
                                          singleTrig[kSingleTrigForwardEndcapECal];
    const double etaPhiCalTrkTriggerSum = singleTrig[kSingleTrigBackEndcapECalTrk] +
                                          singleTrig[kSingleTrigCentBarrelECalTrk] +
                                          singleTrig[kSingleTrigForwardEndcapECalTrk];
    bCombineTriggers[kCombTrigECalTrkAndB0Trk] =
        etaPhiCalTrkTriggerSum > 0 && singleTrig[kSingleTrigB0Trk] > 4;
    bCombineTriggers[kCombTrigECalTrkAndZDCEcal] =
        etaPhiCalTrkTriggerSum > 0 && singleTrig[kSingleTrigZDCECal] > 50;
    bCombineTriggers[kCombTrigECalAndB0Trk] =
        etaPhiCalTriggerSum > 0 && singleTrig[kSingleTrigB0Trk] > 4;
    bCombineTriggers[kCombTrigECalAndZDCEcal] =
        etaPhiCalTriggerSum > 0 && singleTrig[kSingleTrigZDCECal] > 0.005;
    bCombineTriggers[kCombTrigECalTrk] = etaPhiCalTrkTriggerSum > 1;
    bCombineTriggers[kCombTrigECal]    = etaPhiCalTriggerSum > 2;

    if (std::none_of(bCombineTriggers.begin(), bCombineTriggers.end(),
                     [](bool fired) { return fired; })) {
      continue;
    }

    const double fallbackTriggerTime = 0.5 * (tsTimeS + tsTimeE);
    if (bCombineTriggers[kCombTrigECalTrkAndB0Trk])
      combineTrigTime[kCombTrigECalTrkAndB0Trk] =
          averageSelectedTriggerTime(singleTrig, singleTrigTime,
                                     {kSingleTrigBackEndcapECalTrk, kSingleTrigCentBarrelECalTrk,
                                      kSingleTrigForwardEndcapECalTrk, kSingleTrigB0Trk},
                                     fallbackTriggerTime);
    if (bCombineTriggers[kCombTrigECalTrkAndZDCEcal])
      combineTrigTime[kCombTrigECalTrkAndZDCEcal] =
          averageSelectedTriggerTime(singleTrig, singleTrigTime,
                                     {kSingleTrigBackEndcapECalTrk, kSingleTrigCentBarrelECalTrk,
                                      kSingleTrigForwardEndcapECalTrk, kSingleTrigZDCECal},
                                     fallbackTriggerTime);
    if (bCombineTriggers[kCombTrigECalAndB0Trk])
      combineTrigTime[kCombTrigECalAndB0Trk] =
          averageSelectedTriggerTime(singleTrig, singleTrigTime,
                                     {kSingleTrigBackEndcapECal, kSingleTrigCentBarrelECal,
                                      kSingleTrigForwardEndcapECal, kSingleTrigB0Trk},
                                     fallbackTriggerTime);
    if (bCombineTriggers[kCombTrigECalAndZDCEcal]) {
      combineTrigTime[kCombTrigECalAndZDCEcal] =
          averageSelectedTriggerTime(singleTrig, singleTrigTime,
                                     {kSingleTrigBackEndcapECal, kSingleTrigCentBarrelECal,
                                      kSingleTrigForwardEndcapECal, kSingleTrigZDCECal},
                                     fallbackTriggerTime);
    }
    if (bCombineTriggers[kCombTrigECalTrk]) {
      combineTrigTime[kCombTrigECalTrk] =
          averageSelectedTriggerTime(singleTrig, singleTrigTime,
                                     {kSingleTrigBackEndcapECalTrk, kSingleTrigCentBarrelECalTrk,
                                      kSingleTrigForwardEndcapECalTrk},
                                     fallbackTriggerTime);
    }
    if (bCombineTriggers[kCombTrigECal]) {
      combineTrigTime[kCombTrigECal] = averageSelectedTriggerTime(
          singleTrig, singleTrigTime,
          {kSingleTrigBackEndcapECal, kSingleTrigCentBarrelECal, kSingleTrigForwardEndcapECal},
          fallbackTriggerTime);
    }
    double combineTrigCount = 0;
    double totalTrigTime    = 0.0;
    for (size_t iTrig = 0; iTrig < kNumOfCombineTrig; ++iTrig) {
      if (bCombineTriggers[iTrig]) {
        totalTrigTime += combineTrigTime[iTrig];
        ++combineTrigCount;
      }
    }
    timesliceT0 = combineTrigCount > 0 ? totalTrigTime / static_cast<double>(combineTrigCount)
                                       : fallbackTriggerTime;

    // == s == Multiplisity Single Triggers =======================================

    bTimesliceTrigger = std::any_of(bCombineTriggers.begin(), bCombineTriggers.end(),
                                    [](bool fired) { return fired; });
    if (bTimesliceTrigger) {
      break;
    }
  }
  // == e == Time frame scan loop ==========================================================

  bTrigger = bTimesliceTrigger;
  if (bTimesliceTrigger) {
    bOnceTriggered = true;
    // For now, a one-to-one relationship between timeslices and events
    child.SetEventNumber(NewEventCount);
    child.SetRunNumber(parent.GetRunNumber());
    NewEventCount++;
    // Clone truth particles before detector relations so every child SimTrackerHit can
    // point to an MCParticle owned by this PhysicsEvent rather than by the parent Timeslice.
    std::vector<edm4hep::MutableMCParticle> copiedMCParticles;
    copiedMCParticles.reserve(mcParticles_inCol()->size());
    std::unordered_map<std::uint64_t, edm4hep::MCParticle> copiedMCParticleMap;
    copiedMCParticleMap.reserve(mcParticles_inCol()->size());
    for (const auto& mcparticle : *mcParticles_inCol()) {
      auto copiedMCParticle = mcparticle.clone(false);
      mcParticles_outCol()->push_back(copiedMCParticle);
      copiedMCParticles.push_back(copiedMCParticle);
      copiedMCParticleMap.emplace(objIdKey(mcparticle.getObjectID()),
                                  mcParticles_outCol()->at(mcParticles_outCol()->size() - 1));
    }

    // Recreate the MCParticle graph only after all child particles exist. Keeping this graph is
    // required by calorimeter cluster truth association, which walks from each contribution's
    // particle to its primary ancestor.
    for (size_t particleIndex = 0; particleIndex < mcParticles_inCol()->size(); ++particleIndex) {
      const auto parentParticle = mcParticles_inCol()->at(particleIndex);
      auto& copiedParticle      = copiedMCParticles.at(particleIndex);

      for (const auto& parentRelation : parentParticle.getParents()) {
        const auto copiedParent = copiedMCParticleMap.find(objIdKey(parentRelation.getObjectID()));
        if (copiedParent == copiedMCParticleMap.end()) {
          throw std::runtime_error("MCParticle parent relation cannot be remapped to child event");
        }
        copiedParticle.addToParents(copiedParent->second);
      }

      for (const auto& daughterRelation : parentParticle.getDaughters()) {
        const auto copiedDaughter =
            copiedMCParticleMap.find(objIdKey(daughterRelation.getObjectID()));
        if (copiedDaughter == copiedMCParticleMap.end()) {
          throw std::runtime_error(
              "MCParticle daughter relation cannot be remapped to child event");
        }
        copiedParticle.addToDaughters(copiedDaughter->second);
      }
    }

    // == s == Register Tracker Hits =======================================================
    for (size_t trkDetID = 0; trkDetID < trackerHitCollsIn.size(); ++trkDetID) {
      const auto* trkCollIn = trackerHitCollsIn.at(trkDetID);

      if (trkCollIn == nullptr) {
        continue;
      }
      auto& trkCollOut          = trackerHits_outCols().at(trkDetID);
      const auto tempDetID      = static_cast<TrkCollectionIndex>(trkDetID);
      const double detTimeReso  = trkTimeResolution(tempDetID);
      const auto* trkAssoCollIn = trkAssoCollsIn.at(trkDetID);
      auto& rawCollOut          = rawTrackerHit_outCols().at(trkDetID);
      auto& trkAssoCollOut      = trackerHitsAsso_outCols().at(trkDetID);

      for (size_t iHit = 0; iHit < trkCollIn->size(); ++iHit) {
        const auto& trkHit = trkCollIn->at(iHit);
        const double hitT  = timeOfFlightCorrectedTime(trkHit);
        if (!overlapsTimeWindow(hitT, detTimeReso, timesliceT0 - trigTimeWindowBef(),
                                timesliceT0 + trigTimeWindowAft())) {
          continue;
        }

        iniTrkHitPoint[trkDetID] = iHit;
        copyTrkHitWithRelations(trkHit, trkAssoCollIn, trkAssoIds.at(trkDetID), trkCollOut,
                                rawCollOut, trkAssoCollOut, simTrackerHits_outCols().at(trkDetID),
                                recoTrackerHitLinks_outCols().at(trkDetID), mcParticles_outCol());
      }
    }
    // == e == Register Tracker Hits =======================================================

    // == s == Register RICH Hits ==========================================================
    // RICH hits are currently not time-filtered. Copy all RICH raw hits from the parent
    // Timeslice into every triggered PhysicsEvent together with their truth relations.
    for (size_t richDetID = 0; richDetID < richRawHitCollsIn.size(); ++richDetID) {
      const auto* rawCollIn = richRawHitCollsIn.at(richDetID);
      if (rawCollIn == nullptr) {
        continue;
      }

      const auto* assoCollIn = richAssoCollsIn.at(richDetID);

      auto& rawCollOut  = richRawHits_outCols().at(richDetID);
      auto& assoCollOut = richHitsAsso_outCols().at(richDetID);
      auto& linkCollOut = richHitLinks_outCols().at(richDetID);
      auto& simCollOut  = richSimHits_outCols().at(richDetID);

      // Multiple raw hits may refer to the same SimTrackerHit.
      // Copy each simulated hit only once within this child event.
      std::unordered_map<std::uint64_t, edm4hep::SimTrackerHit> copiedSimHits;

      for (size_t iRawHit = 0; iRawHit < rawCollIn->size(); ++iRawHit) {
        const auto& rawHit = rawCollIn->at(iRawHit);

        auto copiedRawHit = rawHit.clone();
        rawCollOut->push_back(copiedRawHit);

        if (assoCollIn == nullptr) {
          continue;
        }

        const auto assocIter = richAssoIds.at(richDetID).find(objIdKey(rawHit.getObjectID()));

        if (assocIter == richAssoIds.at(richDetID).end()) {
          continue;
        }

        for (const size_t associationPosition : assocIter->second) {
          const auto association = assoCollIn->at(associationPosition);

          if (!association.getSimHit().isAvailable()) {
            continue;
          }

          const auto simHit    = association.getSimHit();
          const auto simHitKey = objIdKey(simHit.getObjectID());

          auto copiedSimHitIter = copiedSimHits.find(simHitKey);

          if (copiedSimHitIter == copiedSimHits.end()) {
            auto copiedSimHit = simHit.clone(false);

            // Remap SimTrackerHit -> MCParticle to the particle owned by this child event.
            if (simHit.getParticle().isAvailable()) {
              const auto copiedParticle =
                  copiedMCParticleMap.find(objIdKey(simHit.getParticle().getObjectID()));

              if (copiedParticle == copiedMCParticleMap.end()) {
                throw std::runtime_error(
                    "RICH SimTrackerHit particle relation cannot be remapped to child event");
              }

              copiedSimHit.setParticle(copiedParticle->second);
            }

            simCollOut->push_back(copiedSimHit);

            copiedSimHitIter =
                copiedSimHits.emplace(simHitKey, simCollOut->at(simCollOut->size() - 1)).first;
          }

          const auto copiedSimHit = copiedSimHitIter->second;

          auto copiedAssociation = assoCollOut->create();
          copiedAssociation.setWeight(association.getWeight());
          copiedAssociation.setRawHit(copiedRawHit);
          copiedAssociation.setSimHit(copiedSimHit);

          auto copiedLink = linkCollOut->create();
          copiedLink.setWeight(association.getWeight());
          copiedLink.setFrom(copiedRawHit);
          copiedLink.setTo(copiedSimHit);
        }
      }
    }
    // == e == Register RICH Hits ==========================================================

    // == s == Register Calo Rec Hits =======================================================
    for (size_t calDetID = 0; calDetID < caloRecHitCollsIn.size(); ++calDetID) {
      const auto* caloInColl = caloRecHitCollsIn.at(calDetID);
      if (caloInColl == nullptr) {
        continue;
      }
      auto& caloOutColl = calorimeterHit_outCols().at(calDetID);

      const auto* caloInCollAsso = calrecAssoCollsIn.at(calDetID);
      if (caloInCollAsso == nullptr) {
        continue;
      }

      // Several raw hits can refer to the same simulated hit. Copy each simulated hit and its
      // contribution graph once per child event and detector, then reuse the child-owned proxy
      // for all raw-hit links and associations.
      std::unordered_map<std::uint64_t, edm4hep::SimCalorimeterHit> copiedSimHits;
      std::unordered_map<std::uint64_t, edm4hep::CaloHitContribution> copiedContributions;

      for (size_t iCalHit = 0; iCalHit < caloInColl->size(); ++iCalHit) {
        const auto& caloHit = caloInColl->at(iCalHit);

        double detTimeReso = calTimeResolution(kCalEcalEndcapN); // ??? check ECal Time resolution
        const double hitT  = timeOfFlightCorrectedTime(caloHit);

        if (hitT - detTimeReso > timesliceT0 + trigTimeWindowAft()) {
          continue;
        }
        if (overlapsTimeWindow(hitT, detTimeReso, timesliceT0 - trigTimeWindowBef(),
                               timesliceT0 + trigTimeWindowAft())) {
          auto copiedCaloHit = caloHit.clone();
          copiedCaloHit.setRawHit(edm4hep::RawCalorimeterHit());

          const auto rawHitFromRec = caloHit.getRawHit();
          if (rawHitFromRec.isAvailable()) {
            auto& rawCollOut  = rawCalorimeterHit_outCols().at(calDetID);
            auto copiedRawHit = rawHitFromRec.clone();
            rawCollOut->push_back(copiedRawHit);
            copiedCaloHit.setRawHit(copiedRawHit);

            auto& assocCollOut  = mcRecoCalorimeterHitAssociation_outCols().at(calDetID);
            auto& linkCollOut   = mcRecoCalorimeterHitLink_outCols().at(calDetID);
            auto& simCollOut    = simCalorimeterHit_outCols().at(calDetID);
            const auto rawHitID = rawHitFromRec.getObjectID();

            const auto& association_index = calAssoIds.at(calDetID);
            const auto assocIterCal       = association_index.find(objIdKey(rawHitID));

            if (assocIterCal != association_index.end()) {
              for (const size_t association_position : assocIterCal->second) {
                const auto assoc = caloInCollAsso->at(association_position);

                if (!assoc.getSimHit().isAvailable()) {
                  continue;
                }

                const auto simHit    = assoc.getSimHit();
                const auto simHitKey = objIdKey(simHit.getObjectID());

                auto copiedSimHitIter = copiedSimHits.find(simHitKey);
                if (copiedSimHitIter == copiedSimHits.end()) {
                  auto copiedSimHit         = simHit.clone(false);
                  auto& contributionCollOut = caloHitContribution_outCols().at(calDetID);

                  for (const auto& contribution : simHit.getContributions()) {
                    const auto contributionKey  = objIdKey(contribution.getObjectID());
                    auto copiedContributionIter = copiedContributions.find(contributionKey);

                    if (copiedContributionIter == copiedContributions.end()) {
                      auto copiedContribution = contribution.clone(false);
                      copiedContribution.setParticle(edm4hep::MCParticle());

                      const auto particle = contribution.getParticle();
                      if (particle.isAvailable()) {
                        const auto copiedParticle =
                            copiedMCParticleMap.find(objIdKey(particle.getObjectID()));
                        if (copiedParticle == copiedMCParticleMap.end()) {
                          throw std::runtime_error(
                              "CaloHitContribution particle relation cannot be remapped to child "
                              "event");
                        }
                        copiedContribution.setParticle(copiedParticle->second);
                      }

                      contributionCollOut->push_back(copiedContribution);
                      copiedContributionIter =
                          copiedContributions
                              .emplace(contributionKey,
                                       contributionCollOut->at(contributionCollOut->size() - 1))
                              .first;
                    }

                    copiedSimHit.addToContributions(copiedContributionIter->second);
                  }

                  simCollOut->push_back(copiedSimHit);
                  copiedSimHitIter =
                      copiedSimHits.emplace(simHitKey, simCollOut->at(simCollOut->size() - 1))
                          .first;
                }

                const auto copiedSimHit = copiedSimHitIter->second;

                auto copiedAssoc = assocCollOut->create();
                copiedAssoc.setWeight(assoc.getWeight());
                copiedAssoc.setRawHit(copiedRawHit);
                copiedAssoc.setSimHit(copiedSimHit);

                auto copiedLink = linkCollOut->create();
                copiedLink.setWeight(assoc.getWeight());
                copiedLink.setFrom(copiedRawHit);
                copiedLink.setTo(copiedSimHit);
              }
            }
          }

          caloOutColl->push_back(copiedCaloHit);
          iniCalHitPoint[calDetID] = iCalHit;
        }
      }
    }
    // == e == Register Calo Rec Hits =======================================================

    // == s == For QA relation valuables QA<><><><><><><><><><><><><><><><><><>>
    // == s == For MC Trigger Efficiency Estimation ~~~~~~~~
    unsigned int physEventWeight = 2;
    for (auto it = vPhysCollisionTimes.begin(); it != vPhysCollisionTimes.end(); ++it) {
      const double physCollTime = *it;
      if ((physCollTime + collisionTimeMarginAft() > timesliceT0 - trigTimeWindowBef()) &&
          (physCollTime - collisionTimeMarginBef() < timesliceT0 + trigTimeWindowAft())) {
        physEventWeight = 1;
        vPhysCollisionTimes.erase(it);
        break;
      }
    }

    if (physEventWeight == 1) {
      edm4hep::MutableEventHeader eventHeader_phy;
      eventHeader_phy.setRunNumber(child_idx);
      eventHeader_phy.setEventNumber(eventNumber_TS);
      eventHeader_phy.setTimeStamp(iTimeSlice);
      eventHeader_phy.setWeight(2);
      eventHeaderPhy_outCols()->push_back(eventHeader_phy);

      edm4hep::MutableEventHeader eventHeader_bkg;
      eventHeader_bkg.setRunNumber(child_idx);
      eventHeader_bkg.setEventNumber(eventNumber_TS);
      eventHeader_bkg.setTimeStamp(iTimeSlice);
      eventHeader_bkg.setWeight(1);
      eventHeaderBkg_outCols()->push_back(eventHeader_bkg);

      for (size_t iTrig = 0; iTrig < kNumOfCombineTrig; ++iTrig) {
        if (bCombineTriggers[iTrig]) {
          edm4hep::MutableEventHeader eventHeader_phy;
          eventHeader_phy.setRunNumber(child_idx);
          eventHeader_phy.setEventNumber(eventNumber_TS);
          eventHeader_phy.setTimeStamp(iTimeSlice);
          eventHeader_phy.setWeight(iTrig + 3);
          eventHeaderPhy_outCols()->push_back(eventHeader_phy);
        }
      }

      PhysCount++;
    } else if (physEventWeight == 2) {
      edm4hep::MutableEventHeader eventHeader_phy;
      eventHeader_phy.setRunNumber(child_idx);
      eventHeader_phy.setEventNumber(eventNumber_TS);
      eventHeader_phy.setTimeStamp(iTimeSlice);
      eventHeader_phy.setWeight(1);
      eventHeaderPhy_outCols()->push_back(eventHeader_phy);

      edm4hep::MutableEventHeader eventHeader_bkg;
      eventHeader_bkg.setRunNumber(child_idx);
      eventHeader_bkg.setEventNumber(eventNumber_TS);
      eventHeader_bkg.setTimeStamp(iTimeSlice);
      eventHeader_bkg.setWeight(2);
      eventHeaderBkg_outCols()->push_back(eventHeader_bkg);
      for (size_t iTrig = 0; iTrig < kNumOfCombineTrig; ++iTrig) {
        if (bCombineTriggers[iTrig]) {
          edm4hep::MutableEventHeader eventHeader_bkg;
          eventHeader_bkg.setRunNumber(child_idx);
          eventHeader_bkg.setEventNumber(eventNumber_TS);
          eventHeader_bkg.setTimeStamp(iTimeSlice);
          eventHeader_bkg.setWeight(iTrig + 3);
          eventHeaderBkg_outCols()->push_back(eventHeader_bkg);
        }
      }
    }
    eventNumber_TS++;

    // Insert an independent EventHeader object into the physics event.
    // A subset header would keep a reference to the parent frame collection.
    edm4hep::MutableEventHeader eventHeader;
    if (eventHeader_inCol() != nullptr && !eventHeader_inCol()->empty()) {
      const auto& eventHeader_in = eventHeader_inCol()->at(0);
      eventHeader.setRunNumber(eventHeader_in.getRunNumber());
      eventHeader.setEventNumber(eventHeader_in.getEventNumber());
      eventHeader.setTimeStamp(eventHeader_in.getTimeStamp());
      eventHeader.setWeight(eventHeader_in.getWeight());
    } else {
      eventHeader.setRunNumber(child.GetRunNumber());
      eventHeader.setEventNumber(child.GetEventNumber());
      eventHeader.setTimeStamp(iTimeSlice);
      eventHeader.setWeight(physEventWeight);
    }
    eventHeader_outCol()->push_back(eventHeader);
    // == e == For MC Trigger Efficiency Estimation ~~~~~~~~

    // == s == For QA relation valuables QA<><><><><><><><><><><><><><><><><>>
  }

  if (iTimeSlice >= nTimeSlices) {
    bScanedAllTimeWindows = true;
  }
  if (bScanedAllTimeWindows) {
    bInitialLoop   = true;
    bOnceTriggered = false;

    vPhysCollisionTimes.clear();

    bScanedAllTimeWindows = false;
    iTimeSlice            = 0;
    targetDetId           = 0;
    for (auto& start_point : iniTrkHitPoint) {
      start_point = 0;
    }
    for (auto& start_point : iniCalHitPoint) {
      start_point = 0;
    }

    if (bTrigger) {
      return Result::NextChildNextParent;
    } else {
      return Result::KeepChildNextParent;
    }

  } else if (bTrigger) {
    child_idx++;
    return Result::NextChildKeepParent;
  }
  return Result::KeepChildNextParent;
}
