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
#include "services/log/Log_service.h"

// 2D-STRIP DIGITIZATION
// - Is produced by "MPGDTrackerDigi".
// - Relies on 2DStrip version of "compact" geometry file.
// PIXEL DIGITIZATION
// - Is produced by "SiliconTrackerDigi".

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // ***** PIXEL or 2DSTRIP DIGITIZATION?
  // - This determines which of the MPGDTrackerDigi or SiliconTrackerDigi
  //  factory is used.
  // - It's encoded in XML constants "<detector>_2DStrip", which can be
  //  conveniently set in the XMLs of the MPGDs.
  // - It can be reset from command line via the "MPGD:SiFactoryPattern" option,
  //  which is a bit pattern: 0x1=CyMBaL, 0x2=OuterBarrel, ...
  // - It defaults (when neither XML constant nor command line option) to
  //  SiliconTrackerDigi.
  // Default
  unsigned int SiFactoryPattern = 0x3; // Full-scale SiliconTrackerDigi
  // XML constant
  auto log_service         = app->GetService<Log_service>();
  auto mLog                = log_service->logger("tracking");
  const char* MPGD_names[] = {"InnerMPGDBarrel", "MPGDOuterBarrel"};
  int nMPGDs               = sizeof(MPGD_names) / sizeof(char*);
  for (int mpgd = 0; mpgd < nMPGDs; mpgd++) {
    std::string MPGD_name(MPGD_names[mpgd]);
    std::string constant_name = MPGD_name + std::string("_2DStrip");
    try {
      auto detector = app->GetService<DD4hep_service>()->detector();
      int constant  = detector->constant<int>(constant_name);
      if (constant == 1) {
        SiFactoryPattern &= ~(0x1 << mpgd);
        mLog->info(R"(2DStrip XML loaded for "{}")", MPGD_name);
      } else {
        mLog->info(R"(pixel XML loaded for "{}")", MPGD_name);
      }
    } catch (...) {
      // Variable not present apply legacy pixel readout
    }
  }
  // Command line option
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
  for (int mpgd = 0; mpgd < nMPGDs; mpgd++) {
    std::string MPGD_name(MPGD_names[mpgd]);
    if (SiFactoryPattern & (0x1 << mpgd)) {
      mLog->info(R"(2DStrip digitization will be applied to "{}")", MPGD_name);
    } else {
      mLog->info(R"(pixel digitization will be applied to "{}")", MPGD_name);
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
        "MPGDBarrelRawHits", {"EventHeader", "MPGDBarrelHits"},
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
        "OuterMPGDBarrelRawHits", {"EventHeader", "OuterMPGDBarrelHits"},
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
