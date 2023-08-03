// Copyright (C) 2022, 2023 Chao Peng, Wouter Deconinck, Sylvester Joosten, Dmitry Kalinkin, David Lawrence
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <vector>
#include <set>

#include "CalorimeterIslandCluster.h"

#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>

// Expression evaluator from DD4hep
#include <Evaluator/Evaluator.h>
#include <Evaluator/detail/Evaluator.h>

#include <JANA/JEvent.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>

using namespace edm4eic;

static double Phi_mpi_pi(double phi) {
  return std::remainder(phi, 2 * M_PI);
}

//TODO:Reconcile edm4hep::Vector2f and edm4eic::Vector3f especially with regards to the operators and sign convention
static edm4hep::Vector2f localDistXY(const CaloHit &h1, const CaloHit &h2) {
  //edm4eic::Vector3f h1_pos=geo_converter->position(h1.getCellID());
  //edm4eic::Vector3f h2_pos=geo_converter->position(h2.getCellID());
  const auto delta =h1.getLocal() - h2.getLocal();
  return {delta.x, delta.y};
  //const auto deltax = h1.getLocal()[0] - h2.getLocal()[0];
  //const auto deltay = h1.getLocal()[1] - h2.getLocal()[1];
  //return {delta.x, delta.y,0};
  //return {deltax,deltay};
}
static edm4hep::Vector2f localDistXZ(const CaloHit &h1, const CaloHit &h2) {
  const auto delta = h1.getLocal() - h2.getLocal();
  //const auto deltax = h1.getLocal()[0] - h2.getLocal()[0];
  //const auto deltaz = h1.getLocal()[2] - h2.getLocal()[2];
  return {delta.x, delta.z};
}
static edm4hep::Vector2f localDistYZ(const CaloHit &h1, const CaloHit &h2) {
  const auto delta = h1.getLocal() - h2.getLocal();
  //const auto deltay = h1.getLocal()[1] - h2.getLocal()[1];
  //const auto deltaz = h1.getLocal()[2] - h2.getLocal()[2];
  return {delta.y, delta.z};
}
static edm4hep::Vector2f dimScaledLocalDistXY(const CaloHit &h1, const CaloHit &h2) {
  const auto delta = h1.getLocal() - h2.getLocal();
  //const auto deltax = h1.getLocal()[0] - h2.getLocal()[0];
  //const auto deltay = h1.getLocal()[1] - h2.getLocal()[1];

  const auto dimsum = h1.getDimension() + h2.getDimension();
  //const auto dimsumx = h1.getDimension()[0] + h2.getDimension()[0];
  //const auto dimsumy = h1.getDimension()[1] + h2.getDimension()[1];

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
void CalorimeterIslandCluster::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {

    // Assume all configuration parameter data members have been filled in already.

    // Gaudi implements a random number generator service. It is not clear to me how this
    // can work. There are multiple race conditions that occur in parallel event processing:
    // 1. The exact same events processed by a given thread in one invocation will not
    //    necessarily be the combination of events any thread sees in a subsequent
    //    invocation. Thus, you can't rely on thread_local storage.
    // 2. Its possible for the factory execution order to be modified by the presence of
    //    a processor (e.g. monitoring plugin). This is not as serious since changing the
    //    command line should cause one not to expect reproducibility. Still, one may
    //    expect the inclusion of an "observer" plugin not to have such side affects.
    //
    // More information will be needed. In the meantime, we implement a local random number
    // generator. Ideally, this would be seeded with the run number+event number, but for
    // now, just use default values defined in header file.


    m_log=logger;

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
            {"localDistXY", u_localDistXY},
            {"localDistXZ", u_localDistXZ},
            {"localDistYZ", u_localDistYZ},
            {"globalDistRPhi", u_globalDistRPhi},
            {"globalDistEtaPhi", u_globalDistEtaPhi},
            // default one should be the last one
            {"dimScaledLocalDistXY", u_dimScaledLocalDistXY}
    };

    bool method_found = false;

    // Adjacency matrix methods
    if (!u_adjacencyMatrix.empty()) {
      // sanity checks
      if (!m_geoSvc) {
        m_log->error("Unable to locate Geometry Service. ",
                     "Make sure you have GeoSvc and SimSvc",
                     "in the right order in the configuration.");
        return;
      }
      if (m_readout.empty()) {
        m_log->error("readoutClass is not provided, it is needed to know the fields in readout ids");
      }
      m_idSpec = m_geoSvc->detector()->readout(m_readout).idSpec();
      is_neighbour = [this](const CaloHit &h1, const CaloHit &h2) {
        dd4hep::tools::Evaluator::Object evaluator;
        for(const auto &p : m_idSpec.fields()) {
          const std::string &name = p.first;
          const dd4hep::IDDescriptor::Field* field = p.second;
          evaluator.setVariable((name + "_1").c_str(), field->value(h1.getCellID()));
          evaluator.setVariable((name + "_2").c_str(), field->value(h2.getCellID()));
          m_log->trace("setVariable(\"{}_1\", {});", name, field->value(h1.getCellID()));
          m_log->trace("setVariable(\"{}_2\", {});", name, field->value(h2.getCellID()));
        }
        dd4hep::tools::Evaluator::Object::EvalStatus eval = evaluator.evaluate(u_adjacencyMatrix.c_str());
        if (eval.status()) {
          std::stringstream sstr;
          eval.print_error(sstr);
          throw std::runtime_error(fmt::format("Error evaluating adjacencyMatrix: {}", sstr.str()));
        }
        m_log->trace("Evaluated {} to {}", u_adjacencyMatrix, eval.result());
        return eval.result();
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
              return (edm4eic::magnitude(h1.getPosition() - h2.getPosition()) <= m_sectorDist / dd4hep::mm);
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

    if (m_splitCluster) {
      auto transverseEnergyProfileMetric_it = std::find_if(distMethods.begin(), distMethods.end(), [&](auto &p) { return u_transverseEnergyProfileMetric == p.first; });
      if (transverseEnergyProfileMetric_it == distMethods.end()) {
          throw std::runtime_error(fmt::format("Unsupported value \"{}\" for \"transverseEnergyProfileMetric\"", u_transverseEnergyProfileMetric));
      }
      transverseEnergyProfileMetric = std::get<0>(transverseEnergyProfileMetric_it->second);
      std::vector<double> &units = std::get<1>(transverseEnergyProfileMetric_it->second);
      for (auto unit : units) {
        if (unit != units[0]) {
          throw std::runtime_error(fmt::format("Metric {} has incompatible dimension units", u_transverseEnergyProfileMetric));
        }
      }
      transverseEnergyProfileScaleUnits = units[0];
    }

    return;
}

//------------------------
// AlgorithmChangeRun
//------------------------
void CalorimeterIslandCluster::AlgorithmChangeRun() {
}

//------------------------
// AlgorithmProcess
//------------------------
std::unique_ptr<edm4eic::ProtoClusterCollection> CalorimeterIslandCluster::AlgorithmProcess(const edm4eic::CalorimeterHitCollection &hits) {
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
      dfs_group(hits, groups.back(), i, visits);
    }

    auto protoClusters = std::make_unique<edm4eic::ProtoClusterCollection>();

    for (auto& group : groups) {
      if (group.empty()) {
        continue;
      }
      auto maxima = find_maxima(hits, group, !m_splitCluster);
      split_group(hits, group, maxima, protoClusters.get());

      m_log->debug("hits in a group: {}, local maxima: {}", group.size(), maxima.size());
    }

    return protoClusters;

}
