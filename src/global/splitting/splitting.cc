// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// kuma edit

#include <JANA/JApplication.h>
#include "extensions/jana/JOmniFactoryGeneratorT.h"

#include "HitChecker.h"
#include "timeAlignmentFactory.h"
#include "TimeframeSplitter.h"
// #include "TimeCoincidenceFactory.h"


void InitPlugin_digiBTOF(JApplication* app);
void InitPlugin_digiMPGD(JApplication* app);
void InitPlugin_digiBVTX(JApplication* app);
void InitPlugin_digiBTRK(JApplication* app);
void InitPlugin_digiECTRK(JApplication* app);
void InitPlugin_digiECTOF(JApplication* app);
void InitPlugin_digiB0TRK(JApplication* app);
// void InitPlugin_digiDIRC(JApplication* app);
// void InitPlugin_digiDRICH(JApplication* app);
void InitPlugin_digiFOFFMTRK(JApplication* app);

extern "C" {
void InitPlugin(JApplication* app) {

  // if (!app->RegisterParameter<bool>("split_timeframes", false, "Enable timeframe splitting")) {
  //   return;
  // }

  // This is the plugin initialization function that JANA will call.
  // std::vector<std::string> m_simtrackerhit_collection_names_aligned = {
  //     "B0TrackerHits_aligned",         "BackwardMPGDEndcapHits_aligned",
  //     "DIRCBarHits_aligned",           "DRICHHits_aligned",
  //     "ForwardMPGDEndcapHits_aligned", "ForwardOffMTrackerHits_aligned",
  //     "ForwardRomanPotHits_aligned",   "LumiSpecTrackerHits_aligned",
  //     "MPGDBarrelHits_aligned",        "OuterMPGDBarrelHits_aligned",
  //     "RICHEndcapNHits_aligned",       "SiBarrelHits_aligned",
  //     "TOFBarrelHits_aligned",         "TOFEndcapHits_aligned",
  //     "TaggerTrackerHits_aligned",     "TrackerEndcapHits_aligned",
  //     "VertexBarrelHits_aligned"};

  std::vector<std::string> m_simtrackerhit_collection_names_aligned = 
      {"TOFBarrelRecHits_TK_aligned", "TOFEndcapRecHits_TK_aligned", "MPGDBarrelRecHits_TK_aligned",
       "OuterMPGDBarrelRecHits_TK_aligned", "BackwardMPGDEndcapRecHits_TK_aligned",
       "ForwardMPGDEndcapRecHits_TK_aligned", "SiBarrelVertexRecHits_TK_aligned",
       "SiBarrelTrackerRecHits_TK_aligned", "SiEndcapTrackerRecHits_TK_aligned",
       "TaggerTrackerRecHits_TK_aligned", "B0TrackerRecHits_TK_aligned",
       "DIRCBarRecHits_TK_aligned", "DRICHRecHits_TK_aligned",
       "ForwardOffMTrackerRecHits_TK_aligned", "ForwardRomanPotRecHits_TK_aligned",
       "LumiSpecTrackerRecHits_TK_aligned", "RICHEndcapNRecHits_TK_aligned"};

  std::vector<std::string> m_simtrackerhit_collection_names =
      {"TOFBarrelRecHits_TK", "TOFEndcapRecHits_TK", "MPGDBarrelRecHits_TK",
       "OuterMPGDBarrelRecHits_TK", "BackwardMPGDEndcapRecHits_TK", "ForwardMPGDEndcapRecHits_TK",
       "SiBarrelVertexRecHits_TK", "SiBarrelTrackerRecHits_TK", "SiEndcapTrackerRecHits_TK",
       "TaggerTrackerRecHits_TK", "B0TrackerRecHits_TK", "DIRCBarRecHits_TK", "DRICHRecHits_TK",
       "ForwardOffMTrackerRecHits_TK", "ForwardRomanPotRecHits_TK", "LumiSpecTrackerRecHits_TK",
       "RICHEndcapNRecHits_TK"};

  InitJANAPlugin(app);

  app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
      JOmniFactoryGeneratorT<timeAlignmentFactory>::TypedWiring {
          .m_tag                   = "timeAlignment",
          .m_default_input_tags  = m_simtrackerhit_collection_names,
          .m_default_output_tags = m_simtrackerhit_collection_names_aligned,
          .level                 = JEventLevel::Timeslice,
        }, app));

  // Unfolder that takes timeframes and splits them into physics events.
  app->Add(new TimeframeSplitter());

  // app->Add(new JOmniFactoryGeneratorT<HitChecker>(
  //     jana::components::JOmniFactoryGeneratorT<HitChecker>::TypedWiring{
  //         .tag                   = "hitChecker",
  //         .level                 = JEventLevel::Timeslice,
  //         .variadic_input_names  = m_simtrackerhit_collection_names,
  //         .variadic_output_names = m_simtrackerhit_collection_names_aligned}));
  // app->Add(new JOmniFactoryGeneratorT<HitChecker>(jana::components::JOmniFactoryGeneratorT<HitChecker>::TypedWiring
  // {.tag          = "timeslice_hit_checker",
  //  .level        = JEventLevel::PhysicsEvent,
  //  .input_names  = {"TOFBarrelRecHits"},
  //  .output_names = {"hitChecker_TS"}}));

  InitPlugin_digiBTOF(app);
  InitPlugin_digiMPGD(app);
  InitPlugin_digiBVTX(app);
  InitPlugin_digiBTRK(app);
  InitPlugin_digiECTRK(app);
  InitPlugin_digiECTOF(app);
  InitPlugin_digiB0TRK(app);
  // InitPlugin_digiDIRC(app);
  // InitPlugin_digiDRICH(app);
  InitPlugin_digiFOFFMTRK(app);
}
} // "C"
