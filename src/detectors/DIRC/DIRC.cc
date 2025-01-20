// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Christopher Dilks, Nilanga Wickramaarachchi, Dmitry Kalinkin

#include <DD4hep/Detector.h>
#include <edm4eic/EDM4eicVersion.h>
#include <JANA/JApplication.h>
#include <algorithm>
#include <gsl/pointers>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

// algorithm configurations
#include "algorithms/digi/PhotoMultiplierHitDigiConfig.h"
#include "algorithms/pid_lut/PIDLookupConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
// factories
#include "global/digi/PhotoMultiplierHitDigi_factory.h"
#if EDM4EIC_VERSION_MAJOR >= 8
#include "global/pid/DIRCParticleIDPreML_factory.h"
#include "global/pid/DIRCParticleIDPostML_factory.h"
#endif
#include "global/pid_lut/PIDLookup_factory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#if EDM4EIC_VERSION_MAJOR >= 8
#include "factories/meta/ONNXInference_factory.h"
#endif


extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // configuration parameters ///////////////////////////////////////////////

    // digitization
    PhotoMultiplierHitDigiConfig digi_cfg;
    digi_cfg.seed            = 5; // FIXME: set to 0 for a 'unique' seed, but
                                  // that seems to delay the RNG from actually randomizing
    digi_cfg.hitTimeWindow   = 20.0; // [ns]
    digi_cfg.timeResolution  = 1/16.0; // [ns]
    digi_cfg.speMean         = 80.0;
    digi_cfg.speError        = 16.0;
    digi_cfg.pedMean         = 200.0;
    digi_cfg.pedError        = 3.0;
    digi_cfg.enablePixelGaps = false;
    digi_cfg.safetyFactor    = 1.0;
    // Actual efficiency is implemented in npsim via a stacking action
    digi_cfg.enableQuantumEfficiency = false;
    digi_cfg.quantumEfficiency.clear();

    // digitization
    app->Add(new JOmniFactoryGeneratorT<PhotoMultiplierHitDigi_factory>(
          "DIRCRawHits",
          {"DIRCBarHits"},
          {"DIRCRawHits", "DIRCRawHitsAssociations"},
          digi_cfg,
          app
          ));

    app->Add(new JOmniFactoryGeneratorT<DIRCParticleIDPreML_factory>(
        "DIRCBarrelParticleIDPreML",
        {
          "DIRCRawHits",
          "ReconstructedChargedWithoutPIDParticles",
          "ReconstructedChargedWithoutPIDParticleAssociations",
        },
        {
          "DIRCBarrelParticleIDDIRCInput_features",
          "DIRCBarrelParticleIDTrackInput_features",
          "DIRCBarrelParticleIDPIDTarget",
        },
        {},
        app
        ));
    app->Add(new JOmniFactoryGeneratorT<ONNXInference_factory>(
        "DIRCBarrelParticleIDInference",
        {
          "DIRCBarrelParticleIDTrackInput_features",
          "DIRCBarrelParticleIDDIRCInput_features",
        },
        {
          "DIRCBarrelParticleIDOutput_probability_tensor",
        },
        {
          .modelPath = "calibrations/onnx/DIRCBarrel.onnx",
        },
        app
    ));
    app->Add(new JOmniFactoryGeneratorT<DIRCParticleIDPostML_factory>(
        "DIRCBarrelParticleIDPostML",
        {
          "ReconstructedChargedWithoutPIDParticles",
          "ReconstructedChargedWithoutPIDParticleAssociations",
          "DIRCBarrelParticleIDOutput_probability_tensor",
        },
        {
          "ReconstructedChargedWithRealDIRCParticles",
          "ReconstructedChargedWithRealDIRCParticleAssociations",
          "DIRCBarrelParticleIDs",
        },
        app
    ));

    int BarrelDIRC_ID = 0;
    try {
        auto detector = app->GetService<DD4hep_service>()->detector();
        BarrelDIRC_ID = detector->constant<int>("BarrelDIRC_ID");
    } catch(const std::runtime_error&) {
        // Nothing
    }
    PIDLookupConfig pid_cfg {
      .filename="calibrations/hpdirc.lut.gz",
      .system=BarrelDIRC_ID,
      .pdg_values={11, 211, 321, 2212},
      .charge_values={-1, 1},
      .momentum_edges={0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 3.2, 3.4, 3.6, 3.8, 4.0, 4.2, 4.4, 4.6, 4.8, 5.0, 5.2, 5.4, 5.6, 5.8, 6.0, 6.2, 6.4, 6.6, 6.8, 7.0, 7.2, 7.4, 7.6, 7.8, 8.0, 8.2, 8.4, 8.6, 8.8, 9.0, 9.2, 9.4, 9.6, 9.8, 10.0, 10.2},
      .polar_edges={25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0, 50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 59.0, 60.0, 61.0, 62.0, 63.0, 64.0, 65.0, 66.0, 67.0, 68.0, 69.0, 70.0, 71.0, 72.0, 73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0, 81.0, 82.0, 83.0, 84.0, 85.0, 86.0, 87.0, 88.0, 89.0, 90.0, 91.0, 92.0, 93.0, 94.0, 95.0, 96.0, 97.0, 98.0, 99.0, 100.0, 101.0, 102.0, 103.0, 104.0, 105.0, 106.0, 107.0, 108.0, 109.0, 110.0, 111.0, 112.0, 113.0, 114.0, 115.0, 116.0, 117.0, 118.0, 119.0, 120.0, 121.0, 122.0, 123.0, 124.0, 125.0, 126.0, 127.0, 128.0, 129.0, 130.0, 131.0, 132.0, 133.0, 134.0, 135.0, 136.0, 137.0, 138.0, 139.0, 140.0, 141.0, 142.0, 143.0, 144.0, 145.0, 146.0, 147.0, 148.0, 149.0, 150.0, 151.0, 152.0, 153.0, 154.0, 155.0, 156.0, 157.0, 158.0, 159.0, 160.0},
      .azimuthal_binning={0.0, 30.5, 0.5}, // lower, upper, step
    };

    app->Add(new JOmniFactoryGeneratorT<PIDLookup_factory>(
          "DIRCTruthSeededLUTPID",
          {
          "ReconstructedTruthSeededChargedWithPFRICHTOFPIDParticles",
          "ReconstructedTruthSeededChargedWithPFRICHTOFPIDParticleAssociations",
          },
          {
          "ReconstructedTruthSeededChargedWithPFRICHTOFDIRCPIDParticles",
          "ReconstructedTruthSeededChargedWithPFRICHTOFDIRCPIDParticleAssociations",
          "DIRCTruthSeededParticleIDs",
          },
          pid_cfg,
          app
          ));

    app->Add(new JOmniFactoryGeneratorT<PIDLookup_factory>(
          "DIRCLUTPID",
          {
          "ReconstructedChargedWithPFRICHTOFPIDParticles",
          "ReconstructedChargedWithPFRICHTOFPIDParticleAssociations",
          },
          {
          "ReconstructedChargedWithPFRICHTOFDIRCPIDParticles",
          "ReconstructedChargedWithPFRICHTOFDIRCPIDParticleAssociations",
          "DIRCParticleIDs",
          },
          pid_cfg,
          app
          ));
  }
}
