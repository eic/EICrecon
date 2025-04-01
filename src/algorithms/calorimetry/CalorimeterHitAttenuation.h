// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <Parsers/Primitives.h>
#include <stddef.h>
#include <gsl/pointers>
#include <random>
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
									   "Attenuate hits."} {}

			void init() final;
			void process (const Input&, const Output&) const final;

		private:
			dd4hep::IDDescriptor id_spec;
	



	};

} // namespace eicrecon
