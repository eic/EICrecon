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

                // setSubsetCollection it restricts the branches of the output (that is what I see!)
                // out_particles->setSubsetCollection(); // CHECK: IN THIS REALLY NECESSARY ? (ASK)

                trace("----------------------------------------------------------------");
                trace("                                                                ");
                trace("We have {} particles as input", in_particles->size());

                // Now we check particles and their current information
                for (const auto particle : *in_particles) {
                        bool   isTrack = false;
                        bool   isHCal  = false;
                        bool   isECal  = false;

                        double prelim_pid;
                        
                        for (auto track : particle.getTracks()) {
                                isTrack = true;
                                prelim_pid = particle.getPDG();

                                if (!particle.getPDG())
                                        trace("Particle with associated track PDG == 0");
                        }

                        // NOTE : be careful the cluster selected is NOT associated to a track
                        // NOTE1: prelim, just go through the clusters
                        if (!isTrack) {
                                for (auto cluster : particle.getClusters()) {
                                        for (auto calo_hit : cluster.getHits()) {
                                                const auto cell_id = calo_hit.getCellID();

                                                const dd4hep::VolumeManagerContext* context = m_converter->findContext(cell_id);

                                                if (context) {
                                                        const dd4hep::DetElement det_element = context->element;
                                                        const dd4hep::DetType    type(det_element.typeFlag());

                                                        std::string det_element_type = det_element.type();

                                                        if (det_element_type.find(ecal_string) != std::string::npos) {
                                                                isECal = true;
                                                        }
                                                        if (det_element_type.find(hcal_string) != std::string::npos) {
                                                                isHCal = true;
                                                        }
                                                }

                                                if (isHCal)
                                                        trace("For cellid={} , hcal is {} , ecal is {}", cell_id, isHCal, isECal);
                                        }
                                }
                        }

                        if (isECal) 
                                prelim_pid = 22; //photon
                        if (isHCal)
                                prelim_pid = 2112; // neutron

                        trace("Found particle with PID = {}", prelim_pid);
                        
                        trace("                                                                ");
                }

                trace("                                                                ");
                trace("----------------------------------------------------------------");
        };
}
