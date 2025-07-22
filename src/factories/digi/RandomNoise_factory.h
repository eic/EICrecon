// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 EIC-FT

#pragma once

#include "algorithms/digi/RandomNoise.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

/**
 * @brief JANA factory for the RandomNoise algorithm.
 *
 * This factory creates and configures the RandomNoise algorithm,
 * connecting it to the necessary geometry service and managing its
 * input and output data collections.
 */
class RandomNoise_factory
    : public JOmniFactory<RandomNoise_factory, RandomNoiseConfig> {

public:
    using AlgoT = eicrecon::RandomNoise;

private:
    std::unique_ptr<AlgoT> m_algo;

    // Input collection of raw tracker hits. The name "inputRawHitCollection" is
    // automatically inferred from the algorithm's definition.
    PodioInput<edm4eic::RawTrackerHit> m_in_hits{this};

    // Output collection of raw tracker hits with noise added.
    PodioOutput<edm4eic::RawTrackerHit> m_out_hits{this};

    ParameterRef<bool> m_addNoise{this, "addNoise", config().addNoise};
    ParameterRef<int> m_n_noise_hits_per_system{this, "n_noise_hits_per_system", config().n_noise_hits_per_system};
    ParameterRef<std::string> m_readout_name{this, "readout_name", config().readout_name};
    // Service for accessing detector geometry information.
    Service<DD4hep_service> m_geoSvc{this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
        m_algo->applyConfig(config());
        m_algo->init(m_geoSvc().detector());
        if (!m_in_hits.collection_names.empty()) {
            m_algo->setInputCollectionName(m_in_hits.collection_names[0]);
        }
    }

    void Process(int32_t, uint64_t) {
        m_algo->process({m_in_hits()}, {m_out_hits().get()});
    }
};

} // namespace eicrecon