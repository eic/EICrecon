// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include <string>

#include "algorithms/fardetectors/TorchScriptInterface.h"
#include "algorithms/fardetectors/TorchScriptInterfaceConfig.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class TorchScriptInterface_factory :
    public JOmniFactory<TorchScriptInterface_factory, TorchScriptInterfaceConfig> {

public:
    using AlgoT = eicrecon::TorchScriptInterface;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::TrackerHit> m_in_reco_particles {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_out_reco_particles {this};

    Service<DD4hep_service> m_geoSvc {this};

    ParameterRef<string> model_x_file_path{this, "model_x_file_path", config().model_x_file_path};
    ParameterRef<string> model_y_file_path{this, "model_y_file_path", config().model_y_file_path};
    ParameterRef<string> model_z_file_path{this, "model_z_file_path", config().model_z_file_path};


public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>();
        m_algo->init(logger());
        m_algo->applyConfig(config());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        auto output = m_algo->execute(
          m_in_reco_particles()
        );
        m_out_reco_particles() = std::move(output);
    }

};

} // eicrecon
