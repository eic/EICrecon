// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#include "SiliconTrackerDigi.h"

#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <vector>
#include <cmath>
#include <cstdint>
#include <gsl/pointers>
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
        //return m_rng.gaussian<double>(0., m_cfg.timeResolution);
    };
}

// Logic: timeResolution is the smearing of the time.
// Smeared time marks the beginning of a bucket. If another hit falls within the same bucket,
// the energy is summed up. If not, a new hit is created.

void SiliconTrackerDigi::process(
    const SiliconTrackerDigi::Input& input,
    const SiliconTrackerDigi::Output& output) const {

    const auto [sim_hits] = input;
    auto [raw_hits,associations] = output;

    // A map of unique cellIDs with temporary structure RawHit
    std::unordered_map<std::uint64_t, std::vector<edm4eic::MutableRawTrackerHit>> cell_hit_map;

    auto bucket_width = (std::int32_t) (m_cfg.integrationWindow * 1e3);

    // Some detectors have a steady pulse of hits - generate empty time buckets for those
    // Conundrum: We can't know the number of buckets without knowing the timeSlice length
    // Assuming this is only used for MAPS, and that the time slice length is <= 1ms,
    // we need at most 1ms / 2000ns = 500 buckets
    // Let's use 1000, and pad with over and underflow to allow smearing out of range
    if ( m_cfg.prepopulate ) {
        for (auto cell : cell_hit_map) {
            auto& hits = cell.second;
            for (int i = -1; i <= 1001; i++) {
                hits.push_back(
                    edm4eic::MutableRawTrackerHit{
                    cell.first,
                    0,
                    0
                } );
            }
        }
    }
    
    for (const auto& sim_hit : *sim_hits) {
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

        bool bucket_found = false;
        if (cell_hit_map.count(sim_hit.getCellID()) == 1) {
            // Update an existing hit?
            for (auto& hit : cell_hit_map[sim_hit.getCellID()]) {
                auto existing_time = hit.getTimeStamp();
                // TODO: edge cases?
                if ( hit_time_stamp >= existing_time && hit_time_stamp <= existing_time + bucket_width ) {
                    // There is already a hit within the same time window
                    m_log->debug("  Hit already exists in cell ID={}, within the same time bucket. Time stamp: {}, bucket from {} to {}",
                         sim_hit.getCellID(), hit.getTimeStamp(), existing_time, existing_time + bucket_width);
                    // sum deposited energy
                    auto charge = hit.getCharge();
                    hit.setCharge(charge + (std::int32_t) std::llround(sim_hit.getEDep() * 1e6));
                    bucket_found = true;
                    break;
                } // time bucket found
            } // loop over existing hits
        } // cellID found

        if (!bucket_found) {
            // There is no hit in the same time bucket
            m_log->debug("  No pre-existing hit in cell ID={} in the same time bucket. Time stamp: {}",
                        sim_hit.getCellID(), sim_hit.getTime());

            // Create a new hit
            // Note: time uncertainty in the TrackerHitReconstruction is set independently
            // It would probably be better to move it into the RawTrackerHit class
            // (same for spatial uncertainty, actually)
            // Note 2: It's possible to not fall into a bucket but still be close enough to one or
            // more that uncertainties overlap. Cannot be avoided in the current setup.
            // It could lead to ambiguity which bucket is chosen for a third hit in this area.
            // In reality, this is probably more like dead time; revisit later.
            cell_hit_map[sim_hit.getCellID()].push_back(
                edm4eic::MutableRawTrackerHit{
                sim_hit.getCellID(),
                (std::int32_t) std::llround(sim_hit.getEDep() * 1e6),
                hit_time_stamp
            } );
        } // bucket found
    } // loop over sim hits

    for (auto cell : cell_hit_map) {
        for (auto& hit : cell.second) {
            raw_hits->push_back(hit);

            for (const auto& sim_hit : *sim_hits) {
                if (cell.first == sim_hit.getCellID()) {
                    // set association
                    auto hitassoc = associations->create();
                    hitassoc.setWeight(1.0);
                    hitassoc.setRawHit(hit);
#if EDM4EIC_VERSION_MAJOR >= 6
                    hitassoc.setSimHit(sim_hit);
#else
                    hitassoc.addToSimHits(sim_hit);
#endif
                } // if cellID matches
            } // sim_hits
        } //hits
    } // cell_hit_map


} // process

} // namespace eicrecon
