option(USE_SYSTEM_CATCH2 "Use system Catch2 v3 otherwise build it locally" ON)

if(USE_SYSTEM_CATCH2)
  find_package(Catch2 3 REQUIRED)
else()
  include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.3.2
  )
  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
endif()

if(Catch2_FOUND)
  include(CTest)
  include(Catch)
  add_subdirectory(algorithms_test)
else()
  message(STATUS "Catch2 is not found. Skipping algorithms_test...")
endif()

add_subdirectory(pyjano)
add_subdirectory(BEMC_test)
add_subdirectory(reco_test)
add_subdirectory(tracking_test)
add_subdirectory(track_propagation_test)
add_subdirectory(track_seeding_test)
add_subdirectory(geometry_navigation_test)
add_subdirectory(podio_test)
