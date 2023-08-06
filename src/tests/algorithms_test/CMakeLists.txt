# Automatically set plugin name the same as the directory name
get_filename_component(TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Test dependencies
find_package(spdlog REQUIRED)
find_package(EDM4EIC REQUIRED)

# These tests can use the Catch2-provided main
add_executable(${TEST_NAME}
  calorimetry_CalorimeterIslandCluster.cc
  calorimetry_CalorimeterHitDigi.cc
  pid_MergeTracks.cc
  pid_MergeParticleID.cc
  )

# Explicit linking to podio::podio is needed due to https://github.com/JeffersonLab/JANA2/issues/151
target_link_libraries(${TEST_NAME} PRIVATE Catch2::Catch2WithMain algorithms_calorimetry_library algorithms_pid_library spdlog::spdlog podio::podio podio::podioRootIO EDM4EIC::edm4eic)

# Install executable
install(TARGETS ${TEST_NAME} DESTINATION bin)

add_test(NAME t_${TEST_NAME} COMMAND env LLVM_PROFILE_FILE=${TEST_NAME}.profraw $<TARGET_FILE:${TEST_NAME}>)
