// Copyright (C) 2022, 2023 Chao Peng, Wouter Deconinck, Sylvester Joosten, Dmitry Kalinkin, David Lawrence
// SPDX-License-Identifier: LGPL-3.0-or-later

// References:
//   https://cds.cern.ch/record/687345/files/note01_034.pdf
//   https://www.jlab.org/primex/weekly_meetings/primexII/slides_2012_01_20/island_algorithm.pdf

#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/service.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <algorithm>
#include <cmath>
#include <gsl/pointers>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "CalorimeterIslandCluster.h"
#include "algorithms/calorimetry/CalorimeterIslandClusterConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

using namespace edm4eic;

namespace eicrecon {

static double Phi_mpi_pi(double phi) { return std::remainder(phi, 2 * M_PI); }

static edm4hep::Vector2f localDistXY(const CaloHit& h1, const CaloHit& h2) {
  const auto delta = h1.getLocal() - h2.getLocal();
  return {delta.x, delta.y};
}
static edm4hep::Vector2f localDistXZ(const CaloHit& h1, const CaloHit& h2) {
  const auto delta = h1.getLocal() - h2.getLocal();
  return {delta.x, delta.z};
}
static edm4hep::Vector2f localDistYZ(const CaloHit& h1, const CaloHit& h2) {
  const auto delta = h1.getLocal() - h2.getLocal();
  return {delta.y, delta.z};
}
static edm4hep::Vector2f dimScaledLocalDistXY(const CaloHit& h1, const CaloHit& h2) {
  const auto delta = h1.getLocal() - h2.getLocal();

  const auto dimsum = h1.getDimension() + h2.getDimension();

  return {2 * delta.x / dimsum.x, 2 * delta.y / dimsum.y};
}
static edm4hep::Vector2f globalDistRPhi(const CaloHit& h1, const CaloHit& h2) {
  using vector_type = decltype(edm4hep::Vector2f::a);
  return {static_cast<vector_type>(edm4hep::utils::magnitude(h1.getPosition()) -
                                   edm4hep::utils::magnitude(h2.getPosition())),
          static_cast<vector_type>(Phi_mpi_pi(edm4hep::utils::angleAzimuthal(h1.getPosition()) -
                                              edm4hep::utils::angleAzimuthal(h2.getPosition())))};
}
static edm4hep::Vector2f globalDistEtaPhi(const CaloHit& h1, const CaloHit& h2) {
  using vector_type = decltype(edm4hep::Vector2f::a);
  return {static_cast<vector_type>(edm4hep::utils::eta(h1.getPosition()) -
                                   edm4hep::utils::eta(h2.getPosition())),
          static_cast<vector_type>(Phi_mpi_pi(edm4hep::utils::angleAzimuthal(h1.getPosition()) -
                                              edm4hep::utils::angleAzimuthal(h2.getPosition())))};
}

//------------------------
// AlgorithmInit
//------------------------
void CalorimeterIslandCluster::init() {

  static std::map<std::string,
                  std::tuple<std::function<edm4hep::Vector2f(const CaloHit&, const CaloHit&)>,
                             std::vector<double>>>
      distMethods{{"localDistXY", {localDistXY, {dd4hep::mm, dd4hep::mm}}},
                  {"localDistXZ", {localDistXZ, {dd4hep::mm, dd4hep::mm}}},
                  {"localDistYZ", {localDistYZ, {dd4hep::mm, dd4hep::mm}}},
                  {"dimScaledLocalDistXY", {dimScaledLocalDistXY, {1., 1.}}},
                  {"globalDistRPhi", {globalDistRPhi, {dd4hep::mm, dd4hep::rad}}},
                  {"globalDistEtaPhi", {globalDistEtaPhi, {1., dd4hep::rad}}}};

  // set coordinate system
  auto set_dist_method = [this](std::pair<std::string, std::vector<double>> uprop) {
    if (uprop.second.empty()) {
      return false;
    }
    auto& [method, units] = distMethods[uprop.first];
    if (uprop.second.size() != units.size()) {
      warning("Expect {} values from {}, received {}. ignored it.", units.size(), uprop.first,
              uprop.second.size());
      return false;
    }
    for (std::size_t i = 0; i < units.size(); ++i) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
      neighbourDist[i] = uprop.second[i] / units[i];
    }
    hitsDist = method;
    info("Clustering uses {} with distances <= [{}]", uprop.first, fmt::join(neighbourDist, ","));

    return true;
  };

  std::vector<std::pair<std::string, std::vector<double>>> uprops{
      {"localDistXY", m_cfg.localDistXY},
      {"localDistXZ", m_cfg.localDistXZ},
      {"localDistYZ", m_cfg.localDistYZ},
      {"globalDistRPhi", m_cfg.globalDistRPhi},
      {"globalDistEtaPhi", m_cfg.globalDistEtaPhi},
      // default one should be the last one
      {"dimScaledLocalDistXY", m_cfg.dimScaledLocalDistXY}};

  auto& serviceSvc = algorithms::ServiceSvc::instance();

  std::function hit_pair_to_map = [this](const edm4eic::CalorimeterHit& h1,
                                         const edm4eic::CalorimeterHit& h2) {
    std::unordered_map<std::string, double> params;
    for (const auto& p : m_idSpec.fields()) {
      const std::string& name                  = p.first;
      const dd4hep::IDDescriptor::Field* field = p.second;
      params.emplace(name + "_1", field->value(h1.getCellID()));
      params.emplace(name + "_2", field->value(h2.getCellID()));
      trace("{}_1 = {}", name, field->value(h1.getCellID()));
      trace("{}_2 = {}", name, field->value(h2.getCellID()));
    }
    return params;
  };

  if (m_cfg.readout.empty()) {
    if ((!m_cfg.adjacencyMatrix.empty()) || (!m_cfg.peakNeighbourhoodMatrix.empty())) {
      throw std::runtime_error(
          "'readout' is not provided, it is needed to know the fields in readout ids");
    }
  } else {
    m_idSpec = m_detector->readout(m_cfg.readout).idSpec();
  }

  bool method_found = false;

  // Adjacency matrix methods
  if (!m_cfg.adjacencyMatrix.empty()) {
    is_neighbour = serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")
                       ->compile(m_cfg.adjacencyMatrix, hit_pair_to_map);
    method_found = true;
  }

  // Coordinate distance methods
  if (not method_found) {
    for (auto& uprop : uprops) {
      if (set_dist_method(uprop)) {
        method_found = true;

        is_neighbour = [this](const CaloHit& h1, const CaloHit& h2) {
          // in the same sector
          if (h1.getSector() == h2.getSector()) {
            auto dist = hitsDist(h1, h2);
            return (std::abs(dist.a) <= neighbourDist[0]) && (std::abs(dist.b) <= neighbourDist[1]);
            // different sector, local coordinates do not work, using global coordinates
          } // sector may have rotation (barrel), so z is included
          // (EDM4hep units are mm, so convert sectorDist to mm)
          return (edm4hep::utils::magnitude(h1.getPosition() - h2.getPosition()) <=
                  m_cfg.sectorDist / dd4hep::mm);
        };

        break;
      }
    }
  }

  if (not method_found) {
    throw std::runtime_error("Cannot determine the clustering coordinates");
  }

  if (m_cfg.splitCluster) {
    if (!m_cfg.peakNeighbourhoodMatrix.empty()) {
      is_maximum_neighbourhood = serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")
                                     ->compile(m_cfg.peakNeighbourhoodMatrix, hit_pair_to_map);
    } else {
      is_maximum_neighbourhood = is_neighbour;
    }

    auto transverseEnergyProfileMetric_it =
        std::find_if(distMethods.begin(), distMethods.end(),
                     [&](auto& p) { return m_cfg.transverseEnergyProfileMetric == p.first; });
    if (transverseEnergyProfileMetric_it == distMethods.end()) {
      throw std::runtime_error(
          fmt::format(R"(Unsupported value "{}" for "transverseEnergyProfileMetric")",
                      m_cfg.transverseEnergyProfileMetric));
    }
    transverseEnergyProfileMetric = std::get<0>(transverseEnergyProfileMetric_it->second);
    std::vector<double>& units    = std::get<1>(transverseEnergyProfileMetric_it->second);
    for (auto unit : units) {
      if (unit != units[0]) {
        throw std::runtime_error(fmt::format("Metric {} has incompatible dimension units",
                                             m_cfg.transverseEnergyProfileMetric));
      }
    }
    transverseEnergyProfileScaleUnits = units[0];
  }
}

void CalorimeterIslandCluster::process(const CalorimeterIslandCluster::Input& input,
                                       const CalorimeterIslandCluster::Output& output) const {

  const auto [hits]     = input;
  auto [proto_clusters] = output;

  // group neighboring hits
  std::vector<std::set<std::size_t>> groups;

  std::vector<bool> visits(hits->size(), false);
  for (std::size_t i = 0; i < hits->size(); ++i) {

    {
      const auto& hit = (*hits)[i];
      debug("hit {:d}: energy = {:.4f} MeV, local = ({:.4f}, {:.4f}) mm, global=({:.4f}, {:.4f}, "
            "{:.4f}) mm",
            i, hit.getEnergy() * 1000., hit.getLocal().x, hit.getLocal().y, hit.getPosition().x,
            hit.getPosition().y, hit.getPosition().z);
    }
    // already in a group
    if (visits[i]) {
      continue;
    }
    groups.emplace_back();
    // create a new group, and group all the neighboring hits
    bfs_group(*hits, groups.back(), i, visits);
  }

  for (auto& group : groups) {
    if (group.empty()) {
      continue;
    }
    auto maxima = find_maxima(*hits, group, !m_cfg.splitCluster);
    split_group(*hits, group, maxima, proto_clusters);

    debug("hits in a group: {}, local maxima: {}", group.size(), maxima.size());
  }
}

// grouping function with Breadth-First Search
void CalorimeterIslandCluster::bfs_group(const edm4eic::CalorimeterHitCollection& hits,
                                         std::set<std::size_t>& group, std::size_t idx,
                                         std::vector<bool>& visits) const {
  visits[idx] = true;

  // not a qualified hit to participate clustering, stop here
  if (hits[idx].getEnergy() < m_cfg.minClusterHitEdep) {
    return;
  }

  group.insert(idx);
  std::size_t prev_size = 0;

  while (prev_size != group.size()) {
    prev_size = group.size();
    for (std::size_t idx1 : group) {
      // check neighbours
      for (std::size_t idx2 = 0; idx2 < hits.size(); ++idx2) {
        // not a qualified hit to participate clustering, skip
        if (hits[idx2].getEnergy() < m_cfg.minClusterHitEdep) {
          continue;
        }
        if ((!visits[idx2]) && is_neighbour(hits[idx1], hits[idx2])) {
          group.insert(idx2);
          visits[idx2] = true;
        }
      }
    }
  }
}

// find local maxima that above a certain threshold
std::vector<std::size_t>
CalorimeterIslandCluster::find_maxima(const edm4eic::CalorimeterHitCollection& hits,
                                      const std::set<std::size_t>& group, bool global) const {
  std::vector<std::size_t> maxima;
  if (group.empty()) {
    return maxima;
  }

  if (global) {
    std::size_t mpos = *group.begin();
    for (auto idx : group) {
      if (hits[mpos].getEnergy() < hits[idx].getEnergy()) {
        mpos = idx;
      }
    }
    if (hits[mpos].getEnergy() >= m_cfg.minClusterCenterEdep) {
      maxima.push_back(mpos);
    }
    return maxima;
  }

  for (std::size_t idx1 : group) {
    // not a qualified center
    if (hits[idx1].getEnergy() < m_cfg.minClusterCenterEdep) {
      continue;
    }

    bool maximum = true;
    for (std::size_t idx2 : group) {
      if (idx1 == idx2) {
        continue;
      }

      if (is_maximum_neighbourhood(hits[idx1], hits[idx2]) &&
          (hits[idx2].getEnergy() > hits[idx1].getEnergy())) {
        maximum = false;
        break;
      }
    }

    if (maximum) {
      maxima.push_back(idx1);
    }
  }

  return maxima;
}

// split a group of hits according to the local maxima
//TODO: confirm protoclustering without protoclustercollection
void CalorimeterIslandCluster::split_group(const edm4eic::CalorimeterHitCollection& hits,
                                           std::set<std::size_t>& group,
                                           const std::vector<std::size_t>& maxima,
                                           edm4eic::ProtoClusterCollection* protoClusters) const {
  // special cases
  if (maxima.empty()) {
    debug("No maxima found, not building any clusters");
    return;
  } else if (maxima.size() == 1) {
    edm4eic::MutableProtoCluster pcl = protoClusters->create();
    for (std::size_t idx : group) {
      pcl.addToHits(hits[idx]);
      pcl.addToWeights(1.);
    }

    debug("A single maximum found, added one ProtoCluster");

    return;
  }

  // split between maxima
  // TODO, here we can implement iterations with profile, or even ML for better splits
  std::vector<double> weights(maxima.size(), 1.);
  std::vector<edm4eic::MutableProtoCluster> pcls;
  for (std::size_t k = 0; k < maxima.size(); ++k) {
    pcls.push_back(protoClusters->create());
  }

  for (std::size_t idx : group) {
    std::size_t j = 0;
    // calculate weights for local maxima
    for (std::size_t cidx : maxima) {
      double energy = hits[cidx].getEnergy();
      double dist = edm4hep::utils::magnitude(transverseEnergyProfileMetric(hits[cidx], hits[idx]));
      weights[j] =
          std::exp(-dist * transverseEnergyProfileScaleUnits / m_cfg.transverseEnergyProfileScale) *
          energy;
      j += 1;
    }

    // normalize weights
    vec_normalize(weights);

    // ignore small weights
    for (auto& w : weights) {
      if (w < 0.02) {
        w = 0;
      }
    }
    vec_normalize(weights);

    // split energy between local maxima
    for (std::size_t k = 0; k < maxima.size(); ++k) {
      double weight = weights[k];
      if (weight <= 1e-6) {
        continue;
      }
      pcls[k].addToHits(hits[idx]);
      pcls[k].addToWeights(weight);
    }
  }
  debug("Multiple ({}) maxima found, added a ProtoClusters for each maximum", maxima.size());
}

} // namespace eicrecon
