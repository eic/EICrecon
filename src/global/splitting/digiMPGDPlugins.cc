// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JException.h>
#include <stdexcept>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/MPGDTrackerDigi_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

// 2D-STRIP DIGITIZATION = DEFAULT
// - Is produced by "MPGDTrackerDigi".
// - Relies on "MultiSegmentation" <readout> in "compact" geometry file.
// PIXEL DIGITIZATION = BROUGHT INTO PLAY BY OPTION "MPGD:SiFactoryPattern".
// - Is produced by "SiliconTrackerDigi".

// extern "C" {
void InitPlugin_digiMPGD(JApplication* app) {
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
        JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>::TypedWiring{
            .m_tag                 = "MPGDBarrelRawHits_TK",
            .m_default_input_tags  = {"MPGDBarrelHits"},
            .m_default_output_tags = {"MPGDBarrelRawHits_TK", "MPGDBarrelRawHitAssociations_TK"},
            .m_default_cfg =
                {
                    .threshold      = 100 * dd4hep::eV,
                    .timeResolution = 10,
                },
            .level = JEventLevel::Timeslice},
        app));
  } else {
    app->Add(new JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>(
        JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>::TypedWiring{
            .m_tag                 = "MPGDBarrelRawHits_TK",
            .m_default_input_tags  = {"MPGDBarrelHits"},
            .m_default_output_tags = {"MPGDBarrelRawHits_TK", "MPGDBarrelRawHitAssociations_TK"},
            .m_default_cfg =
                {
                    .readout        = "MPGDBarrelHits",
                    .threshold      = 100 * dd4hep::eV,
                    .timeResolution = 10,
                },
            .level = JEventLevel::Timeslice},
        app));
  }

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>::TypedWiring{
          .m_tag                 = "MPGDBarrelRecHits_TK",
          .m_default_input_tags  = {"MPGDBarrelRawHits_TK"},
          .m_default_output_tags = {"MPGDBarrelRecHits_TK"},
          .m_default_cfg =
              {
                  .timeResolution = 10,
              },
          .level = JEventLevel::Timeslice},
      app));

  // ***** OuterMPGDBarrel
  // Digitization
  if ((SiFactoryPattern & 0x2) != 0U) {
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>::TypedWiring{
            .m_tag                 = "OuterMPGDBarrelRawHits_TK",
            .m_default_input_tags  = {"OuterMPGDBarrelHits"},
            .m_default_output_tags = {"OuterMPGDBarrelRawHits_TK",
                                      "OuterMPGDBarrelRawHitAssociations_TK"},
            .m_default_cfg =
                {
                    .threshold      = 100 * dd4hep::eV,
                    .timeResolution = 10,
                },
            .level = JEventLevel::Timeslice},
        app));
  } else {
    app->Add(new JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>(
        JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>::TypedWiring{
            .m_tag                 = "OuterMPGDBarrelRawHits_TK",
            .m_default_input_tags  = {"OuterMPGDBarrelHits"},
            .m_default_output_tags = {"OuterMPGDBarrelRawHits_TK",
                                      "OuterMPGDBarrelRawHitAssociations_TK"},
            .m_default_cfg =
                {
                    .readout        = "OuterMPGDBarrelHits",
                    .threshold      = 100 * dd4hep::eV,
                    .timeResolution = 10,
                },
            .level = JEventLevel::Timeslice},
        app));
  }

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>::TypedWiring{
          .m_tag                 = "OuterMPGDBarrelRecHits_TK",
          .m_default_input_tags  = {"OuterMPGDBarrelRawHits_TK"},
          .m_default_output_tags = {"OuterMPGDBarrelRecHits_TK"},
          .m_default_cfg =
              {
                  .timeResolution = 10,
              },
          .level = JEventLevel::Timeslice},
      app));

  // ***** "BackwardMPGDEndcap"
  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>::TypedWiring{
          .m_tag                 = "BackwardMPGDEndcapRawHits_TK",
          .m_default_input_tags  = {"BackwardMPGDEndcapHits"},
          .m_default_output_tags = {"BackwardMPGDEndcapRawHits_TK",
                                    "BackwardMPGDEndcapRawHitAssociations_TK"},
          .m_default_cfg =
              {
                  .threshold      = 100 * dd4hep::eV,
                  .timeResolution = 10,
              },
          .level = JEventLevel::Timeslice},
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>::TypedWiring{
          .m_tag                 = "BackwardMPGDEndcapRecHits_TK",
          .m_default_input_tags  = {"BackwardMPGDEndcapRawHits_TK"},
          .m_default_output_tags = {"BackwardMPGDEndcapRecHits_TK"},
          .m_default_cfg =
              {
                  .timeResolution = 10,
              },
          .level = JEventLevel::Timeslice},
      app));

  // ""ForwardMPGDEndcap"
  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>::TypedWiring{
          .m_tag                 = "ForwardMPGDEndcapRawHits_TK",
          .m_default_input_tags  = {"ForwardMPGDEndcapHits"},
          .m_default_output_tags = {"ForwardMPGDEndcapRawHits_TK",
                                    "ForwardMPGDEndcapRawHitAssociations_TK"},
          .m_default_cfg =
              {
                  .threshold      = 100 * dd4hep::eV,
                  .timeResolution = 10,
              },
          .level = JEventLevel::Timeslice},
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>::TypedWiring{
          .m_tag                 = "ForwardMPGDEndcapRecHits_TK",
          .m_default_input_tags  = {"ForwardMPGDEndcapRawHits_TK"},
          .m_default_output_tags = {"ForwardMPGDEndcapRecHits_TK"},
          .m_default_cfg =
              {
                  .timeResolution = 10,
              },
          .level = JEventLevel::Timeslice},
      app));
}
// } // extern "C"
