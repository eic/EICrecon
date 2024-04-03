
#include <catch2/catch_test_macros.hpp>

#include <JANA/JApplication.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <services/pid_lut/PIDLookupTable.h>
#include <spdlog/logger.h>


TEST_CASE("PIDLookupTableFindBin") {

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

TEST_CASE("PIDLookupTableLookup") {

    PIDLookupTable lut;
    lut.GetMomentumBinning().lower_bound = 20;
    lut.GetMomentumBinning().upper_bound = 40;
    lut.GetMomentumBinning().bin_count = 2;
    // Bins are 20, 30

    lut.GetThetaBinning().lower_bound = 5;
    lut.GetThetaBinning().upper_bound = 7;
    lut.GetThetaBinning().bin_count = 2;
    // Bins are 5, 6

    lut.GetPhiBinning().lower_bound = 0;
    lut.GetPhiBinning().upper_bound = 1;
    lut.GetPhiBinning().bin_count = 1; 
    // Bins are 0

    // pdg, charge, momentum, theta, phi, prob_electron, prob_pion, prob_kaon, prob_proton;
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
    REQUIRE(result->theta == 5);
    REQUIRE(result->phi == 0);
    REQUIRE(result->prob_electron == 5.0);

    result = lut.Lookup(1, 1, 36, 5.5, 0.5);
    REQUIRE(result != nullptr);
    REQUIRE(result->pdg == 1);
    REQUIRE(result->charge == 1);
    REQUIRE(result->momentum == 30);
    REQUIRE(result->theta == 5);
    REQUIRE(result->phi == 0);
    REQUIRE(result->prob_electron == 7.0);

    result = lut.Lookup(1, -1, 36, 6.1, 0.5);
    REQUIRE(result != nullptr);
    REQUIRE(result->pdg == 1);
    REQUIRE(result->charge == -1);
    REQUIRE(result->momentum == 30);
    REQUIRE(result->theta == 6);
    REQUIRE(result->phi == 0);
    REQUIRE(result->prob_electron == 4.0);

}



/*
TEST_CASE("Lookup table factory test") {
    JApplication app;
    app.AddPlugin("log");
    app.AddPlugin("pid_lut");
    app.Initialize();
}
*/


