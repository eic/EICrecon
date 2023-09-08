// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "ActsGeometryProvider.h"
#include "IterativeVertexFinderConfig.h"
#include <vector>

#include "DD4hepBField.h"
#include "ActsExamples/EventData/GeometryContainers.hpp"
#include "ActsExamples/EventData/Track.hpp"
#include "ActsExamples/EventData/Trajectories.hpp"

#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <edm4eic/Vertex.h>
#include <spdlog/logger.h>

#include <Acts/Definitions/Common.hpp>
#include "algorithms/interfaces/WithPodConfig.h"
#include <edm4eic/TrackParameters.h>
#include <edm4hep/MCParticle.h>

namespace eicrecon {
class IterativeVertexFinder
    : public eicrecon::WithPodConfig<eicrecon::IterativeVertexFinderConfig> {
public:
  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> log);
  std::vector<edm4eic::Vertex*>
  produce(std::vector<const ActsExamples::Trajectories*> trajectories);

private:
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::MagneticFieldContext m_fieldctx;
  IterativeVertexFinderConfig m_cfg;
};
} // namespace eicrecon
