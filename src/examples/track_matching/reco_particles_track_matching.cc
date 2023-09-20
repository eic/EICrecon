// Very minimal and verbose example of how to use association IDs for MCParticles and ReconstructedParticles
// In general there are two ways associations may work: through PODIO references and IDs
// This example illustrates how to use IDs
//
// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <string>

#include <TFile.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>

#include <fmt/core.h>

void reco_particles_track_matching(const std::string &file_name) {
    auto *file = new TFile(file_name.c_str());
    auto *tree = (TTree *) file->Get("events");
    TTreeReader tree_reader(tree);       // !the tree reader
    tree->Print();

    // Reconstructed particles pz array for each reconstructed particle
    TTreeReaderArray<float> reco_pz_array = {tree_reader, "ReconstructedChargedParticles.momentum.z"};

    // MC particle pz array for each MC particle
    TTreeReaderArray<float> mc_pz_array = {tree_reader, "MCParticles.momentum.z"};

    // Next arrays correspond to particle associations
    // Each association has 2 ids - indexes in corresponding reco and MC arrays
    TTreeReaderArray<unsigned int> rec_id = {tree_reader, "ReconstructedChargedParticleAssociations.recID"};
    TTreeReaderArray<unsigned int> sim_id = {tree_reader, "ReconstructedChargedParticleAssociations.simID"};


    // Read 100 events
    tree_reader.SetEntriesRange(0, 100);
    while (tree_reader.Next()) {

        // Number of mc particles, reco particles and associations may differ
        fmt::print("New event. N reco particles: {}, N mc particles: {}, N assoc: {}\n",
                   reco_pz_array.GetSize(), mc_pz_array.GetSize(), rec_id.GetSize());

        // Iterate over associations
        for(unsigned int i=0; i < rec_id.GetSize(); i++) {

            // For each association pull index of reco and MC array
            auto reco_array_index = rec_id[i];
            auto mc_array_index = sim_id[i];

            float reco_pz = reco_pz_array[reco_array_index];
            float mc_pz = mc_pz_array[mc_array_index];
            fmt::print("   reco={:>10.4f} mc={:>10.4f}\n", reco_pz, mc_pz);
        }
    }
}


int main() {
    reco_particles_track_matching("input.edm4eic.root");
    return 0;
}
