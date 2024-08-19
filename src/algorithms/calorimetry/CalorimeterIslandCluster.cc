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
#include <fmt/format.h>
#include <algorithm>
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

static double Phi_mpi_pi(double phi) {
  return std::remainder(phi, 2 * M_PI);
}

static edm4hep::Vector2f localDistXY(const CaloHit &h1, const CaloHit &h2) {
  const auto delta =h1.getLocal() - h2.getLocal();
  return {delta.x, delta.y};
}
static edm4hep::Vector2f localDistXZ(const CaloHit &h1, const CaloHit &h2) {
  const auto delta = h1.getLocal() - h2.getLocal();
  return {delta.x, delta.z};
}
static edm4hep::Vector2f localDistYZ(const CaloHit &h1, const CaloHit &h2) {
  const auto delta = h1.getLocal() - h2.getLocal();
  return {delta.y, delta.z};
}
static edm4hep::Vector2f dimScaledLocalDistXY(const CaloHit &h1, const CaloHit &h2) {
  const auto delta = h1.getLocal() - h2.getLocal();

  const auto dimsum = h1.getDimension() + h2.getDimension();

  return {2 * delta.x / dimsum.x, 2 * delta.y / dimsum.y};
}
static edm4hep::Vector2f globalDistRPhi(const CaloHit &h1, const CaloHit &h2) {
  using vector_type = decltype(edm4hep::Vector2f::a);
  return {
    static_cast<vector_type>(
      edm4hep::utils::magnitude(h1.getPosition()) - edm4hep::utils::magnitude(h2.getPosition())
    ),
    static_cast<vector_type>(
      Phi_mpi_pi(edm4hep::utils::angleAzimuthal(h1.getPosition()) - edm4hep::utils::angleAzimuthal(h2.getPosition()))
    )
  };
}
static edm4hep::Vector2f globalDistEtaPhi(const CaloHit &h1, const CaloHit &h2) {
  using vector_type = decltype(edm4hep::Vector2f::a);
  return {
    static_cast<vector_type>(
      edm4hep::utils::eta(h1.getPosition()) - edm4hep::utils::eta(h2.getPosition())
    ),
    static_cast<vector_type>(
      Phi_mpi_pi(edm4hep::utils::angleAzimuthal(h1.getPosition()) - edm4hep::utils::angleAzimuthal(h2.getPosition()))
    )
  };
}

//------------------------
// AlgorithmInit
//------------------------
void CalorimeterIslandCluster::init() {

    static std::map<std::string,
                std::tuple<std::function<edm4hep::Vector2f(const CaloHit&, const CaloHit&)>, std::vector<double>>>
    distMethods{
        {"localDistXY", {localDistXY, {dd4hep::mm, dd4hep::mm}}},        {"localDistXZ", {localDistXZ, {dd4hep::mm, dd4hep::mm}}},
        {"localDistYZ", {localDistYZ, {dd4hep::mm, dd4hep::mm}}},        {"dimScaledLocalDistXY", {dimScaledLocalDistXY, {1., 1.}}},
        {"globalDistRPhi", {globalDistRPhi, {dd4hep::mm, dd4hep::rad}}}, {"globalDistEtaPhi", {globalDistEtaPhi, {1., dd4hep::rad}}}
    };


    // set coordinate system
    auto set_dist_method = [this](std::pair<std::string, std::vector<double>> uprop) {
      if (uprop.second.size() == 0) {
        return false;
      }
      auto& [method, units] = distMethods[uprop.first];
      if (uprop.second.size() != units.size()) {
        warning("Expect {} values from {}, received {}. ignored it.", units.size(), uprop.first,  uprop.second.size());
        return false;
      } else {
        for (size_t i = 0; i < units.size(); ++i) {
          neighbourDist[i] = uprop.second[i] / units[i];
        }
        hitsDist = method;
        info("Clustering uses {} with distances <= [{}]", uprop.first, fmt::join(neighbourDist, ","));
      }
      return true;
    };

    std::vector<std::pair<std::string, std::vector<double>>> uprops{
            {"localDistXY", m_cfg.localDistXY},
            {"localDistXZ", m_cfg.localDistXZ},
            {"localDistYZ", m_cfg.localDistYZ},
            {"globalDistRPhi", m_cfg.globalDistRPhi},
            {"globalDistEtaPhi", m_cfg.globalDistEtaPhi},
            // default one should be the last one
            {"dimScaledLocalDistXY", m_cfg.dimScaledLocalDistXY}
    };

    auto& serviceSvc = algorithms::ServiceSvc::instance();

    std::function hit_pair_to_map = [this](const edm4eic::CalorimeterHit &h1, const edm4eic::CalorimeterHit &h2) {
      std::unordered_map<std::string, double> params;
      for(const auto &p : m_idSpec.fields()) {
        const std::string &name = p.first;
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
        throw std::runtime_error("'readout' is not provided, it is needed to know the fields in readout ids");
      }
    } else {
      m_idSpec = m_detector->readout(m_cfg.readout).idSpec();
    }

    bool method_found = false;

    // Adjacency matrix methods
    if (!m_cfg.adjacencyMatrix.empty()) {
      is_neighbour = serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")->compile(m_cfg.adjacencyMatrix, hit_pair_to_map);
      method_found = true;
    }

    // Coordinate distance methods
    if (not method_found) {
      for (auto& uprop : uprops) {
        if (set_dist_method(uprop)) {
          method_found = true;

          is_neighbour = [this](const CaloHit &h1, const CaloHit &h2) {
            // in the same sector
            if (h1.getSector() == h2.getSector()) {
              auto dist = hitsDist(h1, h2);
              return (fabs(dist.a) <= neighbourDist[0]) && (fabs(dist.b) <= neighbourDist[1]);
              // different sector, local coordinates do not work, using global coordinates
            } else {
              // sector may have rotation (barrel), so z is included
              // (EDM4hep units are mm, so convert sectorDist to mm)
              return (edm4hep::utils::magnitude(h1.getPosition() - h2.getPosition()) <= m_cfg.sectorDist / dd4hep::mm);
            }
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
        is_maximum_neighbourhood = serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")->compile(m_cfg.peakNeighbourhoodMatrix, hit_pair_to_map);
      } else {
        is_maximum_neighbourhood = is_neighbour;
      }

      auto transverseEnergyProfileMetric_it = std::find_if(distMethods.begin(), distMethods.end(), [&](auto &p) { return m_cfg.transverseEnergyProfileMetric == p.first; });
      if (transverseEnergyProfileMetric_it == distMethods.end()) {
          throw std::runtime_error(fmt::format("Unsupported value \"{}\" for \"transverseEnergyProfileMetric\"", m_cfg.transverseEnergyProfileMetric));
      }
      transverseEnergyProfileMetric = std::get<0>(transverseEnergyProfileMetric_it->second);
      std::vector<double> &units = std::get<1>(transverseEnergyProfileMetric_it->second);
      for (auto unit : units) {
        if (unit != units[0]) {
          throw std::runtime_error(fmt::format("Metric {} has incompatible dimension units", m_cfg.transverseEnergyProfileMetric));
        }
      }
      transverseEnergyProfileScaleUnits = units[0];
    }

    return;
}


void CalorimeterIslandCluster::process(
      const CalorimeterIslandCluster::Input& input,
      const CalorimeterIslandCluster::Output& output) const {

    const auto [hits] = input;
    auto [proto_clusters] = output;

    // group neighboring hits
    std::vector<std::set<std::size_t>> groups;

    std::vector<bool> visits(hits->size(), false);
    for (size_t i = 0; i < hits->size(); ++i) {

      {
        const auto& hit = (*hits)[i];
        debug("hit {:d}: energy = {:.4f} MeV, local = ({:.4f}, {:.4f}) mm, global=({:.4f}, {:.4f}, {:.4f}) mm", i, hit.getEnergy() * 1000., hit.getLocal().x, hit.getLocal().y, hit.getPosition().x,  hit.getPosition().y, hit.getPosition().z);
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

} // namespace eicrecon
