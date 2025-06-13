# Automatically set plugin name the same as the directory name
get_filename_component(TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# These tests can use the Catch2-provided main
add_executable(
  ${TEST_NAME}
  algorithmsInit.cc
  calorimetry_CalorimeterIslandCluster.cc
  calorimetry_ImagingTopoCluster.cc
  tracking_SiliconSimpleCluster.cc
  calorimetry_CalorimeterHitDigi.cc
  calorimetry_CalorimeterClusterRecoCoG.cc
  calorimetry_CalorimeterClusterShape.cc
  calorimetry_HEXPLIT.cc
  digi_SiliconPulseGeneration.cc
  digi_EICROCDigitization.cc
  pid_MergeTracks.cc
  pid_MergeParticleID.cc
  pid_lut_PIDLookup.cc)

# Explicit linking to podio::podio is needed due to
# https://github.com/JeffersonLab/JANA2/issues/151
target_link_libraries(
  ${TEST_NAME}
  PRIVATE Catch2::Catch2WithMain
          algorithms_calorimetry_library
          algorithms_fardetectors_library
          algorithms_pid_library
          algorithms_pid_lut_library
          algorithms_reco_library
          algorithms_digi_library
          evaluator_library
          pid_lut_library
          podio::podio
          podio::podioRootIO)

# Install executable
install(TARGETS ${TEST_NAME} DESTINATION bin)

add_test(NAME t_${TEST_NAME} COMMAND env LLVM_PROFILE_FILE=${TEST_NAME}.profraw
                                     $<TARGET_FILE:${TEST_NAME}>)
