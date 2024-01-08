// Copyright (C) 2022, 2023 Daniel Brandenburg
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include "extensions/jana/JOmniFactory.h"

#include "algorithms/reco/ElectronReconstruction.h"


namespace eicrecon {

class ReconstructedElectrons_factory : public JOmniFactory<ReconstructedElectrons_factory> {
private:

    // Underlying algorithm
    std::unique_ptr<eicrecon::ElectronReconstruction> m_algo;

    // Declare inputs
    PodioInput<edm4hep::MCParticle> m_in_mc_particles {this, "MCParticles"};
    PodioInput<edm4eic::ReconstructedParticle> m_in_rc_particles {this, "ReconstructedChargedParticles"};
    PodioInput<edm4eic::MCRecoParticleAssociation> m_in_rc_particles_assoc {this, "ReconstructedChargedParticleAssociations"};

    VariadicPodioInput<edm4eic::MCRecoClusterParticleAssociation> m_in_clu_assoc {this};

    // Declare outputs 
    PodioOutput<edm4eic::ReconstructedParticle> m_out_reco_particles {this};

    // Declare parameters here, e.g.
    // ParameterRef<double> m_samplingFraction {this, "samplingFraction", config().sampFrac};
    // ParameterRef<std::string> m_energyWeight {this, "energyWeight", config().energyWeight};
    
    // Declare services here, e.g.
    // Service<DD4hep_service> m_geoSvc {this};

public:
    void Configure() {
        // This is called when the factory is instantiated. 
        // Use this callback to make sure the algorithm is configured.
        // The logger, parameters, and services have all been fetched before this is called
        m_algo = std::make_unique<eicrecon::ElectronReconstruction>();

        // If we had a config object, we'd apply it like so:
        // m_algo->applyConfig(config());

        // If we needed geometry, we'd obtain it like so
        // m_algo->init(m_geoSvc().detector(), m_geoSvc().converter(), logger());

        m_algo->init(logger());
    }

    void ChangeRun(int64_t run_number) {
        // This is called whenever the run number is changed. 
        // Use this callback to retrieve state that is keyed off of run number.
        // This state should usually be managed by a Service.
        // Note: You usually don't need this, because you can declare a Resource instead.
    }

    void Process(int64_t run_number, uint64_t event_number) {
        // This is called on every event.
        // Use this callback to call your Algorithm using all inputs and outputs
        // The inputs will have already been fetched for you at this point.
        auto output = m_algo->execute(
          m_in_mc_particles(),
          m_in_rc_particles(),
          m_in_rc_particles_assoc(),
          m_in_clu_assoc()
        );

        logger()->debug( "Event {}: Found {} reconstructed electron candidates", event_number, output->size() );

        m_out_reco_particles() = std::move(output);
        // JANA will take care of publishing the outputs for you.
    }
};
} // namespace eicrecon
