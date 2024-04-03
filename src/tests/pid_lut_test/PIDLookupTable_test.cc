
#include <catch2/catch_test_macros.hpp>

#include <JANA/JApplication.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <services/pid_lut/PIDLookupTable.h>
#include <spdlog/logger.h>


TEST_CASE("PIDLookupTableFindBin") {

    PIDLookupTable::Binning binning {.lower_bound=0, .upper_bound=5, .step=1};

    REQUIRE(PIDLookupTable::FindBin(binning, 2.5) == 2);
    REQUIRE(PIDLookupTable::FindBin(binning, 2.0) == 2);
    REQUIRE(PIDLookupTable::FindBin(binning, 3.0) == 3);
    REQUIRE(PIDLookupTable::FindBin(binning, 2.9999) == 2);
    REQUIRE(PIDLookupTable::FindBin(binning, 5.0) == std::nullopt);


    binning = {.lower_bound=19, .upper_bound=21, .step=0.25};

    REQUIRE(PIDLookupTable::FindBin(binning, 19.3) == 1);
    REQUIRE(PIDLookupTable::FindBin(binning, 19.0) == 0);
    REQUIRE(PIDLookupTable::FindBin(binning, 19.25) == 1);
    REQUIRE(PIDLookupTable::FindBin(binning, 19.55) == 2);
}


/*
TEST_CASE("Lookup table factory test") {
    JApplication app;
    app.AddPlugin("log");
    app.AddPlugin("pid_lut");
    app.Initialize();
}
*/


