// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025, Dmitry Romanov,  Wouter Deconinck, Kolja Kauder, Barak Schmookler, Honey Khindri, Dmitry Kalinkin

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <edm4eic/EDM4eicVersion.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TMath.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/SimTrackerHit.h>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/EICROCDigitization_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

// extern "C" {
void InitPlugin_digiECTOF(JApplication* app) {
  InitJANAPlugin(app);
  using namespace eicrecon;
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>::TypedWiring{
          .m_tag = "TOFEndcapRawHits_TK",
          .m_default_input_tags = {"EventHeader", "TOFEndcapHits"},
          .m_default_output_tags = {"TOFEndcapRawHits_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "TOFEndcapRawHitLinks_TK",
#endif
                                    "TOFEndcapRawHitAssociations_TK"},
          .m_default_cfg = {
              .threshold = 6.0 * dd4hep::keV,
              .timeResolution = 0.025,
          },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>::TypedWiring{
          .m_tag = "TOFEndcapRecHits_TK",
          .m_default_input_tags = {"TOFEndcapRawHits_TK"},
          .m_default_output_tags = {"TOFEndcapRecHits_TK"},
          .m_default_cfg = {
              .timeResolution = 0.025,
          },
          .level = JEventLevel::Timeslice},
      app));
}
// } // extern "C"


