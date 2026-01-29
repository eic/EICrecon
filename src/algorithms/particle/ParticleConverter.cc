#include <DD4hep/Detector.h>
#include <DD4hep/DetType.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/ReconstructedParticle.h>

#include "ParticleConverter.h"

namespace eicrecon {
        void ParticleConverter::process(const Input& input, const Output& output) const {
                const auto [in_particles] = input;
                auto      [out_particles] = output;

                trace("----------------------------------------------------------------");
                trace("                                                                ");
                trace("We have {} particles as input", in_particles->size());

                for (const auto particle : *in_particles) {
                        bool   isTrack = false;
                        bool   isHCal  = false;
                        bool   isECal  = false;

                        double prelim_pid   = 0;
                        
                        double track_energy = 0;
                        double track_mass   = 0;
                        
                        double calo_energy  = 0;
                        double ecal_energy  = 0;
                        double hcal_energy  = 0;

                        double avge_energy = 0;
                        
                        edm4hep::Vector3f track_momentum_vector;

                        // Looking for tracks
                        for (auto track : particle.getTracks()) {
                                isTrack = true;

                                // trace("Track found!");
                                
                                prelim_pid = particle.getPDG();
                                track_mass = particle.getMass();
                                track_momentum_vector = track.getMomentum();
                                
                                // if (!particle.getPDG())
                                //         trace("Particle with associated track.PDG = {}, particle.PID = {},particle.Energy = {}", track.getPdg(), particle.getPDG(), particle.getEnergy());
                        }

                        // Looking for clusters
                        for (auto cluster : particle.getClusters()) {
                                for (auto calo_hit : cluster.getHits()) {
                                        const auto cell_id = calo_hit.getCellID();

                                        const dd4hep::VolumeManagerContext* context = m_converter->findContext(cell_id);

                                        if (context) {
                                                const dd4hep::DetElement det_element = context->element;
                                                const dd4hep::DetType    type(det_element.typeFlag());

                                                std::string det_element_type = det_element.type();

                                                // Note: will probably change
                                                if (det_element_type.find(ecal_string) != std::string::npos){
                                                        isECal = true;

                                                        ecal_energy += calo_hit.getEnergy();
                                                }
                                                if (det_element_type.find(hcal_string) != std::string::npos){
                                                        isHCal = true;

                                                        hcal_energy += calo_hit.getEnergy();
                                                }
                                        }
                                }
                        }
                        
                        if (isECal && !isHCal) 
                                prelim_pid = 22; //photon
                        if (!isECal && isHCal)
                                prelim_pid = 2112; // neutron

                        // Step 2
                        //      Calculate energy for the track
                        double track_momentum_mag = 0;

                        if (isTrack) {
                                track_momentum_mag = std::sqrt(std::pow(track_momentum_vector.x, 2) + 
                                                               std::pow(track_momentum_vector.y, 2) + 
                                                               std::pow(track_momentum_vector.z, 2));

                                // trace("Associated track momentum mag = {}", track_momentum_mag);

                                track_energy = std::sqrt(std::pow(track_momentum_mag, 2) + std::pow(track_mass, 2));
                        }

                        // if (!particle.getPDG())
                        //         trace(" This particle track.Energy = {}, track.Mass = {}", track_energy, track_mass);

                        // Step 3 (PRELIMINARY IMPLEMENTATION)
                        //      Calculate calo energy
                        if (ecal_energy > 0)
                                calo_energy += ecal_energy;
                        if (hcal_energy > 0)
                                calo_energy += hcal_energy;

                        calo_energy *= m_cfg.calo_energy_norm;

                        // Step 4 (PRELIMINARY IMPLEMENTATION)
                        //      Calculate the average energy. One resolution for the whole cal system?
                        double weight_tracking_resolution = 1. / std::pow(m_cfg.tracking_resolution, 2);
                        double weight_calo_resolution     = 1. / std::pow(m_cfg.ecal_resolution, 2); // USING ECAL RESOLUTION AS PLACEHOLDER!

                        double normalization = 0;
                        
                        if (track_energy > 0 && calo_energy > 0)
                                normalization = weight_tracking_resolution + weight_calo_resolution;
                        else if (track_energy > 0 && calo_energy == 0)
                                normalization = weight_tracking_resolution;
                        else if (track_energy == 0 && calo_energy > 0)
                                normalization = weight_calo_resolution;

                        //  WARNING: must implement some sort of check in case we have energies that are too different
                        //           example: calo.Energy = 2.127329626586288, track.Energy = 0.0005109989433549345
                        avge_energy = (weight_tracking_resolution * track_energy + weight_calo_resolution * calo_energy) / normalization;

                        trace(" Total energy of the particle is E = {} GeV, calo.Energy = {}, track.Energy = {}", avge_energy, calo_energy, track_energy);
                        
                        trace("                                                                ");
                }

                trace("                                                                ");
                trace("----------------------------------------------------------------");
        };
}
