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

#include <TFile.h>

// ACTS projection algorithm configuration;
#include "algorithms/tracking/TrackPropagationConfig.h"

#include <edm4hep/SimTrackerHitCollection.h>

// JANA;
//#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include "extensions/jana/JOmniFactory.h"

// Factories;
//#include "global/pid/RichTrack_factory.h"
#include "EICrecon/factories/pid/RichTrack_factory.h"
#include "global/pid/IrtDebugging_factory.h"

// DD4HEP geometry services;
#include "services/geometry/dd4hep/DD4hep_service.h"

#include <DD4hep/DetElement.h>

using json = nlohmann::json;

// Have to start hardcoding known RICH detectors in some way, before trying to pull their
// configuration data out; for the time being the list is:
//
//  BRICH: backward RICH testbed
//  FRICH: forward RICH testbed
// PFRICH: ePIC backward RICH
//
static const char *RICHes[] = {"PFRICH", "BRICH", "FRICH"};
//static const char *RICHes[] = {"PFRICH", "FRICH", "BRICH"};
//static const char *RICHes[] = {"BRICH"};

#include "IRT/CherenkovDetectorCollection.h"

using namespace eicrecon;

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    printf("@@@ RICH-IRT InitPlugin()\n");

    auto dd4hep_service = app->GetService<DD4hep_service>();
    auto det = dd4hep_service->detector();
      
    // Loop through all known RICH detectors handled by IRT 2.0 algorithm through this plugin;
    for(const auto *RICH: RICHes) {
      // First sanity cross-check: detector should be present in the geometry;
      {
	bool exists = false;
	
	//FIXME: is there a way to poll a detector existence without a segfault?;
	//auto rptr = det->detector(RICH);
#if 0
	{
	  auto dtypes = det->detectorTypes();
      
	  printf("@R@: %ld\n", dtypes.size());
	  for(unsigned iq=0; iq<dtypes.size(); iq++) {
	    printf("@R@ %s\n", dtypes[iq].c_str());
	    
	    auto dets = det->detectors(dtypes[iq].c_str());
	    for(const auto &detElem: dets) 
	      printf("@R@   -> %s\n", detElem.volume()->GetName());
	  } 
	}
#endif	
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

      // Check command line options; expect a "config" key with a JSON configuration file name;
      {
	std::vector<std::string> kstring;
	TString key; key.Form("%s:config", RICH);
	app->SetDefaultParameter(key.Data(), kstring, "Test string");
	//printf("@R@: %s %ld\n", RICH, kstring.size());
	// Expect exactly one key; otherwise skip this RICH detector;
	if (kstring.size() != 1) continue;

	IrtDebuggingConfig config;
	  
	// Import JSON configuration file; sanity check for several keys which are supposed to be present;
	{
	  std::ifstream fcfg(kstring[0].c_str());
	  if (!fcfg) continue;
	  config.m_json_config = json::parse(fcfg);
	  // For less typing;
	  /*const*/ json *jptr = &config.m_json_config;

	  {
	    /*const*/ auto &qe = (*jptr)["Photosensor"]["QuantumEfficiency"];
	    //const auto &qqptr = config[name.Data()];
	    //if (qptr->find("Acceptance") != qptr->end()) continue;
	    //const auto &qe = qptr["QuantumEfficiency"];

	    //printf("@R@ %ld\n", qe.size());
	    //printf("@R@ %7.2f\n", qe["330*nm"]. template get<double>());
#if 0
	    for (json::iterator it = qe.begin(); it != qe.end(); ++it) {
	      //printf("@R@ %s\n", *it);// template get<std::string>().c_str());//, entry.second-> template get<double>);
	      //+std::cout << *it << '\n';
	      //+std::cout << it.key() << " : " << it.value() << "\n";
	      printf("@R@ %s: %7.2f\n", it.key().c_str(),// template get<std::string>().c_str(),
		     it.value(). template get<double>());//std::string>().c_str());//, entry.second-> template get<double>);
	    }
#endif
	    //printf("@R@ %s\n", qe[0].template get<std::string>().c_str());
#if 0
	    for(const auto &entry: qe) {
	      printf("@R@ %s: %7.2f\n", (*entry).key().c_str(),// template get<std::string>().c_str(),
		     entry.value(). template get<double>());//std::string>().c_str());//, entry.second-> template get<double>);
	      // std::cout << *entry << '\n';
	      //std::cout << entry;// >> std::cout;//
	      //printf("@R@ %s\n", entry. template get<std::string>().c_str());//, entry.second-> template get<double>);
	      //printf("@R@ %7.2f\n", entry["330*nm"]. template get<double>());
	    }
#endif	        
	  }
	  
	  //printf("@R@ %ld\n", jptr->size());
	  // An entry describing optics file should be present;
	  if (jptr->find("Optics") == jptr->end()) continue;
	  
	  // An entry describing a nominal acceptance should be present;
	  if (jptr->find("Acceptance") == jptr->end()) continue;
	  const auto &aconfig = (*jptr)["Acceptance"];
	  if (aconfig.find("eta-min") == aconfig.end() || aconfig.find("eta-max") == aconfig.end())
	    continue;

	  // And a group of entries describing various radiator parameters;
	  if (jptr->find("Radiators") == jptr->end()) continue;
	  const auto &rconfig = (*jptr)["Radiators"];
	  
	  // Import Cherenkov detector optics configuration file;
	  {
	    auto foptics = new TFile((*jptr)["Optics"].template get<std::string>().c_str());
	    if (!foptics) continue;
	    
	    config.m_irt_geometry = dynamic_cast<CherenkovDetectorCollection*>(foptics->Get("CherenkovDetectorCollection"));
	    if (!config.m_irt_geometry) continue;
	    
	    auto cdet = config.m_irt_geometry->GetDetector(RICH);
	    if (!cdet) continue;

	    //
	    // Everything is fine, proceed with the essential part;
	    //
	    
	    // Track propagation config; FIXME: for now supposed to work for aerogel only;
	    TrackPropagationConfig track_cfg;
	    
	    for(auto [name,radiator] : cdet->Radiators()) {
	      //printf("@R@ %s\n", name.Data());

	      // There should be an entry in JSON file; skip otherwise;
	      if (rconfig.find(name.Data()) == rconfig.end()) continue;
	      const auto &rrconfig = rconfig[name.Data()];
	      if (rrconfig.find("imaging") != rrconfig.end() &&
		  !strcmp(rrconfig["imaging"].template get<std::string>().c_str(), "no"))
		continue;

	      // FIXME: for now assume thin flat radiators -> this option must be present;
	      if (rrconfig.find("acts-planes") == rrconfig.end()) continue;
	      
	      // FIXME: it is implicitly assumed that radiators are ordered along the track direction; cross-check?;
	      // FIXME: assume isec=0 should work for dRICH as well?;
	      auto sf = dynamic_cast<FlatSurface*>(radiator->GetFrontSide(0));//isec);
	      auto sr = dynamic_cast<FlatSurface*>(radiator->GetRearSide(0));//isec);
	      double zf = sf->GetCenter().Z(), zr = sr->GetCenter().Z();
	      
	      unsigned numPlanes = rrconfig["acts-planes"].template get<int>();
	      //printf("@R@ %d\n", numPlanes);
	      config.m_eta_min = aconfig["eta-min"].template get<double>();
	      config.m_eta_max = aconfig["eta-max"].template get<double>();
	      double theta_min = 2*std::atan(exp(-fabs(config.m_eta_min)));//aconfig["eta-min"].template get<double>())));
	      double theta_max = 2*std::atan(exp(-fabs(config.m_eta_max)));//aconfig["eta-max"].template get<double>())));
	      if (theta_max < theta_min) std::swap(theta_min, theta_max);
	      
	      // "+1": avoid a "coordinate at the boundary condition"; essentially make numPlanes bins and use bin centers;
	      //double zmid = (zf+zr)/2 * dd4hep::mm, step = fabs(zf-zr)/(numPlanes+1) * dd4hep::mm;
	      // NB: 'step' is a signed variable here, whether ACTS reorders the planes in Z internally or not;
	      double zmid = (zf+zr)/2 * dd4hep::mm, step = (zr-zf)/(numPlanes+1) * dd4hep::mm;
		
	      const char *tag = zmid > 0.0 ? "ForwardRICH_ID" : "BackwardRICH_ID";
	      for(int i=0; i<numPlanes; i++) {
		auto zCoord = zmid + step*(i - (numPlanes-1)/2.);
		double rmin = fabs(zCoord)*tan(theta_min), rmax = fabs(zCoord)*tan(theta_max);
		//printf("@R@ %f %f %f (%f %f) %f\n", rmin, rmax, zCoord, zf, zr, step);
		
		track_cfg.target_surfaces.push_back(eicrecon::DiscSurfaceConfig{tag, zCoord, rmin, rmax});
		//m_log->debug("  disk {}: z={} r=[ {}, {} ]", i, z, rmin, rmax);
	      } //for i
	      
		// FIXME: this should be fine?;
	      track_cfg.filter_surfaces.push_back(track_cfg.target_surfaces.back());//eicrecon::DiscSurfaceConfig{tag, zmid, rmin, rmax});
	      
	      // FIXME: may not be a good idea for dRICH;
	      track_cfg.track_point_cut =
		std::function<bool(edm4eic::TrackPoint)> ([] (edm4eic::TrackPoint p) { return true; });
	    } //for radiators
	    
	    // Eventually define the factories to be used; 
	    {
	      TString RICHstr(RICH), RICHtracks = RICHstr + "Tracks";
	      
	      // Charged particle tracks;
	      app->Add(new JOmniFactoryGeneratorT<RichTrack_factory>
		       (
			RICHtracks.Data(),
			{"CentralCKFTracks", "CentralCKFActsTrajectories", "CentralCKFActsTracks"},
			{RICHtracks.Data()},
			track_cfg/*,
				   app*/
			));

#if 1
	      // A unified IRT 2.0 debugging algorithm; FIXME: split digitization step off later;
	      app->Add(new JOmniFactoryGeneratorT<IrtDebugging_factory>
		       (
			//"IrtDebugging",
			(RICHstr + "IrtDebugging").Data(),
			{
			  "MCParticles",
			  "ReconstructedChargedWithoutPIDParticles",
			  "ReconstructedChargedWithoutPIDParticleAssociations",
			  RICHtracks.Data(), (RICHstr + "Hits").Data()
			},
			{(RICHstr + "IrtDebugInfoTables").Data()},
			config/*,
				app*/
			));
#endif
	    }
	  }
	}
      }
    } //for RICH
  }
}
