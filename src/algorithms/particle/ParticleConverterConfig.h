// author : Esteban Molina, Derek Anderson
// date   : January 2026

#pragma once

namespace eicrecon {
        struct ParticleConverterConfig {
                double tracking_resolution = 1;
                
                double ecal_resolution  = 1;
                double hcal_resolution  = 1;
                double calo_energy_norm = 1;
                
                bool use_resolution_in_ecalc = false;
        };
}