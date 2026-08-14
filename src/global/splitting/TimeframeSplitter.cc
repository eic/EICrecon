// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#include "TimeframeSplitter.h"

#include <JANA/JEvent.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4hep/Vector3f.h>

#include <cmath>
#include <stdexcept>
#include <numbers>

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
  if (associations == nullptr)
    return index;
  index.reserve(associations->size());

  for (size_t assoId = 0; assoId < associations->size(); ++assoId) {
    const auto association = associations->at(assoId);
    const auto rawHit      = association.getRawHit();
    if (!rawHit.isAvailable())
      continue;
    index[objIdKey(rawHit.getObjectID())].push_back(assoId);
  }

  return index;
}

TimeframeSplitter::CalorimeterAssociationIndex TimeframeSplitter::buildCalAssoId(
    const edm4eic::MCRecoCalorimeterHitAssociationCollection* associations) {

  CalorimeterAssociationIndex index;
  if (associations == nullptr)
    return index;
  index.reserve(associations->size());

  for (size_t assoId = 0; assoId < associations->size(); ++assoId) {
    const auto association = associations->at(assoId);
    const auto rawHit      = association.getRawHit();
    if (!rawHit.isAvailable())
      continue;
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

size_t TimeframeSplitter::countGridCellsWithMultiplicity(
    const EtaPhiGrid& grid0, const EtaPhiGrid& gridShifted, const EtaPhiTimeGrid& gridTime0,
    const EtaPhiTimeGrid& gridShiftedTime, unsigned int threshold, double& averageTime) {
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
    if (values[index] <= 0.0)
      continue;
    timeSum += times[index];
    count++;
  }
  return count > 0 ? timeSum / count : fallbackTime;
}

double TimeframeSplitter::trkTimeResolution(size_t detectorID) {
  if (detectorID < 3)
    return timeResolution_ACLGad();
  if (detectorID < 7)
    return timeResolution_MPGD();
  return timeResolution_SiMaps();
}

std::pair<int, int> TimeframeSplitter::backEndEtaPhiBins(double hitEta, double hitPhi, int bShift) {
  // MPGD backward Endcap range -3.6 < eta < -1.72, +5%: -3.78 < eta < -1.634
  const double etaMin = -3.78;
  const double etaMax = -1.634;
  return etaPhiBins(hitEta, hitPhi, etaMin, etaMax, bShift);
}

std::pair<int, int> TimeframeSplitter::barrelEtaPhiBins(double hitEta, double hitPhi, int bShift) {
  // MPGD barrel In range -1.49 < eta < 1.722, +5%: -1.56 < eta < 1.81
  // MPGD barrel Out range -1.56 < eta < 1.61, +5%: -1.64 < eta < 1.70
  // TOF barrel range -1.39 < eta < 1.39, +5%: -1.46 < eta < 1.46
  // ECal barrel range -1.71 < eta < 1.31, +5%: -1.80 < eta < 1.38
  const double etaMin = -1.80;
  const double etaMax = 1.81;
  return etaPhiBins(hitEta, hitPhi, etaMin, etaMax, bShift);
}

std::pair<int, int> TimeframeSplitter::forwardEndEtaPhiBins(double hitEta, double hitPhi,
                                                            int bShift) {
  // MPGD forward Endcap range 2.0 < eta < 3.35, +5%: 1.90 < eta < 3.52
  // TOF forward Endcap range 1.86 < eta < 3.85, +5%: 1.77 < eta < 4.04
  // ECal forward Endcap range 1.4 < eta < 3.5, +5%: 1.33 < eta < 3.68
  const double etaMin = 1.77;
  const double etaMax = 4.04;
  return etaPhiBins(hitEta, hitPhi, etaMin, etaMax, bShift);
}

TimeframeSplitter::Result TimeframeSplitter::Unfold(const JEvent& parent, JEvent& child,
                                                    int child_idx) {
  const float m_timeframeWidth      = timeframeWidth();
  const float m_timesplitWidth      = timesplitWidth();
  const double timeResolution_mpgd  = timeResolution_MPGD();
  const double timeResolution_tof   = timeResolution_ACLGad();
  const double timeResolution_emcal = timeResolution_EMCal();

  bool m_bTrigger = false;

  // Materialize these output collections even when no timeslice triggers,
  // so the output schema is available from the first event.
  (void)m_eventHeader_outCol();
  (void)m_eventHeaderPhy_outCols();
  (void)m_eventHeaderBkg_outCols();

  const auto trackerHitCollsIn = m_trackerHits_inCols();
  const auto caloRecHitCollsIn = m_calorimeterHit_inCols();
  const auto trkAssoCollsIn    = m_trackerHitsAsso_inCols();
  const auto calrecAssoCollsIn = m_mcRecoCalorimeterHitAssociation_inCols();

  // == s == Register hits of TOF and MPGD detectors in the time slice ==================
  if (child_idx == 0) {
    m_OrigTFCount++;

    // Association collections belong to the parent Timeslice and remain valid for all
    // of its children. Build each lookup once per parent instead of rebuilding the full
    // index for every triggered PhysicsEvent.
    m_trkAssoIds.clear();
    m_trkAssoIds.reserve(trkAssoCollsIn.size());
    for (const auto* associations : trkAssoCollsIn) {
      m_trkAssoIds.push_back(buildTrkAssoId(associations));
    }

    m_calAssoIds.clear();
    m_calAssoIds.reserve(calrecAssoCollsIn.size());
    for (const auto* associations : calrecAssoCollsIn) {
      m_calAssoIds.push_back(buildCalAssoId(associations));
    }

    // == s == For MC Trigger Efficiency Estimation ~~~~~~~~
    m_vPhysCooTimes.clear();

    double prevMCTime = -9999.0; // temp check mc particle times
    for (const auto& mcparticle : *m_mcParticles_inCol()) {
      if (mcparticle.getGeneratorStatus() != 1)
        continue;
      if (std::abs(prevMCTime - mcparticle.getTime()) < 50.)
        continue;
      double mcCollTime = mcparticle.getTime();
      m_vPhysCooTimes.push_back(mcCollTime);
      prevMCTime = mcCollTime;
    }
    std::sort(m_vPhysCooTimes.begin(), m_vPhysCooTimes.end());
    auto last = std::unique(m_vPhysCooTimes.begin(), m_vPhysCooTimes.end());
    m_vPhysCooTimes.erase(last, m_vPhysCooTimes.end());
    // == e == For MC Trigger Efficiency Estimation ~~~~~~~~
  }
  // == e == Register hits of TOF and MPGD detectors in the time slice ==================

  // == s == Time frame scan loop ==========================================================
  double timesliceT0     = -999.0;
  bool bTimesliceTrigger = false;

  bool bMutipliTriggers[6]   = {false, false, false, false, false, false};
  double multipliTrigTime[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  if (m_timesplitWidth <= 0.0F) {
    throw std::runtime_error("TimeframeSplitter: timesplitWidth must be greater than zero");
  }
  if (m_timeframeWidth <= 0.0F) {
    throw std::runtime_error("TimeframeSplitter: timeframeWidth must be greater than zero");
  }

  const size_t nTimeSlices = static_cast<size_t>(std::floor(m_timeframeWidth / m_timesplitWidth));
  double tsTimeS           = 0.0;
  double tsTimeE           = 0.0;

  // Scan the timeframe one time slice at a time.
  // The scan stops early when a physics trigger fires; otherwise it terminates
  // after all time slices in the timeframe have been processed.
  while (iTimeSlice < nTimeSlices) {
    tsTimeS = iTimeSlice * m_timesplitWidth;
    tsTimeE = (iTimeSlice + 1) * m_timesplitWidth;
    if (tsTimeE > m_timeframeWidth)
      break;
    iTimeSlice++;

    // == s == Multiplisity threshold Triggers =======================================

    // == s == Multiplisity Single Triggers =======================================
    std::array<double, 8> singleTrig{};
    std::array<double, 8> singleTrigTime{};

    // s // EndCap Cal Trigger
    EtaPhiGrid backEndCalGrid{};
    EtaPhiGrid backEndCalGridShifted{};
    EtaPhiTimeGrid backEndIntTimesEtaPhi        = {};
    EtaPhiTimeGrid backEndIntTimesEtaPhiShifted = {};
    fillEtaPhiGrids(caloRecHitCollsIn.at(kCalEndcapN), iniCalHitPoint[kCalEndcapN],
                    timeResolution_emcal, tsTimeS, tsTimeE, backEndCalGrid, backEndCalGridShifted,
                    backEndIntTimesEtaPhi, backEndIntTimesEtaPhiShifted, backEndEtaPhiBins);
    singleTrig[0] =
        countGridCellsWithMultiplicity(backEndCalGrid, backEndCalGridShifted, backEndIntTimesEtaPhi,
                                       backEndIntTimesEtaPhiShifted, 10, singleTrigTime[0]);

    // s // EndCap Cal+Trk Match Trigger
    EtaPhiGrid backEndTrkGrid                          = {};
    EtaPhiGrid backEndTrkGridShifted                   = {};
    EtaPhiTimeGrid backEndIntTimesEtaPhiMatched        = {};
    EtaPhiTimeGrid backEndIntTimesEtaPhiMatchedShifted = {};
    fillEtaPhiGridsMatched(trackerHitCollsIn.at(kTrkBackwardMPGD), iniTrkHitPoint[kTrkBackwardMPGD],
                           timeResolution_mpgd, tsTimeS, tsTimeE, backEndCalGrid,
                           backEndCalGridShifted, backEndTrkGrid, backEndTrkGridShifted, 10,
                           backEndIntTimesEtaPhiMatched, backEndIntTimesEtaPhiMatchedShifted,
                           backEndEtaPhiBins);
    singleTrig[1] = countGridCellsWithMultiplicity(
        backEndTrkGrid, backEndTrkGridShifted, backEndIntTimesEtaPhiMatched,
        backEndIntTimesEtaPhiMatchedShifted, 1, singleTrigTime[1]);

    EtaPhiGrid barrelCalGrid                   = {};
    EtaPhiGrid barrelCalGridShifted            = {};
    EtaPhiTimeGrid barrelIntTimesEtaPhi        = {};
    EtaPhiTimeGrid barrelIntTimesEtaPhiShifted = {};
    fillEtaPhiGrids(caloRecHitCollsIn.at(kCalBarrelScifi), iniCalHitPoint[kCalBarrelScifi],
                    timeResolution_emcal, tsTimeS, tsTimeE, barrelCalGrid, barrelCalGridShifted,
                    barrelIntTimesEtaPhi, barrelIntTimesEtaPhiShifted, barrelEtaPhiBins);
    singleTrig[2] =
        countGridCellsWithMultiplicity(barrelCalGrid, barrelCalGridShifted, barrelIntTimesEtaPhi,
                                       barrelIntTimesEtaPhiShifted, 10, singleTrigTime[2]);

    EtaPhiGrid barrelTrkGrid                          = {};
    EtaPhiGrid barrelTrkGridShifted                   = {};
    EtaPhiTimeGrid barrelIntTimesEtaPhiMatched        = {};
    EtaPhiTimeGrid barrelIntTimesEtaPhiMatchedShifted = {};
    fillEtaPhiGridsMatched(
        trackerHitCollsIn.at(kTrkMPGDBarrel), iniTrkHitPoint[kTrkMPGDBarrel], timeResolution_mpgd,
        tsTimeS, tsTimeE, barrelCalGrid, barrelCalGridShifted, barrelTrkGrid, barrelTrkGridShifted,
        5, barrelIntTimesEtaPhiMatched, barrelIntTimesEtaPhiMatchedShifted, barrelEtaPhiBins);
    fillEtaPhiGridsMatched(trackerHitCollsIn.at(kTrkOuterMPGDBarrel),
                           iniTrkHitPoint[kTrkOuterMPGDBarrel], timeResolution_mpgd, tsTimeS,
                           tsTimeE, barrelCalGrid, barrelCalGridShifted, barrelTrkGrid,
                           barrelTrkGridShifted, 5, barrelIntTimesEtaPhiMatched,
                           barrelIntTimesEtaPhiMatchedShifted, barrelEtaPhiBins);
    fillEtaPhiGridsMatched(
        trackerHitCollsIn.at(kTrkTOFBarrel), iniTrkHitPoint[kTrkTOFBarrel], timeResolution_tof,
        tsTimeS, tsTimeE, barrelCalGrid, barrelCalGridShifted, barrelTrkGrid, barrelTrkGridShifted,
        5, barrelIntTimesEtaPhiMatched, barrelIntTimesEtaPhiMatchedShifted, barrelEtaPhiBins);
    singleTrig[3] = countGridCellsWithMultiplicity(
        barrelTrkGrid, barrelTrkGridShifted, barrelIntTimesEtaPhiMatched,
        barrelIntTimesEtaPhiMatchedShifted, 1, singleTrigTime[3]);

    EtaPhiGrid frontEndCalGrid                   = {};
    EtaPhiGrid frontEndCalGridShifted            = {};
    EtaPhiTimeGrid frontEndIntTimesEtaPhi        = {};
    EtaPhiTimeGrid frontEndIntTimesEtaPhiShifted = {};
    fillEtaPhiGrids(caloRecHitCollsIn.at(kCalEndcapP), iniCalHitPoint[kCalEndcapP],
                    timeResolution_emcal, tsTimeS, tsTimeE, frontEndCalGrid, frontEndCalGridShifted,
                    frontEndIntTimesEtaPhi, frontEndIntTimesEtaPhiShifted, forwardEndEtaPhiBins);
    singleTrig[4] = countGridCellsWithMultiplicity(
        frontEndCalGrid, frontEndCalGridShifted, frontEndIntTimesEtaPhi,
        frontEndIntTimesEtaPhiShifted, 10, singleTrigTime[4]);

    EtaPhiGrid frontEndTrkGrid                          = {};
    EtaPhiGrid frontEndTrkGridShifted                   = {};
    EtaPhiTimeGrid frontEndIntTimesEtaPhiMatched        = {};
    EtaPhiTimeGrid frontEndIntTimesEtaPhiMatchedShifted = {};
    fillEtaPhiGridsMatched(trackerHitCollsIn.at(kTrkForwardMPGD), iniTrkHitPoint[kTrkForwardMPGD],
                           timeResolution_mpgd, tsTimeS, tsTimeE, frontEndCalGrid,
                           frontEndCalGridShifted, frontEndTrkGrid, frontEndTrkGridShifted, 5,
                           frontEndIntTimesEtaPhiMatched, frontEndIntTimesEtaPhiMatchedShifted,
                           forwardEndEtaPhiBins);
    fillEtaPhiGridsMatched(trackerHitCollsIn.at(kTrkTOFEndcap), iniTrkHitPoint[kTrkTOFEndcap],
                           timeResolution_tof, tsTimeS, tsTimeE, frontEndCalGrid,
                           frontEndCalGridShifted, frontEndTrkGrid, frontEndTrkGridShifted, 5,
                           frontEndIntTimesEtaPhiMatched, frontEndIntTimesEtaPhiMatchedShifted,
                           forwardEndEtaPhiBins);
    singleTrig[5] = countGridCellsWithMultiplicity(
        frontEndTrkGrid, frontEndTrkGridShifted, frontEndIntTimesEtaPhiMatched,
        frontEndIntTimesEtaPhiMatchedShifted, 1, singleTrigTime[5]);

    const auto hitsB0 = countHitsInTimeWindow(trackerHitCollsIn.at(kTrkB0), iniTrkHitPoint[kTrkB0],
                                              timeResolution_tof, tsTimeS, tsTimeE);
    iniTrkHitPoint[kTrkB0] = hitsB0.nextStartID;
    singleTrig[6]          = hitsB0.count;
    singleTrigTime[6]      = hitsB0.average_time();

    double totalZDCEnergy      = 0.0;
    double totalZDCEnergyTime  = 0.0;
    const auto* recHitsZDCECal = caloRecHitCollsIn.at(kCalZDC);
    if (recHitsZDCECal != nullptr) {
      for (size_t iHit = iniCalHitPoint[kCalZDC]; iHit < recHitsZDCECal->size(); ++iHit) {
        const auto& hit      = recHitsZDCECal->at(iHit);
        const double hitTime = hit.getTime();
        if (hitTime - timeResolution_emcal > tsTimeE)
          break;
        if (judgeHitInTimeSlice(hitTime, timeResolution_emcal, tsTimeS, tsTimeE)) {
          totalZDCEnergy += hit.getEnergy();
          totalZDCEnergyTime += hit.getEnergy() * hitTime;
          iniCalHitPoint[kCalZDC] = iHit;
        }
      }
    }
    singleTrig[7]     = totalZDCEnergy;
    singleTrigTime[7] = totalZDCEnergy > 0.0 ? totalZDCEnergyTime / totalZDCEnergy : 0.0;

    const double etaPhiCalTriggerSum    = singleTrig[0] + singleTrig[2] + singleTrig[4];
    const double etaPhiCalTrkTriggerSum = singleTrig[1] + singleTrig[3] + singleTrig[5];
    bMutipliTriggers[0]                 = etaPhiCalTrkTriggerSum > 0 && singleTrig[6] > 4;
    bMutipliTriggers[1]                 = etaPhiCalTrkTriggerSum > 0 && singleTrig[7] > 50;
    bMutipliTriggers[2]                 = etaPhiCalTriggerSum > 0 && singleTrig[6] > 4;
    bMutipliTriggers[3]                 = etaPhiCalTriggerSum > 0 && singleTrig[7] > 0.005;
    bMutipliTriggers[4]                 = etaPhiCalTrkTriggerSum > 1;
    bMutipliTriggers[5]                 = etaPhiCalTriggerSum > 2;

    if (!bMutipliTriggers[0] && !bMutipliTriggers[1] && !bMutipliTriggers[2] &&
        !bMutipliTriggers[3] && !bMutipliTriggers[4] && !bMutipliTriggers[5])
      continue;

    const double fallbackTriggerTime = 0.5 * (tsTimeS + tsTimeE);
    if (bMutipliTriggers[0])
      multipliTrigTime[0] =
          averageSelectedTriggerTime(singleTrig, singleTrigTime, {1, 3, 5, 6}, fallbackTriggerTime);
    if (bMutipliTriggers[1])
      multipliTrigTime[1] =
          averageSelectedTriggerTime(singleTrig, singleTrigTime, {1, 3, 5, 7}, fallbackTriggerTime);
    if (bMutipliTriggers[2])
      multipliTrigTime[2] =
          averageSelectedTriggerTime(singleTrig, singleTrigTime, {0, 2, 4, 6}, fallbackTriggerTime);
    if (bMutipliTriggers[3])
      multipliTrigTime[3] =
          averageSelectedTriggerTime(singleTrig, singleTrigTime, {0, 2, 4, 7}, fallbackTriggerTime);
    if (bMutipliTriggers[4])
      multipliTrigTime[4] =
          averageSelectedTriggerTime(singleTrig, singleTrigTime, {1, 3, 5}, fallbackTriggerTime);
    if (bMutipliTriggers[5])
      multipliTrigTime[5] =
          averageSelectedTriggerTime(singleTrig, singleTrigTime, {0, 2, 4}, fallbackTriggerTime);
    double multiTrigCount = 0;
    double totalTrigTime  = 0.0;
    for (size_t iTrig = 0; iTrig < 6; ++iTrig) {
      if (bMutipliTriggers[iTrig]) {
        totalTrigTime += multipliTrigTime[iTrig];
        multiTrigCount++;
      }
    }
    timesliceT0 = multiTrigCount > 0 ? totalTrigTime / multiTrigCount : fallbackTriggerTime;

    // == s == Multiplisity Single Triggers =======================================

    if (bMutipliTriggers[0] || bMutipliTriggers[1] || bMutipliTriggers[2] || bMutipliTriggers[3] ||
        bMutipliTriggers[4] || bMutipliTriggers[5])
      bTimesliceTrigger =
          true; // ???? temporary, need to be removed after geometrical coincidence trigger is implemented
    if (bTimesliceTrigger)
      break;
  }
  // == e == Time frame scan loop ==========================================================

  m_bTrigger = bTimesliceTrigger;
  if (bTimesliceTrigger) {
    m_bOnceTriggered = true;
    // For now, a one-to-one relationship between timeslices and events
    child.SetEventNumber(m_NewEventCount);
    child.SetRunNumber(parent.GetRunNumber());
    m_NewEventCount++;
    // Clone truth particles before detector relations so every child SimTrackerHit can
    // point to an MCParticle owned by this PhysicsEvent rather than by the parent Timeslice.
    for (const auto& mcparticle : *m_mcParticles_inCol()) {
      m_mcParticles_outCol()->push_back(mcparticle.clone(false));
    }

    // == s == Register Tracker Hits =======================================================
    for (size_t trkDetID = 0; trkDetID < trackerHitCollsIn.size(); ++trkDetID) {
      const auto* trkCollIn = trackerHitCollsIn.at(trkDetID);

      if (trkCollIn == nullptr)
        continue;
      auto& trkCollOut          = m_trackerHits_outCols().at(trkDetID);
      const double detTimeReso  = trkTimeResolution(trkDetID);
      const auto* trkAssoCollIn = trkAssoCollsIn.at(trkDetID);
      auto& rawCollOut          = m_rawTrackerHit_outCols().at(trkDetID);
      auto& trkAssoCollOut      = m_trackerHitsAsso_outCols().at(trkDetID);

      for (size_t iHit = 0; iHit < trkCollIn->size(); ++iHit) {
        const auto& trkHit = trkCollIn->at(iHit);

        const double hitT = trkHit.getTime();
        if (!overlapsTimeWindow(hitT, detTimeReso, timesliceT0 - 10., timesliceT0 + 30.)) {
          continue;
        }

        iniTrkHitPoint[trkDetID] = iHit;
        copyTrkHitWithRelations(trkHit, trkAssoCollIn, m_trkAssoIds.at(trkDetID), trkCollOut,
                                rawCollOut, trkAssoCollOut, m_simTrackerHits_outCols().at(trkDetID),
                                m_recoTrackerHitLinks_outCols().at(trkDetID),
                                m_mcParticles_outCol());
      }
    }
    // == e == Register Tracker Hits =======================================================

    // == s == Register Calo Rec Hits =======================================================
    for (size_t calDetID = 0; calDetID < caloRecHitCollsIn.size(); ++calDetID) {
      const auto* caloInColl = caloRecHitCollsIn.at(calDetID);
      if (caloInColl == nullptr)
        continue;
      auto& caloOutColl = m_calorimeterHit_outCols().at(calDetID);

      const auto* caloInCollAsso = calrecAssoCollsIn.at(calDetID);
      if (caloInCollAsso == nullptr)
        continue;

      for (size_t iCalHit = 0; iCalHit < caloInColl->size(); ++iCalHit) {
        const auto& caloHit = caloInColl->at(iCalHit);

        double detTimeReso = timeResolution_emcal;
        double hitT        = caloHit.getTime();

        if (hitT - detTimeReso > timesliceT0 + 30.)
          continue;
        if (overlapsTimeWindow(hitT, detTimeReso, timesliceT0 - 10., timesliceT0 + 30.)) {
          auto copiedCaloHit = caloHit.clone();
          copiedCaloHit.setRawHit(edm4hep::RawCalorimeterHit());

          const auto rawHitFromRec = caloHit.getRawHit();
          if (rawHitFromRec.isAvailable()) {
            auto& rawCollOut  = m_rawCalorimeterHit_outCols().at(calDetID);
            auto copiedRawHit = rawHitFromRec.clone();
            rawCollOut->push_back(copiedRawHit);
            copiedCaloHit.setRawHit(copiedRawHit);

            auto& assocCollOut  = m_mcRecoCalorimeterHitAssociation_outCols().at(calDetID);
            auto& linkCollOut   = m_mcRecoCalorimeterHitLink_outCols().at(calDetID);
            auto& simCollOut    = m_simCalorimeterHit_outCols().at(calDetID);
            const auto rawHitID = rawHitFromRec.getObjectID();

            const auto& association_index = m_calAssoIds.at(calDetID);
            const auto assocIterCal       = association_index.find(objIdKey(rawHitID));

            if (assocIterCal != association_index.end()) {
              for (const size_t association_position : assocIterCal->second) {
                const auto assoc = caloInCollAsso->at(association_position);

                if (!assoc.getSimHit().isAvailable())
                  continue;

                auto copiedSimHit = assoc.getSimHit().clone(false);
                simCollOut->push_back(copiedSimHit);

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
    for (auto it = m_vPhysCooTimes.begin(); it != m_vPhysCooTimes.end(); ++it) {
      const double physCollTime = *it;
      if ((physCollTime + 20 > timesliceT0 - 10) && (physCollTime - 10 < timesliceT0 + 30)) {
        physEventWeight = 1;
        m_vPhysCooTimes.erase(it);
        break;
      }
    }

    if (physEventWeight == 1) {
      edm4hep::MutableEventHeader eventHeader_bkg;
      eventHeader_bkg.setRunNumber(m_eventNumber_TS * 10000 + child_idx);
      eventHeader_bkg.setEventNumber(m_eventNumber_TS);
      eventHeader_bkg.setTimeStamp(iTimeSlice);
      eventHeader_bkg.setWeight(1);
      m_eventHeaderBkg_outCols()->push_back(eventHeader_bkg);

      edm4hep::MutableEventHeader eventHeader_phy;
      eventHeader_phy.setRunNumber(m_eventNumber_TS * 10000 + child_idx);
      eventHeader_phy.setEventNumber(m_eventNumber_TS);
      eventHeader_phy.setTimeStamp(iTimeSlice);
      eventHeader_phy.setWeight(2);
      m_eventHeaderPhy_outCols()->push_back(eventHeader_phy);
      for (size_t iTrig = 0; iTrig < 6; ++iTrig) {
        if (bMutipliTriggers[iTrig]) {
          edm4hep::MutableEventHeader eventHeader_phy;
          eventHeader_phy.setRunNumber(m_eventNumber_TS * 10000 + child_idx);
          eventHeader_phy.setEventNumber(m_eventNumber_TS);
          eventHeader_phy.setTimeStamp(iTimeSlice);
          eventHeader_phy.setWeight(iTrig + 3);
          m_eventHeaderPhy_outCols()->push_back(eventHeader_phy);
        }
      }
      m_PhysCount++;
    } else if (physEventWeight == 2) {
      edm4hep::MutableEventHeader eventHeader_phy;
      eventHeader_phy.setRunNumber(m_eventNumber_TS * 10000 + child_idx);
      eventHeader_phy.setEventNumber(m_eventNumber_TS);
      eventHeader_phy.setTimeStamp(iTimeSlice);
      eventHeader_phy.setWeight(1);
      m_eventHeaderPhy_outCols()->push_back(eventHeader_phy);

      edm4hep::MutableEventHeader eventHeader_bkg;
      eventHeader_bkg.setRunNumber(m_eventNumber_TS * 10000 + child_idx);
      eventHeader_bkg.setEventNumber(m_eventNumber_TS);
      eventHeader_bkg.setTimeStamp(iTimeSlice);
      eventHeader_bkg.setWeight(2);
      m_eventHeaderBkg_outCols()->push_back(eventHeader_bkg);
      for (size_t iTrig = 0; iTrig < 6; ++iTrig) {
        if (bMutipliTriggers[iTrig]) {
          edm4hep::MutableEventHeader eventHeader_bkg;
          eventHeader_bkg.setRunNumber(m_eventNumber_TS * 10000 + child_idx);
          eventHeader_bkg.setEventNumber(m_eventNumber_TS);
          eventHeader_bkg.setTimeStamp(iTimeSlice);
          eventHeader_bkg.setWeight(iTrig + 3);
          m_eventHeaderBkg_outCols()->push_back(eventHeader_bkg);
        }
      }
    }
    m_eventNumber_TS++;

    // Insert an independent EventHeader object into the physics event.
    // A subset header would keep a reference to the parent frame collection.
    edm4hep::MutableEventHeader eventHeader;
    if (m_eventHeader_inCol() != nullptr && !m_eventHeader_inCol()->empty()) {
      const auto& eventHeader_in = m_eventHeader_inCol()->at(0);
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
    m_eventHeader_outCol()->push_back(eventHeader);
    // == e == For MC Trigger Efficiency Estimation ~~~~~~~~

    // == s == For QA relation valuables QA<><><><><><><><><><><><><><><><><>>
  }

  if (iTimeSlice >= nTimeSlices)
    m_bScanedAllTimeWindows = true;
  if (m_bScanedAllTimeWindows) {
    bInitialLoop     = true;
    m_bOnceTriggered = false;

    m_vPhysCooTimes.clear();

    m_bScanedAllTimeWindows = false;
    iTimeSlice              = 0;
    targetDetId             = 0;
    for (auto& start_point : iniTrkHitPoint)
      start_point = 0;
    for (auto& start_point : iniCalHitPoint)
      start_point = 0;

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
