// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//  Sections Copyright (C) 2022 Chao Peng, Wouter Deconinck, Sylvester Joosten
//  under SPDX-License-Identifier: LGPL-3.0-or-later

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



//------------------------
// AlgorithmInit
//------------------------
void CalorimeterIslandCluster::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {

    // Assume all configuration parameter data members have been filled in already.

    // Gaudi implments a random number generator service. It is not clear to me how this
    // can work. There are multiple race conditions that occur in parallel event processing:
    // 1. The exact same events processed by a given thread in one invocation will not
    //    neccessarily be the combination of events any thread sees in a subsequest
    //    invocation. Thus, you can't rely on thread_local storage.
    // 2. Its possible for the factory execution order to be modified by the presence of
    //    a processor (e.g. monitoring plugin). This is not as serious since changing the
    //    command line should cause one not to expect reproducibility. Still, one may
    //    expect the inclusion of an "observer" plugin not to have such side affects.
    //
    // More information will be needed. In the meantime, we implement a local random number
    // generator. Ideally, this would be seeded with the run number+event number, but for
    // now, just use default values defined in header file.


    m_logger=logger;
    // unitless conversion, keep consistency with juggler internal units (GeV, mm, ns, rad)
    minClusterHitEdep    = m_minClusterHitEdep / GeV;
    minClusterCenterEdep = m_minClusterCenterEdep / GeV;
    sectorDist           = m_sectorDist / mm;

    static std::map<std::string,
                std::tuple<std::function<edm4hep::Vector2f(const CaloHit*, const CaloHit*)>, std::vector<double>>>
    distMethods{
        {"localDistXY", {localDistXY, {mm, mm}}},        {"localDistXZ", {localDistXZ, {mm, mm}}},
        {"localDistYZ", {localDistYZ, {mm, mm}}},        {"dimScaledLocalDistXY", {dimScaledLocalDistXY, {1., 1.}}},
        {"globalDistRPhi", {globalDistRPhi, {mm, rad}}}, {"globalDistEtaPhi", {globalDistEtaPhi, {1., rad}}}
    };


    // set coordinate system
    auto set_dist_method = [this](std::pair<std::string, std::vector<double>> uprop) {
      if (uprop.second.size() == 0) {
        return false;
      }
      auto& [method, units] = distMethods[uprop.first];
      if (uprop.second.size() != units.size()) {
        //LOG_INFO(default_cout_logger) << units.size() << LOG_END;
        m_logger->info("units.size() = {}", units.size());
        //LOG_WARN(default_cout_logger) << fmt::format("Expect {} values from {}, received {}. ignored it.", units.size(), uprop.first,  uprop.second.size())  << LOG_END;
        m_logger->warn("Expect {} values from {}, received {}. ignored it.", units.size(), uprop.first,  uprop.second.size());
        return false;
      } else {
        for (size_t i = 0; i < units.size(); ++i) {
          neighbourDist[i] = uprop.second[i] / units[i];
        }
        hitsDist = method;
        //LOG_INFO(default_cout_logger) << fmt::format("Clustering uses {} with distances <= [{}]", uprop.first, fmt::join(neighbourDist, ",")) << LOG_END;
        m_logger->info("Clustering uses {} with distances <= [{}]", uprop.first, fmt::join(neighbourDist, ","));
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

//    std::vector<std::vector<double>> uprops{
//        u_localDistXY,
//        u_localDistXZ,
//        u_localDistYZ,
//        u_globalDistRPhi,
//        u_globalDistEtaPhi,
//        // default one should be the last one
//        u_dimScaledLocalDistXY,
//    };

    bool method_found = false;
    for (auto& uprop : uprops) {
      if (set_dist_method(uprop)) {
        method_found = true;
        break;
      }
    }
    if (not method_found) {
        //LOG_ERROR(default_cerr_logger) << "Cannot determine the clustering coordinates" << LOG_END;
        m_logger->error("Cannot determine the clustering coordinates");
        japp->Quit();
        return;
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

      if (m_logger->level() <=spdlog::level::debug){//msgLevel(MSG::DEBUG)) {
        const auto& hit = hits[i];
        //LOG_INFO(default_cout_logger) << fmt::format("hit {:d}: energy = {:.4f} MeV, local = ({:.4f}, {:.4f}) mm, global=({:.4f}, {:.4f}, {:.4f}) mm", i, hit->getEnergy() * 1000., hit->getLocal().x, hit->getLocal().y, hit->getPosition().x,  hit->getPosition().y, hit->getPosition().z) << LOG_END;
        m_logger->info("hit {:d}: energy = {:.4f} MeV, local = ({:.4f}, {:.4f}) mm, global=({:.4f}, {:.4f}, {:.4f}) mm", i, hit->getEnergy() * 1000., hit->getLocal().x, hit->getLocal().y, hit->getPosition().x,  hit->getPosition().y, hit->getPosition().z);
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
      //TODO: use proper logger

      if (m_logger->level() <=spdlog::level::debug){//msgLevel(MSG::DEBUG)) {
        //LOG_INFO(default_cout_logger) << "hits in a group: " << group.size() << ", " << "local maxima: " << maxima.size() << LOG_END;
        m_logger->info("hits in a group: {}, local maxima: {}", group.size(), maxima.size());
      }
    }

    return;

}
