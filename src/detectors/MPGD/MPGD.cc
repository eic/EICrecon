// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JException.h>
#include <JANA/Utils/JTypeInfo.h>
#include <stdexcept>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/MPGDTrackerDigi_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

// 2D-STRIP DIGITIZATION = DEFAULT
// - Is produced by "MPGDTrackerDigi".
// - Relies on "MultiSegmentation" <readout> in "compact" geometry file.
// PIXEL DIGITIZATION = BROUGHT INTO PLAY BY OPTION "MPGD:SiFactoryPattern".
// - Is produced by "SiliconTrackerDigi".

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // PIXEL DIGITIZATION?
  // It's encoded in bit pattern "SiFactoryPattern": 0x1=CyMBaL, 0x2=OuterBarrel, ...
  // unsigned int SiFactoryPattern = 0x0; // no SiliconTrackerDigi
  unsigned int SiFactoryPattern = 0x3; // using SiliconTrackerDigi
  std::string SiFactoryPattern_str;
  app->SetDefaultParameter("MPGD:SiFactoryPattern", SiFactoryPattern_str,
                           "Hexadecimal Pattern of MPGDs digitized via \"SiliconTrackerDigi\"");
  if (!SiFactoryPattern_str.empty()) {
    try {
      SiFactoryPattern = std::stoul(SiFactoryPattern_str, nullptr, 16);
    } catch (const std::invalid_argument& e) {
      throw JException(
          R"(Option "MPGD:SiFactoryPattern": Error ("%s") parsing input
        string: '%s')",
          e.what(), SiFactoryPattern_str.c_str());
    }
  }

  // ***** "MPGDBarrel" (=CyMBaL)
  // Digitization
  if ((SiFactoryPattern & 0x1) != 0U) {
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "MPGDBarrelRawHits", {"EventHeader", "MPGDBarrelHits"},
        {"MPGDBarrelRawHits", "MPGDBarrelRawHitAssociations"},
        {
            .threshold      = 100 * dd4hep::eV,
            .timeResolution = 10,
        },
        app));
  } else {
    app->Add(new JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>(
        "MPGDBarrelRawHits", {"MPGDBarrelHits"},
        {"MPGDBarrelRawHits", "MPGDBarrelRawHitAssociations"},
        {
            .readout        = "MPGDBarrelHits",
            .threshold      = 100 * dd4hep::eV,
            .timeResolution = 10,
        },
        app));
  }

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "MPGDBarrelRecHits", {"MPGDBarrelRawHits"}, // Input data collection tags
      {"MPGDBarrelRecHits"},                      // Output data tag
      {
          .timeResolution = 10,
      },
      app));

  // ***** OuterMPGDBarrel
  // Digitization
  if ((SiFactoryPattern & 0x2) != 0U) {
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "OuterMPGDBarrelRawHits", {"EventHeader", "OuterMPGDBarrelHits"},
        {"OuterMPGDBarrelRawHits", "OuterMPGDBarrelRawHitAssociations"},
        {
            .threshold      = 100 * dd4hep::eV,
            .timeResolution = 10,
        },
        app));
  } else {
    app->Add(new JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>(
        "OuterMPGDBarrelRawHits", {"OuterMPGDBarrelHits"},
        {"OuterMPGDBarrelRawHits", "OuterMPGDBarrelRawHitAssociations"},
        {
            .readout        = "OuterMPGDBarrelHits",
            .threshold      = 100 * dd4hep::eV,
            .timeResolution = 10,
        },
        app));
  }

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "OuterMPGDBarrelRecHits", {"OuterMPGDBarrelRawHits"}, // Input data collection tags
      {"OuterMPGDBarrelRecHits"},                           // Output data tag
      {
          .timeResolution = 10,
      },
      app));

  // ***** "BackwardMPGDEndcap"
  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "BackwardMPGDEndcapRawHits", {"EventHeader", "BackwardMPGDEndcapHits"},
      {"BackwardMPGDEndcapRawHits", "BackwardMPGDEndcapRawHitAssociations"},
      {
          .threshold      = 100 * dd4hep::eV,
          .timeResolution = 10,
      },
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "BackwardMPGDEndcapRecHits", {"BackwardMPGDEndcapRawHits"}, // Input data collection tags
      {"BackwardMPGDEndcapRecHits"},                              // Output data tag
      {
          .timeResolution = 10,
      },
      app));

  // ""ForwardMPGDEndcap"
  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "ForwardMPGDEndcapRawHits", {"EventHeader", "ForwardMPGDEndcapHits"},
      {"ForwardMPGDEndcapRawHits", "ForwardMPGDEndcapRawHitAssociations"},
      {
          .threshold      = 100 * dd4hep::eV,
          .timeResolution = 10,
      },
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "ForwardMPGDEndcapRecHits", {"ForwardMPGDEndcapRawHits"}, // Input data collection tags
      {"ForwardMPGDEndcapRecHits"},                             // Output data tag
      {
          .timeResolution = 10,
      },
      app));
}
} // extern "C"
