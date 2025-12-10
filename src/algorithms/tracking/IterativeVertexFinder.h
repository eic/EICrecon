// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#if __has_include(<ActsPlugins/DD4hep/DD4hepFieldAdapter.hpp>)
#include <ActsPlugins/DD4hep/DD4hepFieldAdapter.hpp>
#else
#include <Acts/Plugins/DD4hep/DD4hepFieldAdapter.hpp>
#endif
#include <edm4eic/VertexCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <spdlog/logger.h>
#include <memory>
#include <vector>

#include "ActsExamples/EventData/Trajectories.hpp"
#include "ActsGeometryProvider.h"
#include "IterativeVertexFinderConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

// Compatibility alias for different Acts versions
#if __has_include(<ActsPlugins/DD4hep/DD4hepFieldAdapter.hpp>)
using DD4hepFieldAdapter = ActsPlugins::DD4hepFieldAdapter;
#else
using DD4hepFieldAdapter = Acts::DD4hepFieldAdapter;
#endif

namespace eicrecon {
class IterativeVertexFinder
    : public eicrecon::WithPodConfig<eicrecon::IterativeVertexFinderConfig> {
public:
  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> log);
  std::unique_ptr<edm4eic::VertexCollection>
  produce(std::vector<const ActsExamples::Trajectories*> trajectories,
          const edm4eic::ReconstructedParticleCollection* reconParticles);

private:
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const DD4hepFieldAdapter> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::MagneticFieldContext m_fieldctx;
  IterativeVertexFinderConfig m_cfg;
};
} // namespace eicrecon
