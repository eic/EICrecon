// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong

/*
 *  Reconstruct the cluster with Center of Gravity method
 *  Logarithmic weighting is used for mimicking energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 09/27/2020
 */

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4hep/CaloHitContribution.h>
#include <edm4hep/MCParticle.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>

#include "CalorimeterClusterRecoCoGConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

static double constWeight(double /*E*/, double /*tE*/, double /*p*/, int /*type*/) { return 1.0; }
static double linearWeight(double E, double /*tE*/, double /*p*/, int /*type*/) { return E; }
static double logWeight(double E, double tE, double base, int /*type*/) {
  return std::max(0., base + std::log(E / tE));
}

static const std::map<std::string, std::function<double(double, double, double, int)>>
    weightMethods = {
        {"none", constWeight},
        {"linear", linearWeight},
        {"log", logWeight},
};

namespace eicrecon {

using ClustersWithAssociations =
    std::pair<std::unique_ptr<edm4eic::ClusterCollection>,
              std::unique_ptr<edm4eic::MCRecoClusterParticleAssociationCollection>>;

using CalorimeterClusterRecoCoGAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ProtoClusterCollection,
                      std::optional<edm4eic::MCRecoCalorimeterHitAssociationCollection>>,
    algorithms::Output<edm4eic::ClusterCollection,
                       std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>>>;

class CalorimeterClusterRecoCoG : public CalorimeterClusterRecoCoGAlgorithm,
                                  public WithPodConfig<CalorimeterClusterRecoCoGConfig> {

public:
  CalorimeterClusterRecoCoG(std::string_view name)
      : CalorimeterClusterRecoCoGAlgorithm{
            name,
            {"inputProtoClusterCollection", "mcRawHitAssocations"},
            {"outputClusterCollection", "outputAssociations"},
            "Reconstruct a cluster with the Center of Gravity method. For "
            "simulation results it optionally creates a Cluster <-> MCParticle "
            "association provided both optional arguments are provided."} {}

public:
  void init() final;

  void process(const Input&, const Output&) const final;

private:
  std::function<double(double, double, double, int)> weightFunc;

  // Pointer to the geometry service
  dd4hep::IDDescriptor m_idSpec;
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};

private:
  std::optional<edm4eic::MutableCluster> reconstruct(const edm4eic::ProtoCluster& pcl) const;
  void associate(const edm4eic::Cluster& cl,
                 const edm4eic::MCRecoCalorimeterHitAssociationCollection* mchitassociations,
                 edm4eic::MCRecoClusterParticleAssociationCollection* assocs) const;
  static edm4hep::MCParticle get_primary(const edm4hep::CaloHitContribution& contrib);
};

} // namespace eicrecon
