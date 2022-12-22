// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackSeeding.h"

#include "Acts/Seeding/InternalSeed.hpp"
#include "Acts/Seeding/SeedFilterConfig.hpp"
#include "Acts/Seeding/SeedFinderOrthogonalConfig.hpp"
#include "Acts/Seeding/SpacePointGrid.hpp"
#include "Acts/Utilities/KDTree.hpp"
#include "Acts/Seeding/Seed.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/SeedFinderOrthogonal.hpp"

#include <TDatabasePDG.h>

void eicrecon::TrackSeeding::init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> log) {

    m_log = log;

    m_geoSvc = geo_svc;

    m_BField = std::dynamic_pointer_cast<const eicrecon::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
    m_fieldctx = eicrecon::BField::BFieldVariant(m_BField);
    
    m_seederConfig = configureSeeder();
}

std::vector<edm4eic::TrackParameters*> eicrecon::TrackSeeding::produce(std::vector<const edm4eic::TrackerHit*> trk_hits) {

  eicrecon::SeedContainer seeds = runSeeder(trk_hits);

// getGeneratorStatus = 1 means thrown G4Primary
std::vector<edm4eic::TrackParameters*> result = makeTrackParams(seeds);

    return result;
}

eicrecon::SeedContainer eicrecon::TrackSeeding::runSeeder(std::vector<const edm4eic::TrackerHit*>& trk_hits)
{
  std::vector<const eicrecon::SpacePoint*> spacePoints = getSpacePoints(trk_hits);

  Acts::SeedFinderOrthogonal<eicrecon::SpacePoint> finder(m_seederConfig.m_seedFinderConfig);
  eicrecon::SeedContainer seeds = finder.createSeeds(spacePoints);
  return seeds;
}

std::vector<const eicrecon::SpacePoint*> eicrecon::TrackSeeding::getSpacePoints(std::vector<const edm4eic::TrackerHit*>& trk_hits)
{
  std::vector<const eicrecon::SpacePoint*> spacepoints;

  for(const auto hit : trk_hits)
    {
      const eicrecon::SpacePoint* sp = new SpacePoint(*hit);
      spacepoints.push_back(sp);
    }

  return spacepoints;
}

eicrecon::OrthogonalTrackSeedingConfig eicrecon::TrackSeeding::configureSeeder()
{
  eicrecon::OrthogonalTrackSeedingConfig cfg;
  cfg.configure();
  return cfg;
}

std::vector<edm4eic::TrackParameters*> eicrecon::TrackSeeding::makeTrackParams(SeedContainer& seeds)
{
  std::vector<edm4eic::TrackParameters*> trackparams;

  for(auto& seed : seeds)
    {
      for(auto& spptr : seed.sp())
	{
	  
	}
    }

  return trackparams;
}
