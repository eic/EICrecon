// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#include "TimeframeSplitter.h"

#include <JANA/JEvent.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TMath.h>
#include <edm4hep/Vector3f.h>
#include <cmath>

TimeframeSplitter::TimeframeSplitter() {
  SetTypeName(NAME_OF_THIS);
  SetParentLevel(JEventLevel::Timeslice);
  SetChildLevel(JEventLevel::PhysicsEvent);
}

std::uint64_t TimeframeSplitter::object_id_key(const podio::ObjectID& object_id) {
  const auto collection_id =
      static_cast<std::uint64_t>(static_cast<std::uint32_t>(object_id.collectionID));
  const auto index = static_cast<std::uint64_t>(static_cast<std::uint32_t>(object_id.index));

  return (collection_id << 32U) | index;
}

TimeframeSplitter::TrackerAssociationIndex TimeframeSplitter::buildTrkAssoId(
    const edm4eic::MCRecoTrackerHitAssociationCollection* associations) {

  TrackerAssociationIndex index;
  if (associations == nullptr)
    return index;
  index.reserve(associations->size());

  for (size_t assoId = 0; assoId < associations->size(); ++assoId) {
    const auto association = associations->at(assoId);
    const auto raw_hit     = association.getRawHit();
    if (!raw_hit.isAvailable())
      continue;
    index[object_id_key(raw_hit.getObjectID())].push_back(assoId);
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
    const auto raw_hit     = association.getRawHit();
    if (!raw_hit.isAvailable())
      continue;
    index[object_id_key(raw_hit.getObjectID())].push_back(assoId);
  }

  return index;
}

bool TimeframeSplitter::overlaps_time_window(Double_t hitTime, Double_t resolution,
                                             Double_t window_start, Double_t window_end) {
  return hitTime + resolution > window_start && hitTime - resolution < window_end;
}

bool TimeframeSplitter::is_after_time_window(Double_t hitTime, Double_t resolution,
                                             Double_t window_end) {
  return hitTime - resolution >= window_end;
}

bool TimeframeSplitter::isValidEtaPhiBin(Int_t etaBin, Int_t phiBin) {
  return 0 <= etaBin && etaBin < kEtaPhiBins && 0 <= phiBin && phiBin < kEtaPhiBins;
}

bool TimeframeSplitter::is_hit_in_time_slice(Double_t hitTime, Double_t time_resolution,
                                             Double_t time_slice_start, Double_t time_slice_end) {
  return !(hitTime + time_resolution < time_slice_start ||
           hitTime - time_resolution > time_slice_end);
}

std::pair<Int_t, Int_t> TimeframeSplitter::etaPhiBins(Double_t hitEta, Double_t hitPhi,
                                                      Double_t etaMin, Double_t etaMax,
                                                      Int_t bShift) {
  const Double_t etaBinWidth = (etaMax - etaMin) / kEtaPhiBins;
  const Double_t phiMin      = -TMath::Pi();
  const Double_t phiMax      = TMath::Pi();
  const Double_t phiBinWidth = (phiMax - phiMin) / kEtaPhiBins;

  const Double_t halfEtaBin    = 0.5 * etaBinWidth * bShift;
  const Double_t halfPhiBin    = 0.5 * phiBinWidth * bShift;
  const Double_t shiftedEtaMin = etaMin + halfEtaBin;
  const Double_t shiftedEtaMax = etaMax + halfEtaBin;
  const Double_t shiftedPhiMin = phiMin + halfPhiBin;
  const Double_t shiftedPhiMax = phiMax + halfPhiBin;

  if (hitEta < shiftedEtaMin || hitEta >= shiftedEtaMax || hitPhi < shiftedPhiMin ||
      hitPhi >= shiftedPhiMax) {
    return {kInvalidEtaPhiBin, kInvalidEtaPhiBin};
  }

  const Int_t etaBin = static_cast<Int_t>(std::floor((hitEta - shiftedEtaMin) / etaBinWidth));
  const Int_t phiBin = static_cast<Int_t>(std::floor((hitPhi - shiftedPhiMin) / phiBinWidth));
  return {etaBin, phiBin};
}

size_t TimeframeSplitter::countGridCellsWithMultiplicity(const EtaPhiGrid& grid0,
                                                         const EtaPhiGrid& gridShifted,
                                                         const EtaPhiTimeGrid& gridTime0,
                                                         const EtaPhiTimeGrid& gridShiftedTime,
                                                         Int_t threshold, Double_t& averageTime) {
  size_t count     = 0;
  Double_t timeSum = 0.0;
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

Double_t TimeframeSplitter::averageSelectedTriggerTime(const std::array<Double_t, 8>& values,
                                                       const std::array<Double_t, 8>& times,
                                                       std::initializer_list<size_t> indices,
                                                       Double_t fallbackTime) {
  size_t count     = 0;
  Double_t timeSum = 0.0;
  for (const size_t index : indices) {
    if (values[index] <= 0.0)
      continue;
    timeSum += times[index];
    count++;
  }
  return count > 0 ? timeSum / count : fallbackTime;
}

Double_t TimeframeSplitter::tracker_time_resolution(size_t detector_id) {
  if (detector_id < 3)
    return timeResolution_ACLGad();
  if (detector_id < 7)
    return timeResolution_MPGD();
  return timeResolution_SiMaps();
}

std::pair<Int_t, Int_t> TimeframeSplitter::backEndEtaPhiBins(Double_t hitEta, Double_t hitPhi,
                                                             Int_t bShift) {
  // MPGD backward Endcap range -3.6 < eta < -1.72, +5%: -3.78 < eta < -1.634
  const Double_t etaMin = -3.78;
  const Double_t etaMax = -1.634;
  return etaPhiBins(hitEta, hitPhi, etaMin, etaMax, bShift);
}

std::pair<Int_t, Int_t> TimeframeSplitter::barrelEtaPhiBins(Double_t hitEta, Double_t hitPhi,
                                                            Int_t bShift) {
  // MPGD barrel In range -1.49 < eta < 1.722, +5%: -1.56 < eta < 1.81
  // MPGD barrel Out range -1.56 < eta < 1.61, +5%: -1.64 < eta < 1.70
  // TOF barrel range -1.39 < eta < 1.39, +5%: -1.46 < eta < 1.46
  // ECal barrel range -1.71 < eta < 1.31, +5%: -1.80 < eta < 1.38
  const Double_t etaMin = -1.80;
  const Double_t etaMax = 1.81;
  return etaPhiBins(hitEta, hitPhi, etaMin, etaMax, bShift);
}

std::pair<Int_t, Int_t> TimeframeSplitter::forwardEndEtaPhiBins(Double_t hitEta, Double_t hitPhi,
                                                                Int_t bShift) {
  // MPGD forward Endcap range 2.0 < eta < 3.35, +5%: 1.90 < eta < 3.52
  // TOF forward Endcap range 1.86 < eta < 3.85, +5%: 1.77 < eta < 4.04
  // ECal forward Endcap range 1.4 < eta < 3.5, +5%: 1.33 < eta < 3.68
  const Double_t etaMin = 1.77;
  const Double_t etaMax = 4.04;
  return etaPhiBins(hitEta, hitPhi, etaMin, etaMax, bShift);
}

TimeframeSplitter::Result TimeframeSplitter::Unfold(const JEvent& parent, JEvent& child,
                                                    int child_idx) {
  const float m_timeframe_width        = timeframe_width();
  const float m_timesplit_width        = timesplit_width();
  const Double_t time_resolution_mpgd  = timeResolution_MPGD();
  const Double_t time_resolution_tof   = timeResolution_ACLGad();
  const Double_t time_resolution_emcal = timeResolution_EMCal();

  Bool_t m_bTrigger = false;

  // Materialize these output collections even when no timeslice triggers,
  // so the output schema is available from the first event.
  (void)m_event_header_out();
  (void)m_event_header_phy_out();
  (void)m_event_header_bkg_out();

  const auto trackerHitCollsIn = m_trackerhits_in();
  const auto caloRecHitCollsIn = m_calorechit_in();
  const auto trkAssoCollsIn    = m_trackerhitsAsso_in();
  const auto calrecAssoCollsIn = m_calorechitassociation_in();

  // == s == Register hits of TOF and MPGD detectors in the time slice ==================
  if (child_idx == 0) {
    m_OrigTFCount++;

    // Association collections belong to the parent Timeslice and remain valid for all
    // of its children. Build each lookup once per parent instead of rebuilding the full
    // index for every triggered PhysicsEvent.
    m_tracker_association_indices.clear();
    m_tracker_association_indices.reserve(trkAssoCollsIn.size());
    for (const auto* associations : trkAssoCollsIn) {
      m_tracker_association_indices.push_back(buildTrkAssoId(associations));
    }

    m_calorimeter_association_indices.clear();
    m_calorimeter_association_indices.reserve(calrecAssoCollsIn.size());
    for (const auto* associations : calrecAssoCollsIn) {
      m_calorimeter_association_indices.push_back(buildCalAssoId(associations));
    }

    // == s == For MC Trigger Efficiency Estimation ~~~~~~~~
    m_vPhysCooTimes.clear();

    Double_t prevMCTime = -9999.0; // temp check mc particle times
    for (const auto& mcparticle : *m_mcparticles_in()) {
      if (mcparticle.getGeneratorStatus() != 1)
        continue;
      if (std::abs(prevMCTime - mcparticle.getTime()) < 50.)
        continue;
      Double_t mcCollTime = mcparticle.getTime();
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
  Double_t timesliceT0     = -999.0;
  Bool_t bTimesliceTrigger = false;

  Bool_t bMutipliTriggers[6]   = {false, false, false, false, false, false};
  Double_t multipliTrigTime[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  if (m_timesplit_width <= 0.0F) {
    throw std::runtime_error("TimeframeSplitter: timesplit_width must be greater than zero");
  }
  if (m_timeframe_width <= 0.0F) {
    throw std::runtime_error("TimeframeSplitter: timeframe_width must be greater than zero");
  }
  const size_t nTimeSlices = static_cast<size_t>(std::floor(m_timeframe_width / m_timesplit_width));

  Double_t tsTimeS = 0.0;
  Double_t tsTimeE = 0.0;
  // Scan the timeframe one time slice at a time.
  // The scan stops early when a physics trigger fires; otherwise it terminates
  // after all time slices in the timeframe have been processed.
  while (iTimeSlice < nTimeSlices) {
    tsTimeS = iTimeSlice * m_timesplit_width;
    tsTimeE = (iTimeSlice + 1) * m_timesplit_width;
    if (tsTimeE > m_timeframe_width)
      break;
    iTimeSlice++;

    // == s == Multiplisity threshold Triggers =======================================

    // == s == Multiplisity Single Triggers =======================================
    std::array<Double_t, 8> singleTrig{};
    std::array<Double_t, 8> singleTrigTime{};

    // s // EndCap Cal Trigger
    EtaPhiGrid backEndCalGrid{};
    EtaPhiGrid backEndCalGridShifted{};
    EtaPhiTimeGrid backEndIntTimesEtaPhi        = {};
    EtaPhiTimeGrid backEndIntTimesEtaPhiShifted = {};
    fillEtaPhiGrids(caloRecHitCollsIn.at(kCalEndcapN), iniCalHitPoint[kCalEndcapN],
                    time_resolution_emcal, tsTimeS, tsTimeE, backEndCalGrid, backEndCalGridShifted,
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
                           time_resolution_mpgd, tsTimeS, tsTimeE, backEndCalGrid,
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
                    time_resolution_emcal, tsTimeS, tsTimeE, barrelCalGrid, barrelCalGridShifted,
                    barrelIntTimesEtaPhi, barrelIntTimesEtaPhiShifted, barrelEtaPhiBins);
    singleTrig[2] =
        countGridCellsWithMultiplicity(barrelCalGrid, barrelCalGridShifted, barrelIntTimesEtaPhi,
                                       barrelIntTimesEtaPhiShifted, 10, singleTrigTime[2]);

    EtaPhiGrid barrelTrkGrid                          = {};
    EtaPhiGrid barrelTrkGridShifted                   = {};
    EtaPhiTimeGrid barrelIntTimesEtaPhiMatched        = {};
    EtaPhiTimeGrid barrelIntTimesEtaPhiMatchedShifted = {};
    fillEtaPhiGridsMatched(
        trackerHitCollsIn.at(kTrkMPGDBarrel), iniTrkHitPoint[kTrkMPGDBarrel], time_resolution_mpgd,
        tsTimeS, tsTimeE, barrelCalGrid, barrelCalGridShifted, barrelTrkGrid, barrelTrkGridShifted,
        5, barrelIntTimesEtaPhiMatched, barrelIntTimesEtaPhiMatchedShifted, barrelEtaPhiBins);
    fillEtaPhiGridsMatched(trackerHitCollsIn.at(kTrkOuterMPGDBarrel),
                           iniTrkHitPoint[kTrkOuterMPGDBarrel], time_resolution_mpgd, tsTimeS,
                           tsTimeE, barrelCalGrid, barrelCalGridShifted, barrelTrkGrid,
                           barrelTrkGridShifted, 5, barrelIntTimesEtaPhiMatched,
                           barrelIntTimesEtaPhiMatchedShifted, barrelEtaPhiBins);
    fillEtaPhiGridsMatched(
        trackerHitCollsIn.at(kTrkTOFBarrel), iniTrkHitPoint[kTrkTOFBarrel], time_resolution_tof,
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
                    time_resolution_emcal, tsTimeS, tsTimeE, frontEndCalGrid,
                    frontEndCalGridShifted, frontEndIntTimesEtaPhi, frontEndIntTimesEtaPhiShifted,
                    forwardEndEtaPhiBins);
    singleTrig[4] = countGridCellsWithMultiplicity(
        frontEndCalGrid, frontEndCalGridShifted, frontEndIntTimesEtaPhi,
        frontEndIntTimesEtaPhiShifted, 10, singleTrigTime[4]);

    EtaPhiGrid frontEndTrkGrid                          = {};
    EtaPhiGrid frontEndTrkGridShifted                   = {};
    EtaPhiTimeGrid frontEndIntTimesEtaPhiMatched        = {};
    EtaPhiTimeGrid frontEndIntTimesEtaPhiMatchedShifted = {};
    fillEtaPhiGridsMatched(trackerHitCollsIn.at(kTrkForwardMPGD), iniTrkHitPoint[kTrkForwardMPGD],
                           time_resolution_mpgd, tsTimeS, tsTimeE, frontEndCalGrid,
                           frontEndCalGridShifted, frontEndTrkGrid, frontEndTrkGridShifted, 5,
                           frontEndIntTimesEtaPhiMatched, frontEndIntTimesEtaPhiMatchedShifted,
                           forwardEndEtaPhiBins);
    fillEtaPhiGridsMatched(trackerHitCollsIn.at(kTrkTOFEndcap), iniTrkHitPoint[kTrkTOFEndcap],
                           time_resolution_tof, tsTimeS, tsTimeE, frontEndCalGrid,
                           frontEndCalGridShifted, frontEndTrkGrid, frontEndTrkGridShifted, 5,
                           frontEndIntTimesEtaPhiMatched, frontEndIntTimesEtaPhiMatchedShifted,
                           forwardEndEtaPhiBins);
    singleTrig[5] = countGridCellsWithMultiplicity(
        frontEndTrkGrid, frontEndTrkGridShifted, frontEndIntTimesEtaPhiMatched,
        frontEndIntTimesEtaPhiMatchedShifted, 1, singleTrigTime[5]);

    const auto hitsB0 = count_hits_in_window(trackerHitCollsIn.at(kTrkB0), iniTrkHitPoint[kTrkB0],
                                             time_resolution_tof, tsTimeS, tsTimeE);
    iniTrkHitPoint[kTrkB0] = hitsB0.next_start_index;
    singleTrig[6]          = hitsB0.count;
    singleTrigTime[6]      = hitsB0.average_time();

    Double_t totalZDCEnergy     = 0.0;
    Double_t totalZDCEnergyTime = 0.0;
    const auto* recHitsZDCECal  = caloRecHitCollsIn.at(kCalZDC);
    if (recHitsZDCECal != nullptr) {
      for (size_t iHit = iniCalHitPoint[kCalZDC]; iHit < recHitsZDCECal->size(); ++iHit) {
        const auto& hit        = recHitsZDCECal->at(iHit);
        const Double_t hitTime = hit.getTime();
        if (hitTime - time_resolution_emcal > tsTimeE)
          break;
        if (is_hit_in_time_slice(hitTime, time_resolution_emcal, tsTimeS, tsTimeE)) {
          totalZDCEnergy += hit.getEnergy();
          totalZDCEnergyTime += hit.getEnergy() * hitTime;
          iniCalHitPoint[kCalZDC] = iHit;
        }
      }
    }
    singleTrig[7]     = totalZDCEnergy;
    singleTrigTime[7] = totalZDCEnergy > 0.0 ? totalZDCEnergyTime / totalZDCEnergy : 0.0;

    const Double_t etaPhiCalTriggerSum    = singleTrig[0] + singleTrig[2] + singleTrig[4];
    const Double_t etaPhiCalTrkTriggerSum = singleTrig[1] + singleTrig[3] + singleTrig[5];
    bMutipliTriggers[0]                   = etaPhiCalTrkTriggerSum > 0 && singleTrig[6] > 4;
    bMutipliTriggers[1]                   = etaPhiCalTrkTriggerSum > 0 && singleTrig[7] > 50;
    bMutipliTriggers[2]                   = etaPhiCalTriggerSum > 0 && singleTrig[6] > 4;
    bMutipliTriggers[3]                   = etaPhiCalTriggerSum > 0 && singleTrig[7] > 0.005;
    bMutipliTriggers[4]                   = etaPhiCalTrkTriggerSum > 1;
    bMutipliTriggers[5]                   = etaPhiCalTriggerSum > 2;

    if (!bMutipliTriggers[0] && !bMutipliTriggers[1] && !bMutipliTriggers[2] &&
        !bMutipliTriggers[3] && !bMutipliTriggers[4] && !bMutipliTriggers[5])
      continue;

    const Double_t fallbackTriggerTime = 0.5 * (tsTimeS + tsTimeE);
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
    Double_t multiTrigCount = 0;
    Double_t totalTrigTime  = 0.0;
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
    for (const auto& mcparticle : *m_mcparticles_in()) {
      m_mcparticles_out()->push_back(mcparticle.clone(false));
    }

    // == s == Register Tracker Hits =======================================================
    for (size_t trkDetID = 0; trkDetID < trackerHitCollsIn.size(); ++trkDetID) {
      const auto* trkCollIn = trackerHitCollsIn.at(trkDetID);

      if (trkCollIn == nullptr)
        continue;
      auto& trkCollOut           = m_trackerhits_out().at(trkDetID);
      const Double_t detTimeReso = tracker_time_resolution(trkDetID);
      const auto* trkAssoCollIn  = trkAssoCollsIn.at(trkDetID);
      auto& rawCollOut           = m_rawhit_out().at(trkDetID);
      auto& trkAssoCollOut       = m_trackerhitsAsso_out().at(trkDetID);

      for (size_t iHit = 0; iHit < trkCollIn->size(); ++iHit) {
        const auto& trkHit = trkCollIn->at(iHit);

        const Double_t hitT = trkHit.getTime();
        if (!overlaps_time_window(hitT, detTimeReso, timesliceT0 - 10., timesliceT0 + 30.)) {
          continue;
        }

        iniTrkHitPoint[trkDetID] = iHit;
        copy_tracker_hit_with_relations(
            trkHit, trkAssoCollIn, m_tracker_association_indices.at(trkDetID), trkCollOut,
            rawCollOut, trkAssoCollOut, m_simtrackerhits_out().at(trkDetID),
            m_rawhitlinks_out().at(trkDetID), m_mcparticles_out());
      }
    }
    // == e == Register Tracker Hits =======================================================

    // == s == Register Calo Rec Hits =======================================================
    for (size_t calDetID = 0; calDetID < caloRecHitCollsIn.size(); ++calDetID) {
      const auto* caloInColl = caloRecHitCollsIn.at(calDetID);
      if (caloInColl == nullptr)
        continue;
      auto& caloOutColl = m_calorechit_out().at(calDetID);

      const auto* caloInCollAsso = calrecAssoCollsIn.at(calDetID);
      if (caloInCollAsso == nullptr)
        continue;

      for (size_t iCalHit = 0; iCalHit < caloInColl->size(); ++iCalHit) {
        const auto& caloHit = caloInColl->at(iCalHit);

        Double_t detTimeReso = time_resolution_emcal;
        Double_t hitT        = caloHit.getTime();

        if (hitT - detTimeReso > timesliceT0 + 30.)
          continue;
        if (overlaps_time_window(hitT, detTimeReso, timesliceT0 - 10., timesliceT0 + 30.)) {
          auto copiedCaloHit = caloHit.clone();
          copiedCaloHit.setRawHit(edm4hep::RawCalorimeterHit());

          const auto rawHitFromRec = caloHit.getRawHit();
          if (rawHitFromRec.isAvailable()) {
            auto& rawCollOut  = m_calorawhit_out().at(calDetID);
            auto copiedRawHit = rawHitFromRec.clone();
            rawCollOut->push_back(copiedRawHit);
            copiedCaloHit.setRawHit(copiedRawHit);

            auto& assocCollOut  = m_calorechitassociation_out().at(calDetID);
            auto& linkCollOut   = m_calorawhitlinks_out().at(calDetID);
            auto& simCollOut    = m_simcalorimeterhits_out().at(calDetID);
            const auto rawHitID = rawHitFromRec.getObjectID();

            const auto& association_index = m_calorimeter_association_indices.at(calDetID);
            const auto assocIterCal       = association_index.find(object_id_key(rawHitID));

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
    Int_t physEventWeight = 2;
    for (auto it = m_vPhysCooTimes.begin(); it != m_vPhysCooTimes.end(); ++it) {
      const Double_t physCollTime = *it;
      if ((physCollTime + 20 > timesliceT0 - 10) && (physCollTime - 10 < timesliceT0 + 30)) {
        physEventWeight = 1;
        m_vPhysCooTimes.erase(it);
        break;
      }
    }

    if (physEventWeight == 1) {
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
      for (size_t iTrig = 0; iTrig < 6; ++iTrig) {
        if (bMutipliTriggers[iTrig]) {
          edm4hep::MutableEventHeader event_header_phy;
          event_header_phy.setRunNumber(m_event_number_ts * 10000 + child_idx);
          event_header_phy.setEventNumber(m_event_number_ts);
          event_header_phy.setTimeStamp(iTimeSlice);
          event_header_phy.setWeight(iTrig + 3);
          m_event_header_phy_out()->push_back(event_header_phy);
        }
      }
      m_PhysCount++;
    } else if (physEventWeight == 2) {
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
      for (size_t iTrig = 0; iTrig < 6; ++iTrig) {
        if (bMutipliTriggers[iTrig]) {
          edm4hep::MutableEventHeader event_header_bkg;
          event_header_bkg.setRunNumber(m_event_number_ts * 10000 + child_idx);
          event_header_bkg.setEventNumber(m_event_number_ts);
          event_header_bkg.setTimeStamp(iTimeSlice);
          event_header_bkg.setWeight(iTrig + 3);
          m_event_header_bkg_out()->push_back(event_header_bkg);
        }
      }
    }
    m_event_number_ts++;

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

void TimeframeSplitter::thetaPhiBinCalc(edm4eic::TrackerHit hit, Int_t& thetaID1, Int_t& phiID1,
                                        Int_t& thetaID2, Int_t& phiID2) {
  Double_t hitX       = hit.getPosition()[0];
  Double_t hitY       = hit.getPosition()[1];
  Double_t hitZ       = hit.getPosition()[2];
  const Double_t hitR = TMath::Sqrt(hitX * hitX + hitY * hitY + hitZ * hitZ);
  if (hitR <= 0.0) {
    thetaID1 = 0;
    thetaID2 = 0;
    phiID1   = 0;
    phiID2   = 0;
    return;
  }

  const Double_t cosTheta = std::clamp(hitZ / hitR, -1.0, 1.0);
  const Double_t hitTheta = TMath::ACos(cosTheta);

  Double_t hitPhi = TMath::ATan2(hitY, hitX) + 2 * TMath::Pi();
  if (hitPhi < 0)
    hitPhi += 2 * TMath::Pi();
  thetaID1 = hitTheta / (TMath::Pi() / 12.);
  thetaID2 = (hitTheta + TMath::Pi() / 24.) / (TMath::Pi() / 12.);
  phiID1   = hitPhi / (TMath::Pi() / 8.);
  phiID2   = (hitPhi + TMath::Pi() / 16.) / (TMath::Pi() / 8.);
}
