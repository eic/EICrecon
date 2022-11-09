// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <spdlog/spdlog.h>
#include <TGeoSystemOfUnits.h>
#include <edm4hep/MCParticle.h>
#include "SiliconTrackerDigi.h"


void eicrecon::SiliconTrackerDigi::init(std::shared_ptr<spdlog::logger>& logger) {
    // set logger
    m_log = logger;

    // Create random gauss function
    m_gauss = [&](){
        return m_random.Gaus(0, m_cfg.timeResolution);
    };
}


std::vector<edm4eic::RawTrackerHit *>
eicrecon::SiliconTrackerDigi::produce(const std::vector<const edm4hep::SimTrackerHit *>& sim_hits) {
    /** Event by event processing **/
    namespace units = TGeoUnit;

    // A map of unique cellIDs with temporary structure RawHit
    struct RawHit {
        std::int32_t charge;
        std::int32_t time_stamp;
    };
    std::unordered_map<std::uint64_t, RawHit> cell_hit_map;


    for (const auto sim_hit : sim_hits) {

        // time smearing
        double time_smearing = m_gauss();
        double result_time = sim_hit->getTime() + time_smearing;
        auto hit_time_stamp = (std::int32_t) (result_time * 1e3);

        m_log->debug("--------------------");
        m_log->debug("Hit cellID   = {}", sim_hit->getCellID());
        m_log->debug("   position  = ({:.2f}, {:.2f}, {:.2f})", sim_hit->getPosition().x, sim_hit->getPosition().y, sim_hit->getPosition().z);
        m_log->debug("   xy_radius = {:.2f}", std::hypot(sim_hit->getPosition().x, sim_hit->getPosition().y));
        m_log->debug("   momentum  = ({:.2f}, {:.2f}, {:.2f})", sim_hit->getMomentum().x, sim_hit->getMomentum().y, sim_hit->getMomentum().z);
        m_log->debug("   edep = {:.2f}", sim_hit->getEDep());
        m_log->debug("   time = {:.4f}[ns]", sim_hit->getTime());
        m_log->debug("   particle time = {}[ns]", sim_hit->getMCParticle().getTime());
        m_log->debug("   time smearing: {:.4f}, resulting time = {:.4f} [ns]", time_smearing, result_time);
        m_log->debug("   hit_time_stamp: {} [~ps]", hit_time_stamp);


        double edep = sim_hit->getEDep();
        if (edep * units::keV < m_cfg.threshold) {
            m_log->debug("  edep is below threshold of {:.2f} [keV]", m_cfg.threshold / units::keV);
            continue;
        }

        if (cell_hit_map.count(sim_hit->getCellID()) == 0) {
            // This cell doesn't have hits
            cell_hit_map[sim_hit->getCellID()] = {
                    (std::int32_t) std::llround(sim_hit->getEDep() * 1e6),
                    hit_time_stamp}; // ns->ps
        } else {
            // There is previous values in the cell
            RawHit &prev_hit = cell_hit_map[sim_hit->getCellID()];
            m_log->debug("  Hit already exists in cell ID={}, prev. hit time: {}", sim_hit->getCellID(), prev_hit.time_stamp);
            prev_hit.time_stamp = std::min(hit_time_stamp, prev_hit.time_stamp);  // keep earliest time for hit
            prev_hit.charge += (std::int32_t) std::llround(sim_hit->getEDep() * 1e6);
        }
    }

    // Create and fill output array
    std::vector<edm4eic::RawTrackerHit*> rawhits;
    for (auto item : cell_hit_map) {
        rawhits.push_back(new edm4eic::RawTrackerHit(
                item.first,
                item.second.charge,
                item.second.time_stamp));
    }

    return rawhits;
}


