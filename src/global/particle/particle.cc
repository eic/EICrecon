// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson, Subhadip Pal

#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"

#include "factories/particle/CaloRemnantCombiner_factory.h"

extern "C" {

void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // ====================================================================
  // PFAlpha: baseline PF implementation
  // ====================================================================

  // --------------------------------------------------------------------
  // PFA (0) connection: split/merge clusters accordingly
  // --------------------------------------------------------------------

  /* TODO move here when ready */

  // --------------------------------------------------------------------
  // PFA (2) arbitration: combine remnants, form neutral candidates
  // --------------------------------------------------------------------

  // backward -----------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<CaloRemnantCombiner_factory>(
      "EndcapNNeutralCandidateParticlesAlpha", {"EcalEndcapNClusters", "HcalEndcapNClusters"},
      {"EndcapNNeutralCandidateParticlesAlpha"}, {.deltaRAddEM = 0.03, .deltaRAddH = 0.15}, app));

  // central ------------------------------------------------------------

  /* TODO add PFA2 BEMC here */
  /* TODO add PFA2 BHCal here */

  // forward ------------------------------------------------------------

  /* TODO add PFA2 FEMC here */
  /* TODO add PFA2 LFHCAL here */
  /* TODO add PFA2 FHCal insert here */
}
} // extern "C"
