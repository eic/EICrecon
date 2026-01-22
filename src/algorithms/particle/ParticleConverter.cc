#include <edm4eic/ReconstructedParticleCollection.h>

#include "ParticleConverter.h"

namespace eicrecon {
        void ParticleConverter::process(const Input& input, const Output& output) const {
                // - Here I have to write all the necessary instructions to output a ReconstructedParticlesCollection
                // Check Derek's PR where he conveyed the way to loop through clusters
                // How to associate tracks to clusters?
                //      Check the track cluster associators
                //      Should I just follow the same guidelines as there?
                
                const auto [in_particles] = input;
                auto [out_particles]      = output;

                // out_particles->setSubsetCollection(); // CHECK: IN THIS REALLY NECESSARY ? (ASK)

                // 1 - Assign prelim PID
                //      a - If there is an associated track assign PDG from track bank
                //      b - Loop through Clusters
                //              b.1 - If ECal and !HCal -> Photon
                //              b.2 - If !Ecal and Hcal -> Neutron

                trace("----------------------------------------------------------------");
                trace("                                                                ");
                trace("We have {} particles as input", in_particles->size());

                for (auto particle : *in_particles) {
                        double prelim_pid = -9999;

                        // std::cout<<"This particle PID="<<particle.getPDG()<<std::endl;
                        trace("Particle PID     = {}", particle.getPDG());
                        trace("Particle charge  = {}", particle.getCharge());
                        trace("Particle mass    = {}", particle.getMass());
                        trace("Particle energy  = {}", particle.getEnergy());
                        trace("Particle goodPID = {}", particle.getGoodnessOfPID());


                        // Check if there are associated tracks to this particle
                        for (auto track : particle.getTracks()) {
                                trace("         AssocTrack PID    ={}", track.getPdg());
                                trace("         AssocTrack charge ={}", track.getCharge());
                        }                
                                // std::cout<<"The associated tracks PIDs="<<track.getPdg()<<std::endl; // prelim_pid = track.getPdg();

                        // // If there was no associated track
                        // if (prelim_pid == -9999) {
                        //         // Check clusters ...
                        // }

                        // double energy = -9999;

                        // if
                        trace("                                                                ");
                }

                trace("                                                                ");
                trace("----------------------------------------------------------------");
                // 2 - Calculate track energy


                // 3 - Calculate calo energy


                // (OPT) - If true, use energy resolution


                // 4 - Calculate remaining kinematics and output a reco particle

        };

        // void ParticleConverter::init() {};
}