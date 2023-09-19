// Copyright (C) 2022, 2023 Chao Peng, Wouter Deconinck, Sylvester Joosten, Dmitry Kalinkin, David Lawrence
// SPDX-License-Identifier: LGPL-3.0-or-later

// References:
//   https://cds.cern.ch/record/687345/files/note01_034.pdf
//   https://www.jlab.org/primex/weekly_meetings/primexII/slides_2012_01_20/island_algorithm.pdf

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <TInterpreter.h>
#include <TInterpreterValue.h>
#include <bits/utility.h>
#include <edm4eic/CalorimeterHitData.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <fmt/format.h>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "CalorimeterIslandCluster.h"
#include "algorithms/calorimetry/CalorimeterIslandClusterConfig.h"

using namespace edm4eic;

namespace eicrecon {

unsigned int CalorimeterIslandCluster::function_id = 0;

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
      edm4eic::magnitude(h1.getPosition()) - edm4eic::magnitude(h2.getPosition())
    ),
    static_cast<vector_type>(
      Phi_mpi_pi(edm4eic::angleAzimuthal(h1.getPosition()) - edm4eic::angleAzimuthal(h2.getPosition()))
    )
  };
}
static edm4hep::Vector2f globalDistEtaPhi(const CaloHit &h1, const CaloHit &h2) {
  using vector_type = decltype(edm4hep::Vector2f::a);
  return {
    static_cast<vector_type>(
      edm4eic::eta(h1.getPosition()) - edm4eic::eta(h2.getPosition())
    ),
    static_cast<vector_type>(
      Phi_mpi_pi(edm4eic::angleAzimuthal(h1.getPosition()) - edm4eic::angleAzimuthal(h2.getPosition()))
    )
  };
}

//------------------------
// AlgorithmInit
//------------------------
void CalorimeterIslandCluster::init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger) {
    m_log = logger;
    m_detector = detector;

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
        m_log->warn("Expect {} values from {}, received {}. ignored it.", units.size(), uprop.first,  uprop.second.size());
        return false;
      } else {
        for (size_t i = 0; i < units.size(); ++i) {
          neighbourDist[i] = uprop.second[i] / units[i];
        }
        hitsDist = method;
        m_log->info("Clustering uses {} with distances <= [{}]", uprop.first, fmt::join(neighbourDist, ","));
      }
      return true;
    };

    std::map<std::string, std::vector<double>> uprops{
            {"localDistXY", m_cfg.localDistXY},
            {"localDistXZ", m_cfg.localDistXZ},
            {"localDistYZ", m_cfg.localDistYZ},
            {"globalDistRPhi", m_cfg.globalDistRPhi},
            {"globalDistEtaPhi", m_cfg.globalDistEtaPhi},
            // default one should be the last one
            {"dimScaledLocalDistXY", m_cfg.dimScaledLocalDistXY}
    };

    bool method_found = false;

    // Adjacency matrix methods
    if (!m_cfg.adjacencyMatrix.empty()) {
      // sanity checks
      if (m_cfg.readout.empty()) {
        m_log->error("readoutClass is not provided, it is needed to know the fields in readout ids");
      }
      m_idSpec = m_detector->readout(m_cfg.readout).idSpec();

      std::string func_name = fmt::format("_CalorimeterIslandCluster_{}", function_id++);
      std::ostringstream sstr;
      sstr << "bool " << func_name << "(double params[]){";
      unsigned int param_ix = 0;
      for(const auto &p : m_idSpec.fields()) {
        const std::string &name = p.first;
        const dd4hep::IDDescriptor::Field* field = p.second;
        sstr << "double " << name << "_1 = params[" << (param_ix++) << "];";
        sstr << "double " << name << "_2 = params[" << (param_ix++) << "];";
      }
      sstr << "return " << m_cfg.adjacencyMatrix << ";";
      sstr << "}";
      m_log->debug("Compiling {}", sstr.str());

      TInterpreter *interp = TInterpreter::Instance();
      interp->ProcessLine(sstr.str().c_str());
      std::unique_ptr<TInterpreterValue> func_val { gInterpreter->MakeInterpreterValue() };
      interp->Evaluate(func_name.c_str(), *func_val);
      typedef bool (*func_t)(double params[]);
      func_t func = ((func_t)(func_val->GetAsPointer()));

      is_neighbour = [this, func, param_ix](const CaloHit &h1, const CaloHit &h2) {
        std::vector<double> params;
        params.reserve(param_ix);
        for(const auto &p : m_idSpec.fields()) {
          const std::string &name = p.first;
          const dd4hep::IDDescriptor::Field* field = p.second;
          params.push_back(field->value(h1.getCellID()));
          params.push_back(field->value(h2.getCellID()));
          m_log->trace("{}_1 = {}", name, field->value(h1.getCellID()));
          m_log->trace("{}_2 = {}", name, field->value(h2.getCellID()));
        }
        return func(params.data());
      };
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
              return (edm4eic::magnitude(h1.getPosition() - h2.getPosition()) <= m_cfg.sectorDist / dd4hep::mm);
            }
          };

          m_log->info("Using clustering method: {}", uprop.first);
          break;
        }
      }
    }

    if (not method_found) {
      throw std::runtime_error("Cannot determine the clustering coordinates");
    }

    if (m_cfg.splitCluster) {
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


std::unique_ptr<edm4eic::ProtoClusterCollection> CalorimeterIslandCluster::process(const edm4eic::CalorimeterHitCollection &hits) {
    // group neighboring hits
    std::vector<std::set<std::size_t>> groups;

    std::vector<bool> visits(hits.size(), false);
    for (size_t i = 0; i < hits.size(); ++i) {

      {
        const auto& hit = hits[i];
        m_log->debug("hit {:d}: energy = {:.4f} MeV, local = ({:.4f}, {:.4f}) mm, global=({:.4f}, {:.4f}, {:.4f}) mm", i, hit.getEnergy() * 1000., hit.getLocal().x, hit.getLocal().y, hit.getPosition().x,  hit.getPosition().y, hit.getPosition().z);
      }
      // already in a group
      if (visits[i]) {
        continue;
      }
      groups.emplace_back();
      // create a new group, and group all the neighboring hits
      bfs_group(hits, groups.back(), i, visits);
    }

    auto protoClusters = std::make_unique<edm4eic::ProtoClusterCollection>();

    for (auto& group : groups) {
      if (group.empty()) {
        continue;
      }
      auto maxima = find_maxima(hits, group, !m_cfg.splitCluster);
      split_group(hits, group, maxima, protoClusters.get());

      m_log->debug("hits in a group: {}, local maxima: {}", group.size(), maxima.size());
    }

    return protoClusters;

}

} // namespace eicrecon
