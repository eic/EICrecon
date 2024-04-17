// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei

#include <catch2/catch_test_macros.hpp>

#include <services/pid_lut/PIDLookupTable_service.h>
#include <services/pid_lut/PIDLookupTable.h>
//#include <spdlog/logger.h>

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/Services/JComponentManager.h>

#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>

#include <vector>
#include <optional>


TEST_CASE("PIDLookupTable_FindBin") {

    PIDLookupTable::Binning binning {.lower_bound=0, .upper_bound=5, .bin_count=5};

    REQUIRE(PIDLookupTable::FindBin(binning, 2.5) == 2);
    REQUIRE(PIDLookupTable::FindBin(binning, 2.0) == 2);
    REQUIRE(PIDLookupTable::FindBin(binning, 3.0) == 3);
    REQUIRE(PIDLookupTable::FindBin(binning, 2.9999) == 2);
    REQUIRE(PIDLookupTable::FindBin(binning, 5.0) == std::nullopt);


    binning = {.lower_bound=19, .upper_bound=21, .bin_count=8};

    REQUIRE(PIDLookupTable::FindBin(binning, 19.3) == 1);
    REQUIRE(PIDLookupTable::FindBin(binning, 19.0) == 0);
    REQUIRE(PIDLookupTable::FindBin(binning, 19.25) == 1);
    REQUIRE(PIDLookupTable::FindBin(binning, 19.55) == 2);
}

TEST_CASE("PIDLookupTable_Lookup") {

    PIDLookupTable lut;

    lut.GetPDGBinning() = {1};
    lut.GetChargeBinning() = {-1,1};

    lut.GetMomentumBinning().lower_bound = 20;
    lut.GetMomentumBinning().upper_bound = 40;
    lut.GetMomentumBinning().bin_count = 2;
    // Bins are 20, 30

    lut.GetEtaBinning().lower_bound = 5;
    lut.GetEtaBinning().upper_bound = 7;
    lut.GetEtaBinning().bin_count = 2;
    // Bins are 5, 6

    lut.GetPhiBinning().lower_bound = 0;
    lut.GetPhiBinning().upper_bound = 1;
    lut.GetPhiBinning().bin_count = 1;
    // Bins are 0

    // pdg, charge, momentum, eta, phi, prob_electron, prob_pion, prob_kaon, prob_proton;
    lut.AppendEntry({ 1, -1, 20, 5, 0, 1.0, 0.0, 0.0, 0.0 });
    lut.AppendEntry({ 1, -1, 20, 6, 0, 2.0, 0.0, 0.0, 0.0 });
    lut.AppendEntry({ 1, -1, 30, 5, 0, 3.0, 0.0, 0.0, 0.0 });
    lut.AppendEntry({ 1, -1, 30, 6, 0, 4.0, 0.0, 0.0, 0.0 });
    lut.AppendEntry({ 1, 1, 20, 5, 0,  5.0, 0.0, 0.0, 0.0 });
    lut.AppendEntry({ 1, 1, 20, 6, 0,  6.0, 0.0, 0.0, 0.0 });
    lut.AppendEntry({ 1, 1, 30, 5, 0,  7.0, 0.0, 0.0, 0.0 });
    lut.AppendEntry({ 1, 1, 30, 6, 0,  8.0, 0.0, 0.0, 0.0 });

    const PIDLookupTable::Entry* result;
    result = lut.Lookup(1, 1, 22, 5.5, 0.5);
    REQUIRE(result != nullptr);
    REQUIRE(result->pdg == 1);
    REQUIRE(result->charge == 1);
    REQUIRE(result->momentum == 20);
    REQUIRE(result->eta == 5);
    REQUIRE(result->phi == 0);
    REQUIRE(result->prob_electron == 5.0);

    result = lut.Lookup(1, 1, 36, 5.5, 0.5);
    REQUIRE(result != nullptr);
    REQUIRE(result->pdg == 1);
    REQUIRE(result->charge == 1);
    REQUIRE(result->momentum == 30);
    REQUIRE(result->eta == 5);
    REQUIRE(result->phi == 0);
    REQUIRE(result->prob_electron == 7.0);

    result = lut.Lookup(1, -1, 36, 6.1, 0.5);
    REQUIRE(result != nullptr);
    REQUIRE(result->pdg == 1);
    REQUIRE(result->charge == -1);
    REQUIRE(result->momentum == 30);
    REQUIRE(result->eta == 6);
    REQUIRE(result->phi == 0);
    REQUIRE(result->prob_electron == 4.0);

}

TEST_CASE("PIDLookupTable_LoadFile") {
    //PIDLookupTable lut;
    //lut.LoadFile("hpdirc_positive.lut_hidden");

    PIDLookupTable_service svc;
    const PIDLookupTable& lut = *(svc.FetchTable(
         "hpdirc_positive.lut",
         "https://raw.githubusercontent.com/nathanwbrei/fastpid/master/hpdirc_positive.lut"));

    REQUIRE(lut.GetMomentumBinning().lower_bound == 0.20);
    REQUIRE(lut.GetMomentumBinning().upper_bound == 10.2);
    REQUIRE(lut.GetMomentumBinning().bin_count == 50);

    REQUIRE(lut.GetEtaBinning().lower_bound == 25.0);
    REQUIRE(lut.GetEtaBinning().upper_bound == 161.0);
    REQUIRE(lut.GetEtaBinning().bin_count == 136);

    REQUIRE(lut.GetPhiBinning().lower_bound == 0.0);
    REQUIRE(lut.GetPhiBinning().upper_bound == 30.5);
    REQUIRE(lut.GetPhiBinning().bin_count == 61);

    const PIDLookupTable::Entry* result;
    // Retrieve line: 211 1 4.00 81.00 10.00 0.4346 0.5645 0.0009 0.0000
    // Step sizes are: 0.2, 1.0, 0.5
    result = lut.Lookup(211, 1, 4.1, 81.5, 10.25);
    REQUIRE(result != nullptr);
    REQUIRE(result->prob_electron == 0.4346);
    REQUIRE(result->prob_pion == 0.5645);
    REQUIRE(result->prob_kaon == 0.0009);
    REQUIRE(result->prob_proton == 0.0);

    // Retrieve line: 321 1 5.40 136.00 14.00 0.0308 0.0463 0.9229 0.0000
    // Step sizes are: 0.2, 1.0, 0.5
    result = lut.Lookup(321, 1, 5.5, 136.001, 14.4999);
    REQUIRE(result != nullptr);
    REQUIRE(result->prob_electron == 0.0308);
    REQUIRE(result->prob_pion == 0.0463);
    REQUIRE(result->prob_kaon == 0.9229);
    REQUIRE(result->prob_proton == 0.0);

    // Retrieve line: 11 1 0.20 25.00 15.00 0.0000 0.0000 0.0000 0.0000
    // Step sizes are: 0.2, 1.0, 0.5
    result = lut.Lookup(11, 1, 0.3, 25.5, 15.2);
    REQUIRE(result != nullptr);
    REQUIRE(result->prob_electron == 0.0);
    REQUIRE(result->prob_pion == 0.0);
    REQUIRE(result->prob_kaon == 0.0);
    REQUIRE(result->prob_proton == 0.0);

}

TEST_CASE("PIDLookupTable_EndToEnd") {
    // Set up a mini JANA instance
    JApplication app;
    app.AddPlugin("log");
    app.AddPlugin("pid_lut");
    app.AddPlugin("dd4hep");
    app.AddPlugin("richgeo");
    app.AddPlugin("DIRC");
    app.Initialize();

    // Obtain a fully initialized event
    auto jcm = app.GetService<JComponentManager>();
    auto event = std::make_shared<JEvent>();
    jcm->configure_event(*event);

    // Insert some canned input data into the event
    edm4hep::MCParticleCollection parts;
    // TODO: Add some particles here!
    event->InsertCollection<edm4hep::MCParticle>(std::move(parts), "MCParticles");

    // Run our factory
    auto results = event->GetCollection<edm4eic::ReconstructedParticle>("DIRCPID");

    // Validate results
    auto parts2 = event->GetCollection<edm4hep::MCParticle>("MCParticles");
    REQUIRE(results->size() == parts2->size());
    // TODO: More meaningful checks here!
}
