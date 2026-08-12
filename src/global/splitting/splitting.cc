
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/MutableCalorimeterHit.h>
#include <edm4eic/MutableTrackerHit.h>
#include <edm4eic/TrackerHit.h>
#include <map>
#include <string>
#include <vector>

#include "TimeframeSplitter.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/event_building/HitTimeAlignment_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {

  std::vector<std::string> m_simtrackerhit_collection_names_aligned = {
      "TOFBarrelRecHits_aligned",          "TOFEndcapRecHits_aligned",
      "MPGDBarrelRecHits_aligned",         "OuterMPGDBarrelRecHits_aligned",
      "BackwardMPGDEndcapRecHits_aligned", "ForwardMPGDEndcapRecHits_aligned",
      "SiBarrelVertexRecHits_aligned",     "SiBarrelTrackerRecHits_aligned",
      "SiEndcapTrackerRecHits_aligned",    "B0TrackerRecHits_aligned",
      "TaggerTrackerRecHits_aligned",      "ForwardRomanPotRecHits_aligned",
      "ForwardOffMTrackerRecHits_aligned"};

  std::vector<std::string> m_simtrackerhit_collection_names = {
      "TOFBarrelRecHits",         "TOFEndcapRecHits",          "MPGDBarrelRecHits",
      "OuterMPGDBarrelRecHits",   "BackwardMPGDEndcapRecHits", "ForwardMPGDEndcapRecHits",
      "SiBarrelVertexRecHits",    "SiBarrelTrackerRecHits",    "SiEndcapTrackerRecHits",
      "B0TrackerRecHits",         "TaggerTrackerRecHits",      "ForwardRomanPotRecHits",
      "ForwardOffMTrackerRecHits"};

  std::vector<std::string> m_simcalorechit_collection_names = {"B0ECalRecHits",
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

  std::vector<std::string> m_simcalorechit_collection_names_aligned = {
      "B0ECalRecHits_aligned",
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

  InitJANAPlugin(app);

  const bool split_timeframes =
      app->RegisterParameter<bool>("split_timeframes", false, "Enable timeframe splitting");
  if (!split_timeframes) {
    return;
  }

  app->Add(new JOmniFactoryGeneratorT<eicrecon::HitTimeAlignment_factory<edm4eic::TrackerHit>>(
      "timeAlignment", m_simtrackerhit_collection_names, m_simtrackerhit_collection_names_aligned,
      app, JEventLevel::Timeslice));

  app->Add(
      new JOmniFactoryGeneratorT<eicrecon::HitTimeAlignment_factory<edm4eic::CalorimeterHit>>(
          "CalRecTimeAlignment", m_simcalorechit_collection_names,
          m_simcalorechit_collection_names_aligned, app, JEventLevel::Timeslice));

  // Unfolder that takes timeframes and splits them into physics events.
  app->Add(new TimeframeSplitter());
}
} // "C"
