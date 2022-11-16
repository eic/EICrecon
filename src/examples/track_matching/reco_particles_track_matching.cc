// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

// Headers needed by this particular selector
#include <vector>
#include <fmt/core.h>

using std::string;

void reco_particles_track_matching() {
    auto file = new TFile("/home/romanov/eic/soft/eicrecon/main/src/examples/test_data_generator/2022-11-15_pgun_pi-_epic_arches_e0.01-30GeV_alldir_4prt_1000evt_reco.tree.edm4eic.root");
    auto tree = (TTree *) file->Get("events");
    TTreeReader tree_reader(tree);       // !the tree reader
    tree->Print();


    //TTreeReaderArray<unsigned long>
    TTreeReaderArray<float>         reco_px            = {tree_reader, "ReconstructedChargedParticles.momentum.z"};

    tree_reader.SetEntriesRange(0, 100);
    while (tree_reader.Next()) {
        // Just access the data as if myPx and myPy were iterators (note the '*'
        // in front of them):
        fmt::print("new event\n");
        for(size_t i=0; i < reco_px.GetSize(); i++) {
            fmt::print("   z={:>10.4f}\n", reco_px[i]);
        }
    }
}


int main() {
    reco_particles_track_matching();
    return 0;
}