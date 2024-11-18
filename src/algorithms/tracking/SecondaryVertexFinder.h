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
  //produce(std::vector<const edm4eic::Track*>,
  produce(std::vector<const edm4eic::Vertex*>,
          const edm4eic::TrackParametersCollection*,
          std::vector<const ActsExamples::Trajectories*> trajectories);

  // Function to be used to check efficacy of sec. vertex
  bool computeVtxcandidate(const edm4eic::Vertex*,
                           const edm4eic::TrackParameters*,
                           const edm4eic::TrackParameters*,bool);
private:
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::MagneticFieldContext m_fieldctx;
  SecondaryVertexFinderConfig m_cfg;

  //Track paramters to calculate DCA and PCA
  bool secvtxGood=false;
  float trackA_a,trackA_b;
  float trackB_a,trackB_b;
  float vtxA_x,vtxA_y;
  float vtxB_x,vtxB_y;
  float deltaxy_A,deltaA_x,deltaA_y;
  float deltaxy_B,deltaB_x,deltaB_y;
  float minR=0.05;
};
} // namespace eicrecon
