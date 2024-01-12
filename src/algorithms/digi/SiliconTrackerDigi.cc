// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#include "SiliconTrackerDigi.h"

#include <Evaluator/DD4hepUnits.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <utility>

#include "algorithms/digi/SiliconTrackerDigiConfig.h"

namespace eicrecon {

void SiliconTrackerDigi::init(std::shared_ptr<spdlog::logger>& logger) {
    // set logger
    m_log = logger;

    // Create random gauss function
    m_gauss = [&](){
        return m_random.Gaus(0, m_cfg.timeResolution);
    };
}


std::unique_ptr<edm4eic::RawTrackerHitCollection>
SiliconTrackerDigi::process(const edm4hep::SimTrackerHitCollection& sim_hits) {

    auto raw_hits { std::make_unique<edm4eic::RawTrackerHitCollection>() };

    // A map of unique cellIDs with temporary structure RawHit
    std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit> cell_hit_map;


    for (const auto& sim_hit : sim_hits) {

        // time smearing
        double time_smearing = m_gauss();
        double result_time = sim_hit.getTime() + time_smearing;
        auto hit_time_stamp = (std::int32_t) (result_time * 1e3);

        m_log->debug("--------------------");
        m_log->debug("Hit cellID   = {}", sim_hit.getCellID());
        m_log->debug("   position  = ({:.2f}, {:.2f}, {:.2f})", sim_hit.getPosition().x, sim_hit.getPosition().y, sim_hit.getPosition().z);
        m_log->debug("   xy_radius = {:.2f}", std::hypot(sim_hit.getPosition().x, sim_hit.getPosition().y));
        m_log->debug("   momentum  = ({:.2f}, {:.2f}, {:.2f})", sim_hit.getMomentum().x, sim_hit.getMomentum().y, sim_hit.getMomentum().z);
        m_log->debug("   edep = {:.2f}", sim_hit.getEDep());
        m_log->debug("   time = {:.4f}[ns]", sim_hit.getTime());
        m_log->debug("   particle time = {}[ns]", sim_hit.getMCParticle().getTime());
        m_log->debug("   time smearing: {:.4f}, resulting time = {:.4f} [ns]", time_smearing, result_time);
        m_log->debug("   hit_time_stamp: {} [~ps]", hit_time_stamp);


        if (sim_hit.getEDep() < m_cfg.threshold) {
            m_log->debug("  edep is below threshold of {:.2f} [keV]", m_cfg.threshold / dd4hep::keV);
            continue;
        }

        if (cell_hit_map.count(sim_hit.getCellID()) == 0) {
            // This cell doesn't have hits
            cell_hit_map[sim_hit.getCellID()] = {
                sim_hit.getCellID(),
                (std::int32_t) std::llround(sim_hit.getEDep() * 1e6),
                hit_time_stamp  // ns->ps
            };
        } else {
            // There is previous values in the cell
            auto& hit = cell_hit_map[sim_hit.getCellID()];
            m_log->debug("  Hit already exists in cell ID={}, prev. hit time: {}", sim_hit.getCellID(), hit.getTimeStamp());

            // keep earliest time for hit
            auto time_stamp = hit.getTimeStamp();
            hit.setTimeStamp(std::min(hit_time_stamp, hit.getTimeStamp()));

            // sum deposited energy
            auto charge = hit.getCharge();
            hit.setCharge(charge + (std::int32_t) std::llround(sim_hit.getEDep() * 1e6));
        }
    }

    for (auto item : cell_hit_map) {
        raw_hits->push_back(item.second);
    }

    return std::move(raw_hits);
}

} // namespace eicrecon
