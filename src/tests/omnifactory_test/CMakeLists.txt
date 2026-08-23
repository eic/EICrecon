# Automatically set plugin name the same as the directory name
get_filename_component(TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

add_executable(${TEST_NAME} extensions_JOmniFactoryParameter.cc)

target_link_libraries(${TEST_NAME} PRIVATE Catch2::Catch2WithMain podio::podio podio::podioIO)

install(TARGETS ${TEST_NAME} DESTINATION bin)

add_test(NAME t_${TEST_NAME} COMMAND env LLVM_PROFILE_FILE=${TEST_NAME}.profraw
                                     $<TARGET_FILE:${TEST_NAME}>)
