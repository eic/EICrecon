// Copyright 2024, Nathan Brei
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include "services/pid_lut/PIDLookupTable_service.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

struct PIDLookupTableConfig {
    std::string filename = "hpdirc_positive.lut";
}

class PIDLookupTable_factory : public JOmniFactory<PIDLookupTable_factory, PIDLookupTableConfig> {

private:
    PodioInput<edm4hep::MCParticle> m_particles_in {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_recoparticles_out {this};

    ParameterRef<std::string> m_filename {this, "filename", config().filename, "Filename for LUT (relative to CWD)"};
    Service<PIDLookupTable_service> m_lut_svc {this};

    std::mt19937 m_rng;
    std::uniform_real_distribution m_dist(0,1);
    PIDLookupTable* m_lut;

public:
    void Configure() {
        m_lut = m_lut_svc()->FetchTable(m_filename());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {

        // TODO: This is all very handwavy because I haven't been attending the PID datamodel discussions.
        // Please look over this carefully and correct as needed!
        
        for (const auto& mcparticle : m_particles_in()) {

            // Unpack lookup values from input
            
            int pdg = mcparticle.getPDG();
            int charge = mcparticle.getCharge();
            double momentum = edm4hep::utils::magnitude(mcparticle.getMomentum());
            double eta = edm4hep::utils::eta(mcparticle.getVertex());
            double phi = edm4hep::utils::phi(mcparticle.getVertex());
            
            auto entry = m_lut->Lookup(pdg, charge, momentum, eta, phi);

            int identified_pdg = -1; // unknown
                                     // TODO: What is the PDG for 'unknown', actually?
            
            if (entry != nullptr) {

                m_gen.seed(event_number);
                double random_unit_interval = m_dist(m_gen);

                if (random_unit_interval < entry->prob_electron) {
                    identified_pdg = 11; // electron
                }
                else if (random_unit_interval < (entry->prob_electron + entry->prob_pion)) {
                    identified_pdg = 211; // pion
                }
                else if (random_unit_interval < (entry->prob_electron + entry->prob_pion + entry->prob_kaon)) {
                    identified_pdg = 321; // kaon
                }
                else if (random_unit_interval < (entry->prob_electron + entry->prob_pion + entry->prob_kaon + entry->prob_electron)) {
                    identified_pdg = 2212; // proton
                }
                else {
                    identified_pdg = -1; // unknown
                    // TODO: Note that the lookup tables contains entries where all values are zero.
                    // These end up here
                }
            }

            m_recoparticles_out() = std::make_unique<edm4eic::ReconstructedParticleCollection>();
            auto recopart = m_recoparticles_out()->create();

            recopart.setPDG(identified_pdg);
            // TODO: Set other fields?
            // TODO: Association with MCParticle?
        }
    }
};

} // eicrecon
