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
#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <spdlog/logger.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

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

  using CalorimeterClusterRecoCoGAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::ProtoClusterCollection,
      std::optional<edm4hep::SimCalorimeterHitCollection>
    >,
    algorithms::Output<
      edm4eic::ClusterCollection,
      std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>
    >
  >;

  class CalorimeterClusterRecoCoG
      : public CalorimeterClusterRecoCoGAlgorithm {

  public:
    CalorimeterClusterRecoCoG(std::string_view name)
      : CalorimeterClusterRecoCoGAlgorithm{name,
                            {"inputProtoClusterCollection", "mcHits"},
                            {"outputClusterCollection", "outputAssociations"},
                            "Reconstruct a cluster with the Center of Gravity method. For "
                            "simulation results it optionally creates a Cluster <-> MCParticle "
                            "association provided both optional arguments are provided."} {}

  public:
    void init(std::shared_ptr<spdlog::logger>& logger);

    void process(const Input&, const Output&) const final;

  private:
    std::shared_ptr<spdlog::logger> m_log;

    std::function<double(double, double, double, int)> weightFunc;

  private:

    std::optional<edm4eic::Cluster> reconstruct(const edm4eic::ProtoCluster& pcl) const;

    Property<double> m_samplingFraction{this, "samplingFraction", 1.0, "Sampling fraction"};
    Property<double> m_logWeightBase{this, "logWeightBase", 3.6, "Weight base for log weighting"};
    Property<std::string> m_energyWeight{this, "energyWeight", "log", "Default hit weight method"};
    // Constrain the cluster position eta to be within
    // the eta of the contributing hits. This is useful to avoid edge effects
    // for endcaps.
    Property<bool> m_enableEtaBounds{this, "enableEtaBounds", false, "Constrain cluster to hit eta?"};

  };

} // eicrecon
