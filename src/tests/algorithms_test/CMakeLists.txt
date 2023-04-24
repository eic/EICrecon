# Automatically set plugin name the same as the directory name
get_filename_component(TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# These tests can use the Catch2-provided main
add_executable(${TEST_NAME} algorithms_test.cc)
# Explicit linking to podio::podio is needed due to https://github.com/JeffersonLab/JANA2/issues/151
target_link_libraries(${TEST_NAME} PRIVATE Catch2::Catch2WithMain algorithms_calorimetry_library podio::podio podio::podioRootIO)

# Install executable
install(TARGETS ${TEST_NAME} DESTINATION bin)

add_test(NAME t_${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
