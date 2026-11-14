# Automatically set plugin name the same as the directory name
get_filename_component(TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

add_executable(${TEST_NAME} NoPayloadDbServiceTests.cc)

# This test includes EICrecon headers (src/...) and JANA headers
target_include_directories(${TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${TEST_NAME} SYSTEM PRIVATE ${JANA_INCLUDE_DIR})

target_link_libraries(${TEST_NAME} PRIVATE Catch2::Catch2WithMain ${JANA_LIB}
                                           nopayloaddb_library)

install(TARGETS ${TEST_NAME} DESTINATION bin)

# Requires a locally reachable NoPayloadDB instance (NOPAYLOADCLIENT_CONF), so
# this is excluded from the default ctest run; see
# .github/workflows/linux-eic-shell.yml's eicrecon-nopayloadclient job for where
# it's actually invoked, against real postgres + nopayloaddb services.
add_test(NAME t_${TEST_NAME} COMMAND env LLVM_PROFILE_FILE=${TEST_NAME}.profraw
                                     $<TARGET_FILE:${TEST_NAME}>)
