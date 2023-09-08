// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong

/*
 *  Reconstruct the cluster with Center of Gravity method
 *  Logarithmic weighting is used for mimicking energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 09/27/2020
 */

#pragma once

#include <DD4hep/Detector.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/vector_utils.h>
#include <map>
#include <spdlog/spdlog.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "CalorimeterClusterRecoCoGConfig.h"

static double constWeight(double /*E*/, double /*tE*/, double /*p*/, int /*type*/) { return 1.0; }
static double linearWeight(double E, double /*tE*/, double /*p*/, int /*type*/) { return E; }
static double logWeight(double E, double tE, double base, int /*type*/) {
    return std::max(0., base + std::log(E / tE));
}

static const std::map<std::string, std::function<double(double, double, double, int)>> weightMethods={
      {"none", constWeight},
      {"linear", linearWeight},
      {"log", logWeight},
};

namespace eicrecon {

  using ClustersWithAssociations = std::pair<
    std::unique_ptr<edm4eic::ClusterCollection>,
    std::unique_ptr<edm4eic::MCRecoClusterParticleAssociationCollection>
  >;

  class CalorimeterClusterRecoCoG : public WithPodConfig<CalorimeterClusterRecoCoGConfig> {

  public:
    void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger);

    ClustersWithAssociations process(
            const edm4eic::ProtoClusterCollection* proto,
            const edm4hep::SimCalorimeterHitCollection* mchits);

  private:
    const dd4hep::Detector* m_detector;
    std::shared_ptr<spdlog::logger> m_log;

    std::function<double(double, double, double, int)> weightFunc;

  private:

    edm4eic::Cluster* reconstruct(const edm4eic::ProtoCluster& pcl) const;

  };

} // eicrecon
