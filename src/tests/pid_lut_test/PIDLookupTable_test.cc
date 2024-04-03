
#include <catch2/catch_test_macros.hpp>

#include <JANA/JApplication.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <services/pid_lut/PIDLookupTable.h>
#include <spdlog/logger.h>


TEST_CASE("Lookup table test creation") {
    PIDLookupTable sut;
}

/*
TEST_CASE("Lookup table factory test") {
    JApplication app;
    app.AddPlugin("log");
    app.AddPlugin("pid_lut");
    app.Initialize();
}
*/


