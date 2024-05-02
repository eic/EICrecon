// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei

#pragma once

#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/utils/vector_utils.h>

#include "services/pid_lut/PIDLookupTable_service.h"
#include "extensions/jana/JOmniFactory.h"

#include <random>

namespace eicrecon {

struct PIDLookupTableConfig {
    std::string filename;
};

class PIDLookupTable_factory : public JOmniFactory<PIDLookupTable_factory, PIDLookupTableConfig> {

private:
    PodioInput<edm4eic::ReconstructedParticle> m_recoparticles_in {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_recoparticles_out {this};

    ParameterRef<std::string> m_filename {this, "filename", config().filename, "Relative to current working directory"};
    Service<PIDLookupTable_service> m_lut_svc {this};

    std::mt19937 m_gen;
    std::uniform_real_distribution<double> m_dist {0, 1};
    const PIDLookupTable* m_lut;

public:
    void Configure() {
        m_lut = m_lut_svc().load(m_filename());
        if (!m_lut) {
            throw std::runtime_error("LUT table not available");
        }
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {

        // TODO: This is all very handwavy because I haven't been attending the PID datamodel discussions.
        // Please look over this carefully and correct as needed!

        for (const auto& recopart_without_pid : *m_recoparticles_in()) {

            auto recopart = recopart_without_pid.clone();

            int pdg = recopart.getPDG();
            int charge = recopart.getCharge();
            double momentum = edm4hep::utils::magnitude(recopart.getMomentum());

            // TODO: I'm still confused as to whether our lookup table actually contains eta vs theta.
            double eta = edm4hep::utils::eta(recopart.getMomentum());
            double theta = edm4hep::utils::anglePolar(recopart.getMomentum());
            double phi = edm4hep::utils::angleAzimuthal(recopart.getMomentum());

            auto entry = m_lut->Lookup(pdg, charge, momentum, eta, phi);

            int identified_pdg = 0; // unknown

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
                    identified_pdg = 0; // unknown
                    // If the lookup table contains rows where all probabilities are zero, the control flow ends up here
                }
                if (charge < 0) {
                    identified_pdg *= -1;
                    // We want the identified PDG to have the same sign as the charge
                }
            }

            recopart.setPDG(identified_pdg);

            m_recoparticles_out()->push_back(recopart);
        }
    }
};

} // eicrecon
