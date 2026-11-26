// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, 2025, 2026, Alexander Kiselev

#if IRT2_VERSION_MAJOR > 2 || (IRT2_VERSION_MAJOR == 2 && IRT2_VERSION_MINOR >= 2)

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Volumes.h>
#include <Evaluator/DD4hepUnits.h>
#include <IRT2/CherenkovDetector.h>
#include <IRT2/CherenkovDetectorCollection.h>
#include <IRT2/CherenkovRadiator.h>
#include <IRT2/ParametricSurface.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JException.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TGeoVolume.h>
#include <TString.h>
#include <TVector3.h>
#include <cctype>
#include <edm4eic/TrackPoint.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string.h>
#include <algorithm>
#include <cmath>
#include <format>
#include <fstream>
#include <functional>
#include <gsl/pointers>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "algorithms/pid/IrtInterfaceConfig.h"
// ACTS projection algorithm configuration;
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "detectors/RICH-IRT/RichIrtConfig.h"
// JANA;
#include "extensions/jana/JOmniFactoryGeneratorT.h"
// Factories;
#include "factories/tracking/TrackPropagation_factory.h"
#include "global/pid/IrtInterface_factory.h"
// DD4HEP geometry services;
#include "services/geometry/dd4hep/DD4hep_service.h"

using json = nlohmann::json;

// Have to start hardcoding known RICH detectors in some way, before trying
// to pull their configuration data out; for the time being the list is:
//
// PFRICH: ePIC backward proximity focusing RICH
//  DRICH: ePIC forward dual radiator RICH
//
static const std::vector<std::string> RICHes = {"DRICH", "PFRICH"};

using namespace eicrecon;

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  // Loop through all known RICH detectors handled by IRT 2.0 algorithm through this plugin;
  for (const auto& RICH : RICHes) {
    auto configs = GetRichIrtConfigs(app, RICH);
    if (!configs) {
      continue;
    }
    auto& track_cfg = configs->track_cfg;
    auto& config    = configs->config;

    // Eventually define the factories and collections to be used;
    {
      TString RICHstr(RICH), RICHtracks = RICHstr + "Tracks";

      // Charged particle track projections;
      app->Add(new JOmniFactoryGeneratorT<TrackPropagation_factory>(
          RICHtracks.Data(),
          {"CentralCKFTracks", "CentralCKFActsTrackStates", "CentralCKFActsTracks"},
          {RICHtracks.Data()}, track_cfg, app));

      // A unified IRT 2.1 algorithm; FIXME: split digitization step off later;
      app->Add(new JOmniFactoryGeneratorT<IrtInterface_factory>(
          (RICHstr + "IrtInterface").Data(),
          {"EventHeader", "MCParticles", "CentralCKFTracks", "CentralCKFTrackLinks",
           RICHtracks.Data(), (RICHstr + "Hits").Data()},
          {(RICHstr + "IrtRadiatorInfo").Data(), (RICHstr + "IrtParticles").Data()}, config, app));
    }
  } //for RICH
}
}

#endif
