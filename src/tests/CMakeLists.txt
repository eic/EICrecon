
# Check if catch2 exists and unit tests can be built
find_package(Catch2 3)
if(Catch2_FOUND)
    add_subdirectory(algorithms_test)
else()
    message(STATUS "Catch2 is not found. Skipping algorithms_test...")
endif()

add_subdirectory(data_flow_test)
add_subdirectory(pyjano)
add_subdirectory(BEMC_test)
