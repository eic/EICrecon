// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// kuma edit

// #include "HitChecker.h"

#include "timeAlignmentFactory.h"
#include "TimeframeSplitter.h"
#include "TimeCoincidenceFactory.h"

#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplication.h>

extern "C" {
void InitPlugin(JApplication* app) {

  // This is the plugin initialization function that JANA will call.
  std::vector<std::string> m_simtrackerhit_collection_names_aligned = {
      "B0TrackerHits_aligned",         "BackwardMPGDEndcapHits_aligned",
      "DIRCBarHits_aligned",           "DRICHHits_aligned",
      "ForwardMPGDEndcapHits_aligned", "ForwardOffMTrackerHits_aligned",
      "ForwardRomanPotHits_aligned",   "LumiSpecTrackerHits_aligned",
      "MPGDBarrelHits_aligned",        "OuterMPGDBarrelHits_aligned",
      "RICHEndcapNHits_aligned",       "SiBarrelHits_aligned",
      "TOFBarrelHits_aligned",         "TOFEndcapHits_aligned",
      "TaggerTrackerHits_aligned",     "TrackerEndcapHits_aligned",
      "VertexBarrelHits_aligned"};

  std::vector<std::vector<std::string>> m_simtrackerhit_collection_names = {
      {"B0TrackerHits", "BackwardMPGDEndcapHits", "DIRCBarHits", "DRICHHits",
       "ForwardMPGDEndcapHits", "ForwardOffMTrackerHits", "ForwardRomanPotHits",
       "LumiSpecTrackerHits", "MPGDBarrelHits", "OuterMPGDBarrelHits", "RICHEndcapNHits",
       "SiBarrelHits", "TOFBarrelHits", "TOFEndcapHits", "TaggerTrackerHits", "TrackerEndcapHits",
       "VertexBarrelHits"}};

  InitJANAPlugin(app);

  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"VertexBarrelHits"},
  //     .output_names = {"VertexBarrelHits_aligned"}
  //   }));

  app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
      jana::components::JOmniFactoryGeneratorT<timeAlignmentFactory>::TypedWiring{
          .tag                  = "timeAlignment",
          .level                = JEventLevel::Timeslice,
          .variadic_input_names = m_simtrackerhit_collection_names,
          .output_names         = m_simtrackerhit_collection_names_aligned}));

  // Unfolder that takes timeframes and splits them into physics events.
  app->Add(new TimeframeSplitter());


  app->Add(new JOmniFactoryGeneratorT<TimeCoincidenceFactory>(
      jana::components::JOmniFactoryGeneratorT<TimeCoincidenceFactory>::TypedWiring{
          .tag                  = "timeCoincidence",
          .level                = JEventLevel::PhysicsEvent,
          .variadic_input_names = m_simtrackerhit_collection_names,
          .output_names         = {"triggerflag"}
        }
      ));


  // app->Add(new JOmniFactoryGeneratorT<HitChecker>(jana::components::JOmniFactoryGeneratorT<HitChecker>::TypedWiring
  //   {.tag          = "timeslice_hit_checker",
  //    .level        = JEventLevel::Timeslice,
  //    .input_names  = {"SiBarrelHits"},
  //    .output_names = {"ts_checked_hits"}}));

  // app->Add(new JOmniFactoryGeneratorT<HitChecker>(jana::components::JOmniFactoryGeneratorT<HitChecker>::TypedWiring
  //   {.tag          = "physics_hit_checker",
  //    .level        = JEventLevel::PhysicsEvent,
  //    .input_names  = {"SiBarrelHits"},
  //    .output_names = {"phys_checked_hits"}}));

}
} // "C"
