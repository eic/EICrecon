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

#if 0
std::function<bool(edm4eic::TrackPoint)> TrackPointCut( void )//int radiator)
{
  // FIXME: dRICH may still require a more comprehensive code;
  //printf("richgeo::ActsGeo::TrackPointCut() for QRICH!\n");
  return [] (edm4eic::TrackPoint p) { return true; };
}
#endif

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    printf("@@@ QRICH InitPlugin()\n");
#if _LATER_
    {
      auto dd4hep_service = app->GetService<DD4hep_service>();
      auto det = dd4hep_service->detector();

      auto dummy = det->constant<int>("HRPPD_num_px");//DRICH_aerogel_zpos") / dd4hep::mm;
      //printf("@A@: %s %d\n", det->GetName(), dummy);
#if 1
      auto dtypes = det->detectorTypes();
      printf("@A@: %ld\n", dtypes.size());
      for(unsigned iq=0; iq<dtypes.size(); iq++)
	printf("@A@ %s\n", dtypes[iq].c_str());
      auto dets = det->detectors("tracker");//"passive");//compound");//tracker");
      printf("@A@: %ld dets\n", dets.size());
#if 1
      for(auto &detElem: dets) 
	printf("@A@ -> %s\n", detElem.volume()->GetName());//.Data());
      //for(auto const& [detName, detSensor] : detElem.children())// {
      //  printf("@A@ %s\n", detName.c_str());
#endif
      //std::string detName = detElem.nameStr();
      //printf("@A@\n");// %s\n", detName.c_str());
      //}
#endif
#if 0
      //auto rich = det->detector("QRICH");
      //printf("@A@ %s\n", rich.id().c_str());
      //for(auto const& [detName, detSensor] : rich.children()) {
      for(auto const& [detName, detSensor] : det) {
	printf("@A@ %s\n", detName.c_str());
        //if(detName.find("sensor_de_sec")!=std::string::npos) {
	//}
      }
#endif
      
      //auto service = new test_service(app);
    }
#if 0
    std::vector<std::string> test;
    app->SetDefaultParameter("QRICH:test", test, "Test string");
    printf("@T@: %ld\n", test.size());
    if (test.size()) printf("@T@: %s\n", test[0].c_str());
#endif
#endif
    
    using namespace eicrecon;

    // Track propagation; FIXME: get rid of gas part;
    TrackPropagationConfig aerogel_track_cfg;
    //std::vector<eicrecon::SurfaceConfig> aerogel_tracking_planes;//, gas_tracking_planes;
#if 1
    {
      double rmin = 100 * dd4hep::mm, rmax = 800 * dd4hep::mm;

      {
	unsigned numPlanes = 5;
	//std::vector<eicrecon::SurfaceConfig> discs;
	// Step sign: also want to change order;
	double step = sign * 5 * dd4hep::mm, zmid = sign*(z0 - 500./2 + 5. + 25./2) * dd4hep::mm;
	for(int i=0; i<numPlanes; i++) {
	  auto z         = zmid + step*(i - (numPlanes-1)/2.);
	  aerogel_track_cfg.target_surfaces.push_back(eicrecon::DiscSurfaceConfig{tag, z, rmin, rmax});
	  //m_log->debug("  disk {}: z={} r=[ {}, {} ]", i, z, rmin, rmax);
	} //for i
      }
      {
	//unsigned numPlanes = 1;
	// FIXME: make it simple for now;
	double /*step = sign * 5 * dd4hep::cm,*/ zmid = sign * z0 * dd4hep::mm;
	//for(int i=0; i<numPlanes; i++) {
	//auto z         = zmid + step*(i - (numPlanes-1)/2.);
	//gas_tracking_planes.push_back(eicrecon::DiscSurfaceConfig{tag, z, rmin, rmax});
	//m_log->debug("  disk {}: z={} r=[ {}, {} ]", i, z, rmin, rmax);
	//} //for i

	//auto aerogel_track_point_cut = //TrackPointCut();
	//auto gas_track_point_cut = TrackPointCut();
	
	//auto filter_surface = gas_tracking_planes.back();
	// track propagation to each radiator
	aerogel_track_cfg.filter_surfaces.push_back(eicrecon::DiscSurfaceConfig{tag, zmid, rmin, rmax});//filter_surface);
      }
      //aerogel_track_cfg.target_surfaces = aerogel_tracking_planes;
      aerogel_track_cfg.track_point_cut =
	std::function<bool(edm4eic::TrackPoint)> ([] (edm4eic::TrackPoint p) { return true; });//aerogel_track_point_cut;
    }
#else
    TrackPropagationConfig gas_track_cfg;

    // get RICH geo service
    auto richGeoSvc = app->GetService<RichGeo_service>();
    auto dd4hepGeo = richGeoSvc->GetDD4hepGeo();
    if (dd4hepGeo->world().children().contains("QRICH")) {
      printf("@@@ QRICH geometry found!\n");
      auto actsGeo = richGeoSvc->GetActsGeo("QRICH");
      auto aerogel_tracking_planes = actsGeo->TrackingPlanes(richgeo::kAerogel, 5);
      auto aerogel_track_point_cut = actsGeo->TrackPointCut(richgeo::kAerogel);
      auto gas_tracking_planes = actsGeo->TrackingPlanes(richgeo::kGas, 1);//10);
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
#endif
    
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
