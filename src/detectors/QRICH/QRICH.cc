//
// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto
// Subject to the terms in the LICENSE file found in the top-level directory.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin
//
// Copyright (C) 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <functional>

// algorithm configurations
#include "algorithms/tracking/TrackPropagationConfig.h"

#include <edm4hep/SimTrackerHitCollection.h>

// JANA
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "extensions/jana/JOmniFactory.h"

// factories
#include "global/pid/RichTrack_factory.h"
#include "global/pid/IrtDebugging_factory.h"

// services
#include "services/geometry/richgeo/ActsGeo.h"
#include "services/geometry/richgeo/RichGeo.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    printf("@@@ QRICH InitPlugin()\n");
    
    using namespace eicrecon;

    // Track propagation; FIXME: get rid of gas part;
    TrackPropagationConfig aerogel_track_cfg;
    TrackPropagationConfig gas_track_cfg;

    // get RICH geo service
    auto richGeoSvc = app->GetService<RichGeo_service>();
    auto dd4hepGeo = richGeoSvc->GetDD4hepGeo();
    if (dd4hepGeo->world().children().contains("QRICH")) {
      printf("@@@ QRICH geometry found!\n");
      auto actsGeo = richGeoSvc->GetActsGeo("QRICH");
      auto aerogel_tracking_planes = actsGeo->TrackingPlanes(richgeo::kAerogel, 5);
      auto aerogel_track_point_cut = actsGeo->TrackPointCut(richgeo::kAerogel);
      auto gas_tracking_planes = actsGeo->TrackingPlanes(richgeo::kGas, 10);
      auto gas_track_point_cut = actsGeo->TrackPointCut(richgeo::kGas);
      auto filter_surface = gas_tracking_planes.back();
      // track propagation to each radiator
      aerogel_track_cfg.filter_surfaces.push_back(filter_surface);
      aerogel_track_cfg.target_surfaces = aerogel_tracking_planes;
      aerogel_track_cfg.track_point_cut = aerogel_track_point_cut;
      //gas_track_cfg.filter_surfaces.push_back(filter_surface);
      //gas_track_cfg.target_surfaces = gas_tracking_planes;
      //gas_track_cfg.track_point_cut = gas_track_point_cut;
    }

    // charged particle tracks
    app->Add(new JOmniFactoryGeneratorT<RichTrack_factory>(
          "QRICHAerogelTracks",
          {"CentralCKFTracks", "CentralCKFActsTrajectories", "CentralCKFActsTracks"},
          {"QRICHAerogelTracks"},
          aerogel_track_cfg,
          app
          ));
    
    // QRICH IRT debugging algorithm;
    app->Add(new JOmniFactoryGeneratorT<IrtDebugging_factory>(
          "IrtDebugging",
          {
            "MCParticles",
	    "ReconstructedChargedWithoutPIDParticles",
	    "ReconstructedChargedWithoutPIDParticleAssociations",
	    "QRICHAerogelTracks", "QRICHHits"
          },
          {"IrtDebugInfoTables"},
          //irt_cfg,
          app
          ));
  }
}
