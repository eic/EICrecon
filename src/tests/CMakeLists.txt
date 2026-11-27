# Check if catch2 exists and unit tests can be built
find_package(Catch2 3)
if(Catch2_FOUND)
  add_subdirectory(algorithms_test)
  add_subdirectory(omnifactory_test)
  add_subdirectory(nopayloaddb_test)
else()
  message(
    STATUS
      "Catch2 is not found. Skipping algorithms_test, omnifactory_test, and nopayloaddb_test..."
  )
endif()
