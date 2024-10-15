// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#include "SiliconTrackerDigi.h"

#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <gsl/pointers>
#include <unordered_map>
#include <utility>

#include "algorithms/digi/SiliconTrackerDigiConfig.h"

namespace eicrecon {

void SiliconTrackerDigi::init(const dd4hep::rec::CellIDPositionConverter* converter) {
    // Create random gauss function
    m_gauss = [&](){
        return m_random.Gaus(0, m_cfg.timeResolution);
        //return m_rng.gaussian<double>(0., m_cfg.timeResolution);
    };

    m_converter = converter;
}


void SiliconTrackerDigi::process(
        const SiliconTrackerDigi::Input& input,
        const SiliconTrackerDigi::Output& output) const {

    const auto [sim_hits] = input;
    auto [raw_hits,associations] = output;

    // A map of unique cellIDs with temporary structure RawHit
    std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit> cell_hit_map;


    for (const auto& sim_hit : *sim_hits) {

        // time smearing
        double time_smearing = m_gauss();
        double result_time = sim_hit.getTime() + time_smearing;
        auto hit_time_stamp = (std::int32_t) (result_time * 1e3);

        debug("--------------------");
        debug("Hit cellID   = {}", sim_hit.getCellID());
        debug("   position  = ({:.2f}, {:.2f}, {:.2f})", sim_hit.getPosition().x, sim_hit.getPosition().y, sim_hit.getPosition().z);
        debug("   xy_radius = {:.2f}", std::hypot(sim_hit.getPosition().x, sim_hit.getPosition().y));
        debug("   momentum  = ({:.2f}, {:.2f}, {:.2f})", sim_hit.getMomentum().x, sim_hit.getMomentum().y, sim_hit.getMomentum().z);
        debug("   edep = {:.2f}", sim_hit.getEDep());
        debug("   time = {:.4f}[ns]", sim_hit.getTime());
        debug("   particle time = {}[ns]", sim_hit.getMCParticle().getTime());
        debug("   time smearing: {:.4f}, resulting time = {:.4f} [ns]", time_smearing, result_time);
        debug("   hit_time_stamp: {} [~ps]", hit_time_stamp);


        if (sim_hit.getEDep() < m_cfg.threshold) {
            debug("  edep is below threshold of {:.2f} [keV]", m_cfg.threshold / dd4hep::keV);
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
            debug("  Hit already exists in cell ID={}, prev. hit time: {}", sim_hit.getCellID(), hit.getTimeStamp());

            // keep earliest time for hit
            auto time_stamp = hit.getTimeStamp();
            hit.setTimeStamp(std::min(hit_time_stamp, hit.getTimeStamp()));

            // sum deposited energy
            auto charge = hit.getCharge();
            hit.setCharge(charge + (std::int32_t) std::llround(sim_hit.getEDep() * 1e6));
        }
    }

    //Add some noise hits
    int num_noise_hits = 10; //Add a fixed number for now
    int noise_hits_counter(0);

    if(m_cfg.add_noise_hits){ //We are only doing this for BVTX right now

	while(noise_hits_counter < num_noise_hits){
	
		//System is fixed
		unsigned long sys_bits = 31;

		//Determine layer (0, 1, or 4)
		unsigned long layer_bits;
		auto layer_rand = m_random.Uniform();
		
		if( layer_rand < (36./(36.+48.+120.) ) 
			{ layer_bits = 0; }
		else if( layer_rand < (36.+48.) / (36.+48.+120.) ) 
			{ layer_bits = 1; } 
		else 
			{ layer_bits = 4; }

		//Determine module (integer from [0,127])
		unsigned long module_bits;
		module_bits = (unsigned long) std::floor( m_random.Uniform(0,128) );

		//Set sensor bits (always 1???)
		unsigned long sensor_bits = 1;
	
		//FIX ME: Determine xy pixel position within module
		unsigned long xy_bits = 0;		

		//FIX ME: Determine z pixel position within module
		unsigned long z_bits = 0;

		//Create CellID value
		unsigned long cell_id = (zbits<<48) + 
					(xybits<<32) + 
					(sensor_bits<<24) + 
					(module_bits<<12) + 
					(layer_bits<<8) + 
					(sys_bits<<0);
	}
    }
    //Finished adding noise hits

    for (auto item : cell_hit_map) {
        raw_hits->push_back(item.second);

        for (const auto& sim_hit : *sim_hits) {
          if (item.first == sim_hit.getCellID()) {
            // set association
            auto hitassoc = associations->create();
            hitassoc.setWeight(1.0);
            hitassoc.setRawHit(item.second);
#if EDM4EIC_VERSION_MAJOR >= 6
            hitassoc.setSimHit(sim_hit);
#else
            hitassoc.addToSimHits(sim_hit);
#endif
          }
        }

    }
}

} // namespace eicrecon
