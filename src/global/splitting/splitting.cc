// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// kuma edit

#include "HitChecker.h"

#include "timeAlignmentFactory.h"
#include "TimeframeSplitter.h"

#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplication.h>

extern "C" {
void InitPlugin(JApplication* app) {

  InitJANAPlugin(app);

  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"B0TrackerHits"},
  //     .output_names = {"B0TrackerHits_aligned"}
  //   }));

  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"BackwardMPGDEndcapHits"},
  //     .output_names = {"BackwardMPGDEndcapHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"DIRCBarHits"},
  //     .output_names = {"DIRCBarHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"DRICHHits"},
  //     .output_names = {"DRICHHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"ForwardMPGDEndcapHits"},
  //     .output_names = {"ForwardMPGDEndcapHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"ForwardOffMTrackerHits"},
  //     .output_names = {"ForwardOffMTrackerHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"ForwardRomanPotHits"},
  //     .output_names = {"ForwardRomanPotHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"LumiSpecTrackerHits"},
  //     .output_names = {"LumiSpecTrackerHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"MPGDBarrelHits"},
  //     .output_names = {"MPGDBarrelHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"OuterMPGDBarrelHits"},
  //     .output_names = {"OuterMPGDBarrelHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"RICHEndcapNHits"},
  //     .output_names = {"RICHEndcapNHits_aligned"}
  //   }));
  app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
    { .tag = "timeAlignment",
      .level = JEventLevel::Timeslice,
      .input_names = {"SiBarrelHits"},
      .output_names = {"SiBarrelHits_aligned"}
    }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"TOFBarrelHits"},
  //     .output_names = {"TOFBarrelHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"TOFEndcapHits"},
  //     .output_names = {"TOFEndcapHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"TaggerTrackerHits"},
  //     .output_names = {"TaggerTrackerHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"TrackerEndcapHits"},
  //     .output_names = {"TrackerEndcapHits_aligned"}
  //   }));
  // app->Add(new JOmniFactoryGeneratorT<timeAlignmentFactory>(
  //   { .tag = "timeAlignment",
  //     .level = JEventLevel::Timeslice,
  //     .input_names = {"VertexBarrelHits"},
  //     .output_names = {"VertexBarrelHits_aligned"}
  //   }));


  // Unfolder that takes timeframes and splits them into physics events.
  app->Add(new TimeframeSplitter());

  app->Add(new JOmniFactoryGeneratorT<HitChecker>(
    { .tag = "timeslice_hit_checker",
      .level = JEventLevel::Timeslice,
      .input_names = {"SiBarrelHits"},
      .output_names = {"ts_checked_hits"}
    }));

  app->Add(new JOmniFactoryGeneratorT<HitChecker>(
    { .tag = "physics_hit_checker",
      .level = JEventLevel::PhysicsEvent,
      .input_names = {"SiBarrelHits"},
      .output_names = {"phys_checked_hits"}
    }));

  // Factory that produces timeslice-level protoclusters from timeslice-level hits
  /*
    app->Add(new JOmniFactoryGeneratorT<MyProtoclusterFactory>(
                { .tag = "timeslice_protoclusterizer",
                  .level = JEventLevel::Timeslice,
                  .input_names = {"hits"},
                  .output_names = {"ts_protoclusters"}
                }));
    */

  }
} // "C"
