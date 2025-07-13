// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/VertexCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <vector>

#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "IterativeVertexFinderConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/ActsExamplesEdm.h"
#include "algorithms/tracking/ActsPodioEdm.h"

namespace eicrecon {
template <typename edm_t = eicrecon::ActsExamplesEdm>
class IterativeVertexFinder
    : public eicrecon::WithPodConfig<eicrecon::IterativeVertexFinderConfig> {
public:
  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> log);
  std::unique_ptr<edm4eic::VertexCollection>
  produce(std::vector<const typename edm_t::Trajectories*> trajectories,
          const edm4eic::ReconstructedParticleCollection* reconParticles);

private:
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::MagneticFieldContext m_fieldctx;
  IterativeVertexFinderConfig m_cfg;
};
} // namespace eicrecon
