# Automatically set plugin name the same as the directory name
get_filename_component(TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

add_executable(${TEST_NAME} extensions_JOmniFactoryParameter.cc)

# This test includes EICrecon headers (src/...) and JANA headers
target_include_directories(${TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${TEST_NAME} SYSTEM PRIVATE ${JANA_INCLUDE_DIR})

target_link_libraries(
  ${TEST_NAME}
  PRIVATE Catch2::Catch2WithMain
          ${JANA_LIB}
          podio::podio
          podio::podioIO
          spdlog::spdlog
          fmt::fmt
          EDM4HEP::edm4hep
          EDM4EIC::edm4eic)

install(TARGETS ${TEST_NAME} DESTINATION bin)

add_test(NAME t_${TEST_NAME} COMMAND env LLVM_PROFILE_FILE=${TEST_NAME}.profraw
                                     $<TARGET_FILE:${TEST_NAME}>)
