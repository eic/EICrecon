// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include "algorithms/calorimetry/CalorimeterHitAttenuation.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

	class CalorimeterHitAttenuation_factory : public JOmniFactory<CalorimeterHitAttenuation_factory, CalorimeterHitAttenuationConfig> {

		public:
			using AlgoT = eicrecon::CalorimeterHitAttenuation;
		private:
			std::unique_ptr<AlgoT> m_algo;

			PodioInput<edm4hep::SimCalorimeterHit> m_hits_input {this};
			PodioOutput<edm4hep::SimCalorimeterHit> m_hits_output {this};

			ParameterRef<std::vector<double>> m_attenuationParameters {this, "attenuationParameters", config().attPars};
			ParameterRef<std::vector<double>> m_layerParameters {this, "layerParameters", config().layPars};
			ParameterRef<std::vector<std::string>> m_fields {this, "fields", config().fields};
			ParameterRef<std::string> m_readout {this, "readout", config().readout};

			Service<AlgorithmsInit_service> m_algorithmsInit {this};

		public:
			void Configure() {
				m_algo = std::make_unique<AlgoT>(GetPrefix());
				m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
				m_algo->applyConfig(config());
				m_algo->init();
			}

			void ChangeRun(int64_t run_number) {
			}

			void Process(int64_t run_number, uint64_t event_number) {
				m_algo->process({m_hits_input()}, {m_hits_output().get()});
			}
	};

} // eicrecon
