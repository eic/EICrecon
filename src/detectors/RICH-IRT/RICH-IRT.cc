
//
// Copyright (C) 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifdef WITH_IRT2_SUPPORT

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Volumes.h>
#include <Evaluator/DD4hepUnits.h>
#include <IRT2/CherenkovDetector.h>
#include <IRT2/CherenkovRadiator.h>
#include <IRT2/ParametricSurface.h>
#include <IRT2/CherenkovDetectorCollection.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JException.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TGeoVolume.h>
#include <TString.h>
#include <TVector3.h>
#include <edm4eic/TrackPoint.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string.h>
#include <cmath>
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
static const char* RICHes[] = {"PFRICH", "DRICH"};

using namespace eicrecon;

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  auto dd4hep_service = app->GetService<DD4hep_service>();
  auto det            = dd4hep_service->detector();

  // Loop through all known RICH detectors handled by IRT 2.0 algorithm through this plugin;
  for (const auto* RICH : RICHes) {
    // First a sanity cross-check: detector with this name should be present in the geometry;
    {
      bool exists = false;

      // FIXME: is there a way to poll a detector existence without a segfault?;
      // For now resort to using a loop; assume RICH detector category is "tracker"
      // (it presently is); FIXME: may want to loop through all types?;
      for (const auto& detElem : det->detectors("tracker"))
        if (!strcmp(RICH, detElem.volume()->GetName())) {
          exists = true;
          break;
        } //for detElem

      // Detector was not present in the geometry -> throw error
      if (!exists)
        continue;
    }

    // Check command line options; expect a "config" key with a JSON configuration file name;
    {
      std::vector<std::string> kstring;
      TString key;
      key.Form("%s:config", RICH);
      app->SetDefaultParameter(key.Data(), kstring, "Test string");
#if 1
      if (kstring.size() != 1)
        continue;
#else
      // Expect exactly one key; otherwise throw error
      if (kstring.size() != 1)
        throw JException("RICH detector '%s' requires exactly one config file, got %zu", RICH,
                         kstring.size());
#endif

      IrtConfig config;

      // Import JSON configuration file; sanity checks for several keys which are supposed
      // to be present; FIXME: add warning / error printouts;
      {
        std::ifstream fcfg(kstring[0].c_str());
        if (!fcfg)
          throw JException("RICH detector '%s' cannot open config file '%s'", RICH,
                           kstring[0].c_str());
        config.m_json_config = json::parse(fcfg);
        // For less typing;
        json* jptr = &config.m_json_config;

        // An entry describing a nominal acceptance should be present;
        if (jptr->find("Acceptance") == jptr->end())
          throw JException("RICH detector '%s' config missing 'Acceptance' section", RICH);
        {
          const auto& aconfig = (*jptr)["Acceptance"];

          if (aconfig.find("eta-min") == aconfig.end() || aconfig.find("eta-max") == aconfig.end())
            throw JException(
                "RICH detector '%s' config missing 'eta-min' or 'eta-max' in Acceptance section",
                RICH);

          config.m_eta_min = aconfig["eta-min"].template get<double>();
          config.m_eta_max = aconfig["eta-max"].template get<double>();
          // FIXME: it will for now (and perhaps forever) be assumed that these min
          // and max values have the same sign (and are therefore usable to identify
          // what endcap the detector belongs to);
          if (config.m_eta_min * config.m_eta_max < 0.0)
            throw JException(
                "RICH detector '%s' eta-min and eta-max must have same sign, got %f and %f", RICH,
                config.m_eta_min, config.m_eta_max);
        }

        // And a group of entries describing various radiator parameters;
        if (jptr->find("Radiators") == jptr->end())
          throw JException("RICH detector '%s' config missing 'Radiators' section", RICH);
        const auto& rconfig = (*jptr)["Radiators"];

        // Obtain a handle to a Cherenkov detector optics configuration;
        {
          config.m_irt_geometry = IRT2::CherenkovDetectorCollection::Instance();
          if (!config.m_irt_geometry)
            throw JException(
                "RICH detector '%s' failed to get CherenkovDetectorCollection instance", RICH);

          //printf("@Q@ config.m_irt_geometry->GetDetectors() size: %ld\n", config.m_irt_geometry->GetDetectors().size());
          auto cdet = config.m_irt_detector = config.m_irt_geometry->GetDetector(RICH);
          if (!cdet)
            throw JException("RICH detector '%s' not found in IRT geometry collection", RICH);

          //
          // Everything is fine, proceed with the essential part;
          //

          // Track propagation config;
          TrackPropagationConfig track_cfg;

          // Eventually do all this in a proper way: consider all sectors, and make use
          // of eta boundaries to define a Z-range for a particular radiator; reorder radiators in Z if needed;
          std::map<double, std::pair<double, double>> disks;

          for (auto [name, radiator] : cdet->Radiators()) {
            // There should be an entry in JSON file; skip otherwise;
            if (rconfig.find(name.Data()) == rconfig.end())
              continue;

            const auto& rrconfig = rconfig[name.Data()];
            // FIXME: per default assume that all defined radiators participate in ring imaging?;
            if (rrconfig.find("imaging") != rrconfig.end() &&
                !strcmp(rrconfig["imaging"].template get<std::string>().c_str(), "no"))
              continue;

            // Well, this option which instructs ACTS where to build track projections must be present;
            if (rrconfig.find("acts-planes") == rrconfig.end())
              continue;

            unsigned numPlanes = rrconfig["acts-planes"].template get<int>();
            if (!numPlanes)
              throw JException("RICH detector '%s' radiator '%s' has zero acts-planes", RICH,
                               name.Data());

            double theta_min = 2 * std::atan(exp(-config.m_eta_min));
            double theta_max = 2 * std::atan(exp(-config.m_eta_max));

            // Estimate a required Z-range;
            double zmin = 0.0, zmax = 0.0;
            for (unsigned iq = 0; iq < 2; iq++) {
              double theta = iq ? theta_max : theta_min;
              TVector3 x0(0, 0, 0), n0(0.0, sin(theta), cos(theta)), from, to;
              unsigned isec = cdet->GetSector(n0);

              auto sf = radiator->GetFrontSide(isec);
              auto sr = radiator->GetRearSide(isec);

              // FIXME: may want to check return codes?;
              [[maybe_unused]] bool bf = sf->GetCrossing(x0, n0, &from, false);
              [[maybe_unused]] bool br = sr->GetCrossing(x0, n0, &to, false);

              double zf = from.Z(), zr = to.Z();
              if (!iq || fabs(zf) < fabs(zmin))
                zmin = zf;
              if (!iq || fabs(zr) > fabs(zmax))
                zmax = zr;
            } //for iq

            // "+1": avoid a "coordinate at the boundary condition"; essentially make 'numPlanes'
            // bins and use bin centers;
            double zmid = (zmin + zmax) / 2 * dd4hep::mm,
                   step = fabs(zmax - zmin) / (numPlanes + 1) * dd4hep::mm;

            for (unsigned i = 0; i < numPlanes; i++) {
              auto zCoord =
                  zmid + step * (static_cast<int>(i) - (static_cast<int>(numPlanes) - 1) / 2.);
              double rmin = fabs(zCoord * tan(theta_min)), rmax = fabs(zCoord * tan(theta_max));
              if (rmax < rmin)
                std::swap(rmin, rmax);

              // Yes, prefer to order in ascending fabs(z) order in both endcaps; FIXME: implicitly assume
              // all these coordinates are >0 in the hadron-going endcap and <0 in the electron-going one;
              disks[fabs(zCoord)] = std::make_pair(rmin, rmax);
            } //for i
          } //for radiators

          const char* tag = config.m_eta_min > 0.0 ? "ForwardRICH_ID" : "BackwardRICH_ID";
          // Will be ordered along the track direction (assuming IP is the origin),
          // no matter ACTS performs reordering internally or not;
          for (auto disk : disks) {
            double z    = (config.m_eta_min > 0.0 ? 1.0 : -1.0) * disk.first;
            double rmin = disk.second.first, rmax = disk.second.second;

            track_cfg.target_surfaces.push_back(eicrecon::DiscSurfaceConfig{tag, z, rmin, rmax});
          } //for disk

          // FIXME: this should be fine?;
          track_cfg.filter_surfaces.push_back(track_cfg.target_surfaces.back());

          // FIXME: may not be a good idea for dRICH;
          track_cfg.track_point_cut =
              std::function<bool(edm4eic::TrackPoint)>([](edm4eic::TrackPoint) { return true; });

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
                {"MCParticles", "CombinedTruthSeededTracks", "CombinedTruthSeededTrackAssociations",
                 RICHtracks.Data(), (RICHstr + "Hits").Data()},
                {(RICHstr + "IrtRadiatorInfo").Data(), (RICHstr + "IrtParticles").Data()}, config,
                app));
          }
        }
      }
    }
  } //for RICH
}
}

#endif
