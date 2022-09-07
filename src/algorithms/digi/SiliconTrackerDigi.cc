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


std::vector<eicd::RawTrackerHit *>
eicrecon::SiliconTrackerDigi::produce(const std::vector<const edm4hep::SimTrackerHit *>& sim_hits) {
//return std::vector<eicd::RawTrackerHit *>();
    /** Event by event processing **/
    namespace units = TGeoUnit;

    // A map of unique cellIDs with temporary structure RawHit
    struct RawHit {
        std::int32_t charge;
        std::int32_t time_stamp;
    };
    std::unordered_map<std::uint64_t, RawHit> cell_hit_map;


    for (const auto sim_hit : sim_hits) {

        m_log->debug("--------------------");
        m_log->debug("Hit cellID   = {}", sim_hit->getCellID());
        m_log->debug("   position  = ({:.2f}, {:.2f}, {:.2f})", sim_hit->getPosition().x, sim_hit->getPosition().y, sim_hit->getPosition().z);
        m_log->debug("   xy_radius = {:.2f}", std::hypot(sim_hit->getPosition().x, sim_hit->getPosition().y));
        m_log->debug("   momentum  = ({:.2f}, {:.2f}, {:.2f})", sim_hit->getMomentum().x, sim_hit->getMomentum().y, sim_hit->getMomentum().z);
        m_log->debug("   edep = {:.2f}", sim_hit->getEDep());

        if (sim_hit->getEDep() * units::keV < m_cfg.threshold) {
            m_log->debug("  edep is below threshold of {:.2f} [keV]\n", m_cfg.threshold / units::keV);
            continue;
        }

        if (cell_hit_map.count(sim_hit->getCellID()) == 0) {
            // This cell doesn't have hits
            cell_hit_map[sim_hit->getCellID()] = {
                    (std::int32_t) (sim_hit->getMCParticle().getTime() * 1e6 + m_gauss() * 1e3), // ns->fs
                    (std::int32_t) std::llround(sim_hit->getEDep() * 1e6)};
        } else {
            // There is previous values in the cell
            RawHit &hit = cell_hit_map[sim_hit->getCellID()];
            hit.time_stamp = (std::int32_t) (sim_hit->getMCParticle().getTime() * 1e6 + m_gauss() * 1e3);
            hit.charge += (std::int32_t) std::llround(sim_hit->getEDep() * 1e6);
        }
    }

    // Create and fill output array
    std::vector<eicd::RawTrackerHit*> rawhits;
    for (auto item : cell_hit_map) {
        rawhits.push_back(new eicd::RawTrackerHit(
                item.first,
                item.second.charge,
                item.second.time_stamp));
    }

    return rawhits;
}


