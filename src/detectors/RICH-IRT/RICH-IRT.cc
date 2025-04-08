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

#include "services/geometry/dd4hep/DD4hep_service.h"

#include <DD4hep/DetElement.h>

#define _ELECTRON_GOING_ENDCAP_CASE_

#ifdef _ELECTRON_GOING_ENDCAP_CASE_
static const double sign = -1.0;
#else
static const double sign =  1.0;
#endif
//static const double z0 = 1456.;//1550.0;
static const double z0 = 1550.0;
  
// Does it really matter?;
static const char *tag = sign > 0.0 ? "ForwardRICH_ID" : "BackwardRICH_ID";

//#include "nlohmann/json.hpp"

// Have to start hardcoding known RICH detectors in some way, before trying to pull their
// configuration data out;
static const char *RICHes[] = {"QRICH", "XRICH"};

#include <TFile.h>

#include "IRT/CherenkovDetectorCollection.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    printf("@@@ RICH InitPlugin()\n");

    auto dd4hep_service = app->GetService<DD4hep_service>();
    auto det = dd4hep_service->detector();
      
    // Loop through all known RICH detectors handled by IRT 2.0 algorithm through this plugin;
    for(const auto *RICH: RICHes) {
      // First sanity cross-check: detector should be present in the geometry;
      {
	bool exists = false;
	
	//FIXME: is there a way to poll a detector existence without a segfault?;
	//auto rptr = det->detector(RICH);

	// Resort to using a loop; assume RICH detector category is "tracker" (it presently is);
	{
	  // May want to loop through all types?; 
	  //+auto dtypes = det->detectorTypes();
	  
	  auto dets = det->detectors("tracker");
	  for(const auto &detElem: dets)
	    if (!strcmp(RICH, detElem.volume()->GetName())) {
	      exists = true;
	      break;
	    } //for
	  //printf("@R@ -> %s\n", detElem.volume()->GetName());
	}

	// Detector was not present in the geometry -> nothing to talk about, ignore command
	// line options and all the rest;
	if (!exists) continue;
      }

      // Check command line options; FIXME: later on use a json file where not only optics
      // .root file, but e.g. digitization parameters can be specified;
      {
	std::vector<std::string> kstring;
	TString key; key.Form("%s:optics", RICH);
	app->SetDefaultParameter(key.Data(), kstring, "Test string");
	//printf("@R@: %s %ld\n", RICH, kstring.size());
	// Expect exactly one key;
	if (kstring.size() != 1) continue;

	// Import optics configuration file;
	{
	  auto fcfg = new TFile(kstring[0].c_str());
	  if (!fcfg) continue;
	  
	  auto geometry = dynamic_cast<CherenkovDetectorCollection*>(fcfg->Get("CherenkovDetectorCollection"));
	  if (!geometry) continue;
	  
	  auto *cdet = geometry->GetDetector(RICH);
	  if (!cdet) continue;
	
	  // Everything is fine, proceed with the essential part;
	  using namespace eicrecon;

	  // Track propagation config; for now QRICH hardcoded and aerogel only; 
	  TrackPropagationConfig aerogel_track_cfg;
	  {
	    double rmin = 100 * dd4hep::mm, rmax = 800 * dd4hep::mm;
	    
	    {
	      unsigned numPlanes = 5;
	      // Step sign: also want to change order;
	      double step = sign * 5 * dd4hep::mm, zmid = sign*(z0 - 500./2 + 5. + 25./2) * dd4hep::mm;
	      for(int i=0; i<numPlanes; i++) {
		auto z         = zmid + step*(i - (numPlanes-1)/2.);
		aerogel_track_cfg.target_surfaces.push_back(eicrecon::DiscSurfaceConfig{tag, z, rmin, rmax});
		//m_log->debug("  disk {}: z={} r=[ {}, {} ]", i, z, rmin, rmax);
	      } //for i
	    }
	    {
	      // FIXME: make it simple for now;
	      double zmid = sign * z0 * dd4hep::mm;
	      
	      aerogel_track_cfg.filter_surfaces.push_back(eicrecon::DiscSurfaceConfig{tag, zmid, rmin, rmax});
	    }
	    aerogel_track_cfg.track_point_cut =
	      std::function<bool(edm4eic::TrackPoint)> ([] (edm4eic::TrackPoint p) { return true; });
	  }

	  // This part obviously is RICH type agnostic; 
	  {
	    TString RICHstr(RICH), RICHtracks = RICHstr + "Tracks";
	    
	    // Charged particle tracks;
	    app->Add(new JOmniFactoryGeneratorT<RichTrack_factory>
		     (
		      RICHtracks.Data(),
		      {"CentralCKFTracks", "CentralCKFActsTrajectories", "CentralCKFActsTracks"},
		      {RICHtracks.Data()},
		      aerogel_track_cfg,
		      app
		      ));
	    
	    // A unified IRT 2.0 debugging algorithm; FIXME: split digitization step off;
	    app->Add(new JOmniFactoryGeneratorT<IrtDebugging_factory>
		     (
		      "IrtDebugging",
		      {
			"MCParticles",
			"ReconstructedChargedWithoutPIDParticles",
			"ReconstructedChargedWithoutPIDParticleAssociations",
			RICHtracks.Data(), (RICHstr + "Hits").Data()//"QRICHHits"
		      },
		      {"IrtDebugInfoTables"},
		      //irt_cfg,
		      app
		    ));
	  }
	}
      }
    } //for RICH
  }
}
