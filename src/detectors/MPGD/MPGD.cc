// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <DD4hep/Detector.h>
#include <edm4eic/EDM4eicVersion.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JException.h>
#include <JANA/Utils/JTypeInfo.h>
#include <fmt/format.h>
#include <spdlog/logger.h>
#include <gsl/pointers>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/MPGDTrackerDigi_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"
#include "factories/tracking/MPGDHitReconstruction_factory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
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
  auto log_service               = app->GetService<Log_service>();
  auto mLog                      = log_service->logger("tracking");
  const int nMPGDs               = 2;
  const char* MPGD_names[nMPGDs] = {"InnerMPGDBarrel", "MPGDOuterBarrel"};
  for (int mpgd = 0; mpgd < nMPGDs; mpgd++) {
    std::string MPGD_name(MPGD_names[mpgd]);
    std::string constantName = MPGD_name + std::string("_2DStrip");
    try {
      auto detector = app->GetService<DD4hep_service>()->detector();
      int constant  = detector->constant<int>(constantName);
      if (constant == 1) {
        SiFactoryPattern &= ~(0x1 << mpgd);
        mLog->info(R"(2DStrip XML loaded for "{}")", MPGD_name);
      } else {
        mLog->info(R"(pixel XML loaded for "{}")", MPGD_name);
      }
    } catch (const std::runtime_error&) {
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
        string: '%s'.)",
          e.what(), SiFactoryPattern_str.c_str());
    }
  }
  for (int mpgd = 0; mpgd < nMPGDs; mpgd++) {
    std::string MPGD_name(MPGD_names[mpgd]);
    if (SiFactoryPattern & (0x1 << mpgd)) {
      mLog->info(R"(pixel digitization will be applied to "{}")", MPGD_name);
    } else {
      mLog->info(R"(2DStrip digitization will be applied to "{}")", MPGD_name);
    }
  }

  // ***** "MPGDBarrel" (=CyMBaL)
  // Digitization
  if ((SiFactoryPattern & 0x1) != 0U) {
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "MPGDBarrelRawHits", {"EventHeader", "MPGDBarrelHits"},
        {"MPGDBarrelRawHits",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
         "MPGDBarrelRawHitLinks",
#endif
         "MPGDBarrelRawHitAssociations"},
        {
            .threshold      = 100 * dd4hep::eV,
            .timeResolution = 10,
        },
        app));
  } else {
    // Configuration parameters
    MPGDTrackerDigiConfig digi_cfg;
    digi_cfg.readout             = "MPGDBarrelHits";
    digi_cfg.threshold           = 100 * dd4hep::eV;
    digi_cfg.timeResolution      = 10;
    digi_cfg.gain                = 10000;
    digi_cfg.stripResolutions[0] = digi_cfg.stripResolutions[1] = 150 * dd4hep::um;
    // Get #channels from XML
    const char* constantNames[] = {"MMnStripsPhi", "MMnStripsZ"};
    for (int phiZ = 0; phiZ < 2; phiZ++) {
      std::string constantName = std::string(constantNames[phiZ]);
      try {
        auto detector               = app->GetService<DD4hep_service>()->detector();
        digi_cfg.stripNumbers[phiZ] = detector->constant<int>(constantName);
      } catch (...) {
        throw JException(
            R"(MPGD "%s": Error retrieving #channels from XML: no "%s" constant found.)",
            digi_cfg.readout.c_str(), constantName.c_str());
      }
    }
    app->Add(new JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>(
        "MPGDBarrelRawHits", {"EventHeader", "MPGDBarrelHits"},
        {"MPGDBarrelRawHits",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
	 "MPGDBarrelRawHitLinks",
#endif
	 "MPGDBarrelRawHitAssociations"}, digi_cfg, app));
  }

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  if ((SiFactoryPattern & 0x1) != 0U) {
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "MPGDBarrelRecHits", {"MPGDBarrelRawHits"}, // Input data collection tags
        {"MPGDBarrelRecHits"},                      // Output data tag
        {
            .timeResolution = 10,
        },
        app));
  } else {
    MPGDHitReconstructionConfig digi_cfg;
    digi_cfg.readout             = "MPGDBarrelHits";
    digi_cfg.timeResolution      = 10;
    digi_cfg.stripResolutions[0] = digi_cfg.stripResolutions[1] = 150 * dd4hep::um;
    app->Add(new JOmniFactoryGeneratorT<MPGDHitReconstruction_factory>(
        "MPGDBarrelRecHits", {"MPGDBarrelRawHits"}, // Input data collection tags
        {"MPGDBarrelRecHits"},                      // Output data tag
        digi_cfg, app));
  }

  // ***** OuterMPGDBarrel
  // Digitization
  if ((SiFactoryPattern & 0x2) != 0U) {
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "OuterMPGDBarrelRawHits", {"EventHeader", "OuterMPGDBarrelHits"},
        {"OuterMPGDBarrelRawHits",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
         "OuterMPGDBarrelRawHitLinks",
#endif
         "OuterMPGDBarrelRawHitAssociations"},
        {
            .threshold      = 100 * dd4hep::eV,
            .timeResolution = 10,
        },
        app));
  } else {
    MPGDTrackerDigiConfig digi_cfg;
    digi_cfg.readout             = "OuterMPGDBarrelHits";
    digi_cfg.threshold           = 100 * dd4hep::eV;
    digi_cfg.timeResolution      = 5;
    digi_cfg.gain                = 10000;
    digi_cfg.stripResolutions[0] = digi_cfg.stripResolutions[1] = 150 * dd4hep::um;
    // Get #channels from XML
    std::string constantName = std::string("MPGDOuterBarrelnStrips");
    try {
      auto detector            = app->GetService<DD4hep_service>()->detector();
      digi_cfg.stripNumbers[0] = digi_cfg.stripNumbers[1] = detector->constant<int>(constantName);
    } catch (...) {
      throw JException(R"(MPGD "%s": Error retrieving #channels from XML: no "%s" constant found.)",
                       digi_cfg.readout.c_str(), constantName.c_str());
    }
    app->Add(new JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>(
        "OuterMPGDBarrelRawHits", {"EventHeader", "OuterMPGDBarrelHits"},
        {"OuterMPGDBarrelRawHits",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
         "OuterMPGDBarrelRawHitLinks",
#endif
	 "OuterMPGDBarrelRawHitAssociations"}, digi_cfg, app));
  }

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  if ((SiFactoryPattern & 0x2) != 0U) {
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "OuterMPGDBarrelRecHits", {"OuterMPGDBarrelRawHits"}, // Input data collection tags
        {"OuterMPGDBarrelRecHits"},                           // Output data tag
        {
            .timeResolution = 10,
        },
        app));
  } else {
    MPGDHitReconstructionConfig digi_cfg;
    digi_cfg.readout             = "OuterMPGDBarrelHits";
    digi_cfg.timeResolution      = 10;
    digi_cfg.stripResolutions[0] = digi_cfg.stripResolutions[1] = 150 * dd4hep::um;
    app->Add(new JOmniFactoryGeneratorT<MPGDHitReconstruction_factory>(
        "OuterMPGDBarrelRecHits", {"OuterMPGDBarrelRawHits"}, // Input data collection tags
        {"OuterMPGDBarrelRecHits"},                           // Output data tag
        digi_cfg, app));
  }

  // ***** "BackwardMPGDEndcap"
  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "BackwardMPGDEndcapRawHits", {"EventHeader", "BackwardMPGDEndcapHits"},
      {"BackwardMPGDEndcapRawHits",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "BackwardMPGDEndcapRawHitLinks",
#endif
       "BackwardMPGDEndcapRawHitAssociations"},
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
      {"ForwardMPGDEndcapRawHits",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "ForwardMPGDEndcapRawHitLinks",
#endif
       "ForwardMPGDEndcapRawHitAssociations"},
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
