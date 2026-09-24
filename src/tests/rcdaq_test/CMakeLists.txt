# Unit tests for the rcdaq event source.
#
# Built conditionally when: - Catch2 v3 is available (already checked by the
# parent CMakeLists.txt) - rcdaq format headers are found (rcdaq_INCLUDE_DIR
# cache variable is set)

get_filename_component(TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# The rcdaq plugin must have been built (sets rcdaq_INCLUDE_DIR in cache)
if(NOT TARGET rcdaq_library)
  message(
    STATUS "${TEST_NAME}: rcdaq_library not available — skipping rcdaq tests")
  return()
endif()

add_executable(${TEST_NAME} rcdaq_test.cc)

target_link_libraries(${TEST_NAME} PRIVATE Catch2::Catch2WithMain rcdaq_library)

# Pass the EICrecon source root so the integration test can find the .evt file
target_compile_definitions(${TEST_NAME} PRIVATE SRCDIR="${CMAKE_SOURCE_DIR}")

# Install executable
install(TARGETS ${TEST_NAME} DESTINATION bin)

add_test(NAME t_${TEST_NAME} COMMAND env LLVM_PROFILE_FILE=${TEST_NAME}.profraw
                                     $<TARGET_FILE:${TEST_NAME}>)
