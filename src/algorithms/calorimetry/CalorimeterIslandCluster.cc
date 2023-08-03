// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//  Sections Copyright (C) 2023 Chao Peng, Wouter Deconinck, Sylvester Joosten, Dmitry Kalinkin
//  under SPDX-License-Identifier: LGPL-3.0-or-later

#include <sstream>

#include <TInterpreter.h>
#include <TInterpreterValue.h>

#include "CalorimeterIslandCluster.h"

#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>

#include <JANA/JEvent.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>

using namespace edm4eic;

//
// This algorithm converted from:
//
//  https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugDigi/src/components/CalorimeterHitDigi.cpp
//
// TODO:
// - Array type configuration parameters are not yet supported in JANA (needs to be added)
// - Random number service needs to bew resolved (on global scale)
// - It is possible standard running of this with Gaudi relied on a number of parameters
//   being set in the config. If that is the case, they should be moved into the default
//   values here. This needs to be confirmed.


unsigned int CalorimeterIslandCluster::function_id = 0;

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
                std::tuple<std::function<edm4hep::Vector2f(const CaloHit*, const CaloHit*)>, std::vector<double>>>
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
      sstr << "return " << u_adjacencyMatrix << ";";
      sstr << "}";
      m_log->debug("Compiling {}", sstr.str());

      TInterpreter *interp = TInterpreter::Instance();
      interp->ProcessLine(sstr.str().c_str());
      std::unique_ptr<TInterpreterValue> func_val { gInterpreter->MakeInterpreterValue() };
      interp->Evaluate(func_name.c_str(), *func_val);
      typedef bool (*func_t)(double params[]);
      func_t func = ((func_t)(func_val->GetAsPointer()));

      is_neighbour = [this, func, param_ix](const CaloHit* h1, const CaloHit* h2) {
        std::vector<double> params;
        params.reserve(param_ix);
        for(const auto &p : m_idSpec.fields()) {
          const std::string &name = p.first;
          const dd4hep::IDDescriptor::Field* field = p.second;
          params.push_back(field->value(h1->getCellID()));
          params.push_back(field->value(h2->getCellID()));
          m_log->trace("{}_1 = {}", name, field->value(h1->getCellID()));
          m_log->trace("{}_2 = {}", name, field->value(h2->getCellID()));
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

          is_neighbour = [this](const CaloHit* h1, const CaloHit* h2) {
            // in the same sector
            if (h1->getSector() == h2->getSector()) {
              auto dist = hitsDist(h1, h2);
              return (fabs(dist.a) <= neighbourDist[0]) && (fabs(dist.b) <= neighbourDist[1]);
              // different sector, local coordinates do not work, using global coordinates
            } else {
              // sector may have rotation (barrel), so z is included
              // (EDM4hep units are mm, so convert sectorDist to mm)
              return (edm4eic::magnitude(h1->getPosition() - h2->getPosition()) <= m_sectorDist / dd4hep::mm);
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
    /// This is automatically run before Process, when a new run number is seen
    /// Usually we update our calibration constants by asking a JService
    /// to give us the latest data for this run number
}

//------------------------
// AlgorithmProcess
//------------------------
void CalorimeterIslandCluster::AlgorithmProcess()  {
// input collections
    //const auto& hits = event
    // Create output collections
    //auto& proto = *(m_outputProtoCollection.createAndPut());

    // group neighboring hits
    std::vector<std::vector<std::pair<uint32_t, const CaloHit*>>> groups;

    //FIXME: protocluster collection to this?
    std::vector<edm4eic::ProtoCluster> proto;

    std::vector<bool> visits(hits.size(), false);
    //TODO: use the right logger
    for (size_t i = 0; i < hits.size(); ++i) {

      {
        const auto& hit = hits[i];
        m_log->debug("hit {:d}: energy = {:.4f} MeV, local = ({:.4f}, {:.4f}) mm, global=({:.4f}, {:.4f}, {:.4f}) mm", i, hit->getEnergy() * 1000., hit->getLocal().x, hit->getLocal().y, hit->getPosition().x,  hit->getPosition().y, hit->getPosition().z);
      }
      // already in a group
      if (visits[i]) {
        continue;
      }
      groups.emplace_back();
      // create a new group, and group all the neighboring hits
      dfs_group(groups.back(), i, hits, visits);
    }

    for (auto& group : groups) {
      if (group.empty()) {
        continue;
      }
      auto maxima = find_maxima(group, !m_splitCluster);
      split_group(group, maxima, protoClusters);

      m_log->debug("hits in a group: {}, local maxima: {}", group.size(), maxima.size());
    }

    return;

}
