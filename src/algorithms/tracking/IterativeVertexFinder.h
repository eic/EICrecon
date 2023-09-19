// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <memory>
#include <vector>

#include "IterativeVertexFinderConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

class ActsGeometryProvider;
namespace ActsExamples { struct Trajectories; }
namespace edm4eic { class Vertex; }
namespace eicrecon::BField { class DD4hepBField; }
namespace spdlog { class logger; }

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
