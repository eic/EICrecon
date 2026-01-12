// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/VertexCollection.h>
#include <spdlog/logger.h>
#include <memory>

#include "ActsExamples/EventData/Track.hpp"
#include "ActsGeometryProvider.h"
#include "IterativeVertexFinderConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

class IterativeVertexFinder
    : public eicrecon::WithPodConfig<eicrecon::IterativeVertexFinderConfig> {
public:
  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> log);
  std::unique_ptr<edm4eic::VertexCollection>
  produce(const Acts::ConstVectorMultiTrajectory* trackStates,
          const Acts::ConstVectorTrackContainer* tracks,
          const edm4eic::ReconstructedParticleCollection* reconParticles);

private:
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const Acts::MagneticFieldProvider> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::MagneticFieldContext m_fieldctx;
  IterativeVertexFinderConfig m_cfg;
};
} // namespace eicrecon
