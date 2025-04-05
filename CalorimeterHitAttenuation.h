// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <DD4hep/IDDescriptor.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <stdint.h>
#include <string>
#include <string_view>
#include <functional>

#include "CalorimeterHitAttenuationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon{

	using CalorimeterHitAttenuationAlgorithm = algorithms::Algorithm<
					    algorithms::Input<edm4hep::SimCalorimeterHitCollection>,
					    algorithms::Output<edm4hep::SimCalorimeterHitCollection>>;

	class CalorimeterHitAttenuation : public CalorimeterHitAttenuationAlgorithm,
					  public WithPodConfig<CalorimeterHitAttenuationConfig>{

		public:
			CalorimeterHitAttenuation(std::string_view name) 
				: CalorimeterHitAttenuationAlgorithm{name, {"inputHitCollection"},
									   {"outputHitCollection"},
									    "Regroup the hits by particle, add up the hits if" 
									    "they have the same z-segmentation, and attenuate."} {}

			void init() final;
			void process (const Input&, const Output&) const final;

		private:
			uint64_t id_mask{0};

			dd4hep::IDDescriptor id_spec;

			const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();

			// to get the detector edge position for attenuation	
			double z_edge;

		private:
			edm4hep::MCParticle get_primary(const edm4hep::CaloHitContribution& contrib) const;

			// attenuation function
			double get_attenuation(double zpos) const;
	};

} // namespace eicrecon
