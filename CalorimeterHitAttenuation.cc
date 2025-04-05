// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#include "CalorimeterHitAttenuation.h"

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DD4hep/config.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/service.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <limits>
#include <stdexcept>
#include <string>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitAttenuationConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

using namespace dd4hep;

namespace eicrecon{

	void CalorimeterHitAttenuation::init(){

		// readout checks
                if (m_cfg.readout.empty()) {
                        error("readoutClass is not provided, it is needed to know the fields in readout ids");
                        throw std::runtime_error("readoutClass is not provided");
                }

                // get decoders
                try {
                        id_spec = m_geo.detector()->readout(m_cfg.readout).idSpec();
                } catch (...) {
                        debug("Failed to load ID decoder for {}", m_cfg.readout);
                        throw std::runtime_error(fmt::format("Failed to load ID decoder for {}", m_cfg.readout));
                }

		 decltype(id_mask) id_inverse_mask = 0;
                // get id_mask for adding up hits that have the same z-segmentation
		if (!m_cfg.zField.empty()) {
			id_inverse_mask |= id_spec.field(m_cfg.zField)->mask();
			debug("ID mask in {:s}: {:#064b}", m_cfg.readout, id_mask);
                }
                id_mask = ~id_inverse_mask;

		z_edge = m_geo.detector()->constant<double>(m_cfg.lengthField) / dd4hep::cm;
	}

	void CalorimeterHitAttenuation::process(const CalorimeterHitAttenuation::Input& input,
                        			const CalorimeterHitAttenuation::Output& output) const{

		const auto [in_hits] = input;
		auto [out_hits] = output;

		// map for regrouping
		std::map<edm4hep::MCParticle, std::vector<edm4hep::SimCalorimeterHit>> mapMCParToSimCalHit;

		// regroup the sim hits by mc particle
		for(const auto &ih : *in_hits){
			for(const auto &contrib : ih.getContributions()){
				edm4hep::MCParticle primary = get_primary(contrib);

				edm4hep::MutableSimCalorimeterHit simhit;
				simhit.setCellID(ih.getCellID());
				simhit.setEnergy(contrib.getEnergy());
				simhit.setPosition(ih.getPosition());
				simhit.addToContributions(contrib);
			
				mapMCParToSimCalHit[primary].push_back(simhit);

				trace("Identified primary: id = {}, pid = {}, total energy = {}, contributed = {}",
				      primary.getObjectID().index, 
				      primary.getPDG(), 
				      primary.getEnergy(), 
				      mapMCParToSimCalHit[primary].back().getEnergy());
			}
		}

		// Attenuate energies of the sim hits
		// 1. sum the hits if they have the same z-segmentation
		// 2. attenuate the summed hits 	
		for(const auto &[par, hits] : mapMCParToSimCalHit){
			std::unordered_map<uint64_t, std::vector<std::size_t>> merge_map;	

			// map for adding up the hits that have the same z-segmentation
			std::size_t ix = 0;
			for(const auto &ahit : hits){
				uint64_t hid = ahit.getCellID() & id_mask;

				trace("org cell ID in {:s}: {:#064b}", m_cfg.readout, ahit.getCellID());
				trace("new cell ID in {:s}: {:#064b}", m_cfg.readout, hid);

				merge_map[hid].push_back(ix);
				ix++;
			}

			for(const auto &[id, ixs] : merge_map){
				float edepSum = 0;
				float timeEar = std::numeric_limits<double>::max();
				auto leading_hit = hits[0];
				auto leading_contrib = hits[0].getContributions(0);

				for(size_t i=0; i<ixs.size(); ++i){
					auto hit = hits[i];
					edepSum += hit.getEnergy();

					for (const auto& c : hit.getContributions()){
						if (c.getTime() <= timeEar) {
							timeEar = c.getTime();
						}
					}
				}

				// attenuation
				float attFactor = get_attenuation(leading_hit.getPosition().z);
				trace("z = {}, attFactor = {}", leading_hit.getPosition().z, attFactor);

				edm4hep::CaloHitContributionCollection contribs;
				auto contrib = contribs.create();
				contrib.setPDG(leading_contrib.getPDG());
				contrib.setEnergy(leading_contrib.getEnergy());
				contrib.setTime(timeEar);
				contrib.setStepPosition(leading_contrib.getStepPosition());
				contrib.setParticle(par);

				edm4hep::MutableSimCalorimeterHit out_hit;
				out_hit.setCellID(leading_hit.getCellID());
				out_hit.setEnergy(edepSum*attFactor);
				out_hit.setPosition(leading_hit.getPosition());
				out_hit.addToContributions(contrib);
				out_hits->push_back(out_hit);
			}
		}
	}

	edm4hep::MCParticle CalorimeterHitAttenuation::get_primary(const edm4hep::CaloHitContribution& contrib) const{
		const auto contributor = contrib.getParticle();

		edm4hep::MCParticle primary = contributor;
		while (primary.parents_size() > 0) {
			if (primary.getGeneratorStatus() != 0) break;
			primary = primary.getParents(0);
		}
		return primary;
	}

	double CalorimeterHitAttenuation::get_attenuation(double zpos) const{
		auto length = std::abs(z_edge - 0.1*zpos);
		auto factor = m_cfg.attPars[0]*std::exp(-length/m_cfg.attPars[1]) + (1-m_cfg.attPars[0])*std::exp(-length/m_cfg.attPars[2]);
		return factor;
	}
}
