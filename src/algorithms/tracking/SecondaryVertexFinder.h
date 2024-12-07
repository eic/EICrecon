// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/Vertex.h>
#include <edm4eic/TrackParametersCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <vector>

#include "ActsExamples/EventData/Trajectories.hpp"
#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "SecondaryVertexFinderConfig.h"
#include "IterativeVertexFinder.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {
class SecondaryVertexFinder
    : public eicrecon::WithPodConfig<eicrecon::SecondaryVertexFinderConfig> {
public:
  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> log);
  std::unique_ptr<edm4eic::VertexCollection>
  produce(std::vector<const edm4eic::Vertex*>,
          const edm4eic::TrackParametersCollection*,
          const edm4eic::ReconstructedParticleCollection*,
          std::vector<const ActsExamples::Trajectories*> trajectories);

private:
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::MagneticFieldContext m_fieldctx;
  SecondaryVertexFinderConfig m_cfg;

};
} // namespace eicrecon
