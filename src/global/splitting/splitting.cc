// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// kuma edit

#include "HitChecker.h"

#include "timeAlignmentFactory.h"
#include "TimeframeSplitter.h"
// #include "TimeCoincidenceFactory.h"

// #include "extensions/jana/JOmniFactoryGeneratorT.h"
// #include "extensions/jana/JOmniFactory.h"
#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplication.h>

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

  std::vector<std::vector<std::string>> m_simtrackerhit_collection_names_aligned = {{
      "B0TrackerRecHits_TK_aligned",         "BackwardMPGDEndcapRecHits_TK_aligned",
      "DIRCBarRecHits_TK_aligned",           "DRICHRecHits_TK_aligned",
      "ForwardMPGDEndcapRecHits_TK_aligned", "ForwardOffMTrackerRecHits_TK_aligned",
      "ForwardRomanPotRecHits_TK_aligned",   "LumiSpecTrackerRecHits_TK_aligned",
      "MPGDBarrelRecHits_TK_aligned",        "OuterMPGDBarrelRecHits_TK_aligned",
      "RICHEndcapNRecHits_TK_aligned",       "SiBarrelTrackerRecHits_TK_aligned",
      "TOFBarrelRecHits_TK_aligned",         "TOFEndcapRecHits_TK_aligned",
      "TaggerTrackerRecHits_TK_aligned",     "SiEndcapTrackerRecHits_TK_aligned",
      "SiBarrelVertexRecHits_TK_aligned"}};

  std::vector<std::vector<std::string>> m_simtrackerhit_collection_names = {
      {"B0TrackerRecHits_TK", "BackwardMPGDEndcapRecHits_TK", "DIRCBarRecHits_TK",
       "DRICHRecHits_TK", "ForwardMPGDEndcapRecHits_TK", "ForwardOffMTrackerRecHits_TK",
       "ForwardRomanPotRecHits_TK", "LumiSpecTrackerRecHits_TK", "MPGDBarrelRecHits_TK",
       "OuterMPGDBarrelRecHits_TK", "RICHEndcapNRecHits_TK", "SiBarrelTrackerRecHits_TK",
       "TOFBarrelRecHits_TK", "TOFEndcapRecHits_TK", "TaggerTrackerRecHits_TK",
       "SiEndcapTrackerRecHits_TK", "SiBarrelVertexRecHits_TK"}};

  InitJANAPlugin(app);

  app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
      jana::components::JOmniFactoryGeneratorT<timeAlignmentFactory>::TypedWiring{
          .tag                  = "timeAlignment",
          .level                = JEventLevel::Timeslice,
          .variadic_input_names = m_simtrackerhit_collection_names,
          .variadic_output_names         = m_simtrackerhit_collection_names_aligned}));

  // Unfolder that takes timeframes and splits them into physics events.
  app->Add(new TimeframeSplitter());

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
