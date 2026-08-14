// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/TrackerHit.h>
#include <string>
#include <vector>

#include "TimeframeSplitter.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/event_building/HitTimeAlignment_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {

    const std::vector<std::pair<std::string, std::string>> trkHitTimeCollectionNames = {
        {"TOFBarrelRecHits", "TOFBarrelRecHits_aligned"},
        {"TOFEndcapRecHits", "TOFEndcapRecHits_aligned"},
        {"MPGDBarrelRecHits", "MPGDBarrelRecHits_aligned"},
        {"OuterMPGDBarrelRecHits", "OuterMPGDBarrelRecHits_aligned"},
        {"BackwardMPGDEndcapRecHits", "BackwardMPGDEndcapRecHits_aligned"},
        {"ForwardMPGDEndcapRecHits", "ForwardMPGDEndcapRecHits_aligned"},
        {"SiBarrelVertexRecHits", "SiBarrelVertexRecHits_aligned"},
        {"SiBarrelTrackerRecHits", "SiBarrelTrackerRecHits_aligned"},
        {"SiEndcapTrackerRecHits", "SiEndcapTrackerRecHits_aligned"},
        {"B0TrackerRecHits", "B0TrackerRecHits_aligned"},
        {"TaggerTrackerRecHits", "TaggerTrackerRecHits_aligned"},
        {"ForwardRomanPotRecHits", "ForwardRomanPotRecHits_aligned"},
        {"ForwardOffMTrackerRecHits", "ForwardOffMTrackerRecHits_aligned"},
    };

    const std::vector<std::pair<std::string, std::string>> calHitTimeCollectionNames = {
        {"B0ECalRecHits", "B0ECalRecHits_aligned"},
        {"EcalBarrelImagingRecHits", "EcalBarrelImagingRecHits_aligned"},
        {"EcalBarrelScFiRecHits", "EcalBarrelScFiRecHits_aligned"},
        {"EcalEndcapNRecHits", "EcalEndcapNRecHits_aligned"},
        {"EcalEndcapPRecHits", "EcalEndcapPRecHits_aligned"},
        {"EcalFarForwardZDCRecHits", "EcalFarForwardZDCRecHits_aligned"},
        {"EcalLumiSpecRecHits", "EcalLumiSpecRecHits_aligned"},
        {"HcalBarrelRecHits", "HcalBarrelRecHits_aligned"},
        {"HcalEndcapNRecHits", "HcalEndcapNRecHits_aligned"},
        {"HcalEndcapPInsertRecHits", "HcalEndcapPInsertRecHits_aligned"},
        {"HcalFarForwardZDCRecHits", "HcalFarForwardZDCRecHits_aligned"},
        {"LFHCALRecHits", "LFHCALRecHits_aligned"},
    };


  InitJANAPlugin(app);

  const bool splitTimeframes =
      app->RegisterParameter<bool>("split_timeframes", false, "Enable timeframe splitting");
  if (!splitTimeframes) {
    return;
  }

    for (const auto& [inName, outName] : trkHitTimeCollectionNames) {
        app->Add(
            new JOmniFactoryGeneratorT<eicrecon::HitTimeAlignment_factory<edm4eic::TrackerHit>>(
                outName, {inName}, {outName}, app,
                JEventLevel::Timeslice));
    }

    for (const auto& [inName, outName] : calHitTimeCollectionNames) {
        app->Add(
            new JOmniFactoryGeneratorT<eicrecon::HitTimeAlignment_factory<edm4eic::CalorimeterHit>>(
                outName, {inName}, {outName}, app,
                JEventLevel::Timeslice));
    }

  // Unfolder that takes timeframes and splits them into physics events.
  app->Add(new TimeframeSplitter());
}
} // "C"
