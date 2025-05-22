// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// kuma edit

#include "addDummyEntryFactory.h"
#include "addDummyEntryProcesser.h"

#include "TimeframeSplitter.h"

#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplication.h>

extern "C" {
void InitPlugin(JApplication* app) {

    InitJANAPlugin(app);

    app->Add(new addDummyEntryProcesser());

    // app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
    //   { .tag = "addDummyEntryFactorySiBarrelHits",
    //     .input_names = {"SiBarrelHits"},
    //     .output_names = {"SiBarrelHitsWDummy"}
    //   }));

      // Tracker hits
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryB0ECalHits",
        .input_names = {"B0ECalHits"},
        .output_names = {"B0ECalHitsWDummy"}
      }));

    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryB0TrackerHits",
        .input_names = {"B0TrackerHits"},
        .output_names = {"B0TrackerHitsWDummy"}
      }));

    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryBackwardMPGDEndcapHits",
        .input_names = {"BackwardMPGDEndcapHits"},
        .output_names = {"BackwardMPGDEndcapHitsWDummy"}
      }));

    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryDIRCBarHits",
        .input_names = {"DIRCBarHits"},
        .output_names = {"DIRCBarHitsWDummy"}
      }));

    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryDRICHHits",
        .input_names = {"DRICHHits"},
        .output_names = {"DRICHHitsWDummy"}
      }));

    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryForwardMPGDEndcapHits",
        .input_names = {"ForwardMPGDEndcapHits"},
        .output_names = {"ForwardMPGDEndcapHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryForwardOffMTrackerHits",
        .input_names = {"ForwardOffMTrackerHits"},
        .output_names = {"ForwardOffMTrackerHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryForwardRomanPotHits",
        .input_names = {"ForwardRomanPotHits"},
        .output_names = {"ForwardRomanPotHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryLumiSpecTrackerHits",
        .input_names = {"LumiSpecTrackerHits"},
        .output_names = {"LumiSpecTrackerHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryMPGDBarrelHits",
        .input_names = {"MPGDBarrelHits"},
        .output_names = {"MPGDBarrelHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryOuterMPGDBarrelHits",
        .input_names = {"OuterMPGDBarrelHits"},
        .output_names = {"OuterMPGDBarrelHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryRICHEndcapNHits",
        .input_names = {"RICHEndcapNHits"},
        .output_names = {"RICHEndcapNHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactorySiBarrelHits",
        .input_names = {"SiBarrelHits"},
        .output_names = {"SiBarrelHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryTOFBarrelHits",
        .input_names = {"TOFBarrelHits"},
        .output_names = {"TOFBarrelHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryTOFEndcapHits",
        .input_names = {"TOFEndcapHits"},
        .output_names = {"TOFEndcapHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryTaggerTrackerHits",
        .input_names = {"TaggerTrackerHits"},
        .output_names = {"TaggerTrackerHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryTrackerEndcapHits",
        .input_names = {"TrackerEndcapHits"},
        .output_names = {"TrackerEndcapHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryVertexBarrelHits",
        .input_names = {"VertexBarrelHits"},
        .output_names = {"VertexBarrelHitsWDummy"}
      }));

    // Calorimeter hits
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryB0ECalHits",
        .input_names = {"B0ECalHits"},
        .output_names = {"B0ECalHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalBarrelImagingHits",
        .input_names = {"EcalBarrelImagingHits"},
        .output_names = {"EcalBarrelImagingHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalBarrelScFiHits",
        .input_names = {"EcalBarrelScFiHits"},
        .output_names = {"EcalBarrelScFiHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalEndcapNHits",
        .input_names = {"EcalEndcapNHits"},
        .output_names = {"EcalEndcapNHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalEndcapPHits",
        .input_names = {"EcalEndcapPHits"},
        .output_names = {"EcalEndcapPHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalEndcapPInsertHits",
        .input_names = {"EcalEndcapPInsertHits"},
        .output_names = {"EcalEndcapPInsertHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalFarForwardZDCHits",
        .input_names = {"EcalFarForwardZDCHits"},
        .output_names = {"EcalFarForwardZDCHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalLumiSpecHits",
        .input_names = {"EcalLumiSpecHits"},
        .output_names = {"EcalLumiSpecHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryHcalBarrelHits",
        .input_names = {"HcalBarrelHits"},
        .output_names = {"HcalBarrelHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryHcalEndcapNHits",
        .input_names = {"HcalEndcapNHits"},
        .output_names = {"HcalEndcapNHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryHcalEndcapPInsertHits",
        .input_names = {"HcalEndcapPInsertHits"},
        .output_names = {"HcalEndcapPInsertHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryHcalFarForwardZDCHits",
        .input_names = {"HcalFarForwardZDCHits"},
        .output_names = {"HcalFarForwardZDCHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryLFHCALHits",
        .input_names = {"LFHCALHits"},
        .output_names = {"LFHCALHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryLumiDirectPCALHits",
        .input_names = {"LumiDirectPCALHits"},
        .output_names = {"LumiDirectPCALHitsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalBarrelScFiRawHits",
        .input_names = {"EcalBarrelScFiRawHits"},
        .output_names = {"EcalBarrelScFiRawHitsWDummy"}
      }));

     // "EcalBarrelScFiRawHits",
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryB0ECalHitsContributions",
        .input_names = {"B0ECalHitsContributions"},
        .output_names = {"B0ECalHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalBarrelImagingHitsContributions",
        .input_names = {"EcalBarrelImagingHitsContributions"},
        .output_names = {"EcalBarrelImagingHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalBarrelScFiHitsContributions",
        .input_names = {"EcalBarrelScFiHitsContributions"},
        .output_names = {"EcalBarrelScFiHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalEndcapNHitsContributions",
        .input_names = {"EcalEndcapNHitsContributions"},
        .output_names = {"EcalEndcapNHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalEndcapPHitsContributions",
        .input_names = {"EcalEndcapPHitsContributions"},
        .output_names = {"EcalEndcapPHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalEndcapPInsertHitsContributions",
        .input_names = {"EcalEndcapPInsertHitsContributions"},
        .output_names = {"EcalEndcapPInsertHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalLumiSpecHitsContributions",
        .input_names = {"EcalLumiSpecHitsContributions"},
        .output_names = {"EcalLumiSpecHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryEcalFarForwardZDCHitsContributions",
        .input_names = {"EcalFarForwardZDCHitsContributions"},
        .output_names = {"EcalFarForwardZDCHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryHcalBarrelHitsContributions",
        .input_names = {"HcalBarrelHitsContributions"},
        .output_names = {"HcalBarrelHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryHcalEndcapNHitsContributions",
        .input_names = {"HcalEndcapNHitsContributions"},
        .output_names = {"HcalEndcapNHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryHcalEndcapPInsertHitsContributions",
        .input_names = {"HcalEndcapPInsertHitsContributions"},
        .output_names = {"HcalEndcapPInsertHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryHcalFarForwardZDCHitsContributions",
        .input_names = {"HcalFarForwardZDCHitsContributions"},
        .output_names = {"HcalFarForwardZDCHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryLFHCALHitsContributions",
        .input_names = {"LFHCALHitsContributions"},
        .output_names = {"LFHCALHitsContributionsWDummy"}
      }));
    app->Add(new JOmniFactoryGeneratorT<addDummyEntryFactory>(
      { .tag = "addDummyEntryFactoryLumiDirectPCALHitsContributions",
        .input_names = {"LumiDirectPCALHitsContributions"},
        .output_names = {"LumiDirectPCALHitsContributionsWDummy"}
      }));

}
} // "C"
