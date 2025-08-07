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
#include "JANA/Components/JOmniFactoryGeneratorT.h"
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
        {.tag          = "MPGDBarrelRawHits_TK",
         .level        = JEventLevel::Timeslice,
         .input_names  = {"EventHeader", "MPGDBarrelHits"},
         .output_names = {"MPGDBarrelRawHits_TK", "MPGDBarrelRawHitAssociations_TK"},
         .configs      = {
                  .threshold      = 100 * dd4hep::eV,
                  .timeResolution = 10,
         }}));
  } else {
    app->Add(new JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>(
        {.tag          = "MPGDBarrelRawHits_TK",
         .level        = JEventLevel::Timeslice,
         .input_names  = {"MPGDBarrelHits"},
         .output_names = {"MPGDBarrelRawHits_TK", "MPGDBarrelRawHitAssociations_TK"},
         .configs      = {
                  .readout        = "MPGDBarrelHits",
                  .threshold      = 100 * dd4hep::eV,
                  .timeResolution = 10,
         }}));
  }

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      {.tag          = "MPGDBarrelRecHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"MPGDBarrelRawHits_TK"},
       .output_names = {"MPGDBarrelRecHits_TK"},
       .configs      = {
                .timeResolution = 10,
       }}));

  // ***** OuterMPGDBarrel
  // Digitization
  if ((SiFactoryPattern & 0x2) != 0U) {
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        {.tag          = "OuterMPGDBarrelRawHits_TK",
         .level        = JEventLevel::Timeslice,
         .input_names  = {"EventHeader", "OuterMPGDBarrelHits"},
         .output_names = {"OuterMPGDBarrelRawHits_TK", "OuterMPGDBarrelRawHitAssociations_TK"},
         .configs      = {
                  .threshold      = 100 * dd4hep::eV,
                  .timeResolution = 10,
         }}));
  } else {
    app->Add(new JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>(
        {.tag          = "OuterMPGDBarrelRawHits_TK",
         .level        = JEventLevel::Timeslice,
         .input_names  = {"OuterMPGDBarrelHits"},
         .output_names = {"OuterMPGDBarrelRawHits_TK", "OuterMPGDBarrelRawHitAssociations_TK"},
         .configs      = {
                  .readout        = "OuterMPGDBarrelHits",
                  .threshold      = 100 * dd4hep::eV,
                  .timeResolution = 10,
         }}));
  }

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      {.tag          = "OuterMPGDBarrelRecHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"OuterMPGDBarrelRawHits_TK"},
       .output_names = {"OuterMPGDBarrelRecHits_TK"},
       .configs      = {
                .timeResolution = 10,
       }}));

  // ***** "BackwardMPGDEndcap"
  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>({
      .tag          = "BackwardMPGDEndcapRawHits_TK",
      .level        = JEventLevel::Timeslice,
      .input_names  = {"EventHeader", "BackwardMPGDEndcapHits"},
      .output_names = {"BackwardMPGDEndcapRawHits_TK", "BackwardMPGDEndcapRawHitAssociations_TK"},
      .configs =
          {
              .threshold      = 100 * dd4hep::eV,
              .timeResolution = 10,
          },
  }));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      {.tag          = "BackwardMPGDEndcapRecHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"BackwardMPGDEndcapRawHits_TK"},
       .output_names = {"BackwardMPGDEndcapRecHits_TK"},
       .configs      = {
                .timeResolution = 10,
       }}));

  // ""ForwardMPGDEndcap"
  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      {.tag          = "ForwardMPGDEndcapRawHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"EventHeader", "ForwardMPGDEndcapHits"},
       .output_names = {"ForwardMPGDEndcapRawHits_TK", "ForwardMPGDEndcapRawHitAssociations_TK"},
       .configs      = {
                .threshold      = 100 * dd4hep::eV,
                .timeResolution = 10,
       }}));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>({
      .tag          = "ForwardMPGDEndcapRecHits_TK",
      .level        = JEventLevel::Timeslice,
      .input_names  = {"ForwardMPGDEndcapRawHits_TK"},
      .output_names = {"ForwardMPGDEndcapRecHits_TK"},
      .configs =
          {
              .timeResolution = 10,
          },
  }));
}
// } // extern "C"
