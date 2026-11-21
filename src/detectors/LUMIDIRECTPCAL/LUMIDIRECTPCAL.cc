// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <cmath>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // Luminosity direct-photon calorimeter (epic compact/far_backward/lumi/directPC.xml):
  // a single 20 x 20 x 20 cm^3 PbWO4 block on the electron beam axis behind the pair
  // spectrometer, with NoSegmentation, so there is exactly one readout channel. It sees
  // bremsstrahlung and ISR photons. The geometry is a placeholder without a photosensor
  // or readout specification, so the ADC and TDC settings below are copied from the
  // EcalLumiSpec digitization (LUMISPECCAL), the other calorimeter of the luminosity
  // system. The 20 GeV range covers one photon at the highest electron beam energy;
  // several photons in the same frame saturate the ADC, which is not modelled further.
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      "LumiDirectPCALRawHits", {"EventHeader", "LumiDirectPCALHits"},
      {"LumiDirectPCALRawHits", "LumiDirectPCALRawHitLinks", "LumiDirectPCALRawHitAssociations"},
      {
          .eRes          = {0.0 * sqrt(dd4hep::GeV), 0.02, 0.0 * dd4hep::GeV}, // flat 2%
          .tRes          = 0.0 * dd4hep::ns,
          .capADC        = 16384,
          .dyRangeADC    = 20 * dd4hep::GeV,
          .pedMeanADC    = 100,
          .pedSigmaADC   = 1,
          .resolutionTDC = 10 * dd4hep::picosecond,
          .corrMeanScale = "1.0",
          .readout       = "LumiDirectPCALHits",
      },
      app // TODO: Remove me once fixed
      ));
}
}
