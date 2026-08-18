# Check if catch2 exists and unit tests can be built
find_package(Catch2 3)
if(Catch2_FOUND)
  add_subdirectory(algorithms_test)
  add_subdirectory(omnifactory_test)
else()
  message(
    STATUS
      "Catch2 is not found. Skipping algorithms_test and omnifactory_test...")
endif()

if(EICRECON_HAS_ARROW)
  add_subdirectory(arrow_stream_test)
else()
  message(STATUS "Arrow not available. Skipping arrow_stream_test...")
endif()
add_subdirectory(tracking_test)
add_subdirectory(track_propagation_test)
add_subdirectory(geometry_navigation_test)
