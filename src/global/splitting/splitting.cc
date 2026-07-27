// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// kuma edit

#include <JANA/JApplication.h>
#include "extensions/jana/JOmniFactoryGeneratorT.h"

#include "HitChecker.h"
#include "TrkTimeAlignmentFactory.h"
#include "CalRecTimeAlignmentFactory.h"
#include "CalTimeAlignmentFactory.h"
#include "TimeframeSplitter.h"

extern "C" {
void InitPlugin(JApplication* app) {

  std::vector<std::string> m_simtrackerhit_collection_names_aligned = {
      "TOFBarrelRecHits_aligned",
      "TOFEndcapRecHits_aligned",
      "MPGDBarrelRecHits_aligned",
      "OuterMPGDBarrelRecHits_aligned",
      "BackwardMPGDEndcapRecHits_aligned",
      "ForwardMPGDEndcapRecHits_aligned",
      "SiBarrelVertexRecHits_aligned",
      "SiBarrelTrackerRecHits_aligned",
      "SiEndcapTrackerRecHits_aligned",
      "B0TrackerRecHits_aligned",
      "TaggerTrackerRecHits_aligned",
      "ForwardRomanPotRecHits_aligned",
    "ForwardOffMTrackerRecHits_aligned"
    };
    //   "RICHEndcapNRecHits_TK_aligned"
    // "DIRCBarRecHits_TK_aligned",
    //   "DRICHRecHits_TK_aligned",

  std::vector<std::string> m_simtrackerhit_collection_names = {
      "TOFBarrelRecHits",
      "TOFEndcapRecHits",
      "MPGDBarrelRecHits",
      "OuterMPGDBarrelRecHits",
      "BackwardMPGDEndcapRecHits",
      "ForwardMPGDEndcapRecHits",
      "SiBarrelVertexRecHits",
      "SiBarrelTrackerRecHits",
      "SiEndcapTrackerRecHits",
      "B0TrackerRecHits",
      "TaggerTrackerRecHits",
      "ForwardRomanPotRecHits",
      "ForwardOffMTrackerRecHits"
    };
    // "RICHEndcapNRecHits_TK"
    // "DIRCBarRecHits_TK",
    // "DRICHRecHits_TK"
    

    std::vector<std::string> m_simcalorechit_collection_names = {
      "B0ECalRecHits",
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
      "LFHCALRecHits"
    };

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
      "LFHCALRecHits_aligned"
    };


  std::vector<std::string> m_simcalocluster_collection_names_aligned = {
      "B0ECalClusters_TK_aligned",
      "EcalBarrelClusters_TK_aligned",
      "EcalEndcapNClusters_TK_aligned",
      "EcalEndcapPClusters_TK_aligned"
    };
    // "EcalFarForwardZDCClusters_TK_aligned",
    //   "EcalLumiSpecClusters_TK_aligned",
    //   "HcalBarrelClusters_TK_aligned",
    //   "HcalEndcapNClusters_TK_aligned",
    //   "HcalEndcapPInsertClusters_TK_aligned",
    //   "HcalFarForwardZDCClusters_TK_aligned",
    //   "LFHCALClusters_TK_aligned"

  std::vector<std::string> m_simcalocluster_collection_names = {
    "B0ECalClusters_TK",
    "EcalBarrelClusters_TK",
    "EcalEndcapNClusters_TK",
    "EcalEndcapPClusters_TK"
    };

  InitJANAPlugin(app);

  app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
      JOmniFactoryGeneratorT<timeAlignmentFactory>::TypedWiring{
          .m_tag                 = "timeAlignment",
          .m_default_input_tags  = m_simtrackerhit_collection_names,
          .m_default_output_tags = m_simtrackerhit_collection_names_aligned,
          .level                 = JEventLevel::Timeslice,
      },
      app));

    app->Add(new JOmniFactoryGeneratorT<CalRecTimeAlignmentFactory>(
      JOmniFactoryGeneratorT<CalRecTimeAlignmentFactory>::TypedWiring{
          .m_tag                 = "CalRecTimeAlignment",
          .m_default_input_tags  = m_simcalorechit_collection_names,
          .m_default_output_tags = m_simcalorechit_collection_names_aligned,
          .level                 = JEventLevel::Timeslice,
      },
      app));

    app->Add(new JOmniFactoryGeneratorT<CalTimeAlignmentFactory>(
      JOmniFactoryGeneratorT<CalTimeAlignmentFactory>::TypedWiring{
          .m_tag                 = "CalTimeAlignment",
          .m_default_input_tags  = m_simcalocluster_collection_names,
          .m_default_output_tags = m_simcalocluster_collection_names_aligned,
          .level                 = JEventLevel::Timeslice,
      },
      app));

  // Unfolder that takes timeframes and splits them into physics events.
  app->Add(new TimeframeSplitter());

  app->Add(new JOmniFactoryGeneratorT<HitChecker>(
      JOmniFactoryGeneratorT<HitChecker>::TypedWiring{
          .m_tag                 = "timeframe_hit_checker",
          .m_default_input_tags  = {"TOFBarrelRecHits"},
          .m_default_output_tags = {"hitChecker_TF"},
          .level                 = JEventLevel::Timeslice,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<HitChecker>(
      JOmniFactoryGeneratorT<HitChecker>::TypedWiring{
          .m_tag                 = "timeslice_hit_checker",
          .m_default_input_tags  = {"TOFBarrelRecHits"},
          .m_default_output_tags = {"hitChecker_TS"},
          .level                 = JEventLevel::PhysicsEvent,
      },
      app));

}
} // "C"
