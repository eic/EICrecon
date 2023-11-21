# Automatically set plugin name the same as the directory name
get_filename_component(TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# These tests can use the Catch2-provided main
add_executable(${TEST_NAME}
  calorimetry_CalorimeterIslandCluster.cc
  calorimetry_CalorimeterHitDigi.cc
  pid_MergeTracks.cc
  pid_MergeParticleID.cc
  )

# Explicit linking to podio::podio is needed due to https://github.com/JeffersonLab/JANA2/issues/151
target_link_libraries(${TEST_NAME} PRIVATE Catch2::Catch2WithMain algorithms_calorimetry_library algorithms_pid_library podio::podio podio::podioRootIO)
# As-needed fails due to absent podio symbols in libJANA.so (https://github.com/JeffersonLab/JANA2/pull/255)
if (CMAKE_CXX_COMPILER_LINKER_ID MATCHES "GNU(gold)?")
  target_link_options(${TEST_NAME} PRIVATE "-Wl,--no-as-needed")
endif()

# Install executable
install(TARGETS ${TEST_NAME} DESTINATION bin)

add_test(NAME t_${TEST_NAME} COMMAND env LLVM_PROFILE_FILE=${TEST_NAME}.profraw $<TARGET_FILE:${TEST_NAME}>)
