if(NOT JANA_VERSION VERSION_GREATER "2.4.2")
  # Disable omnifactory tests for versions of JANA > 2.4.2. These tests use some
  # internal JANA machinery which is going away. Tests will be re-enabled once
  # the next version has been released.

  # Automatically set plugin name the same as the directory name
  get_filename_component(TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

  # These tests can use the Catch2-provided main
  add_executable(${TEST_NAME} JOmniFactoryTests.cc)

  find_package(spdlog REQUIRED)
  find_package(fmt REQUIRED)

  # Explicit linking to podio::podio is needed due to
  # https://github.com/JeffersonLab/JANA2/issues/151
  target_include_directories(
    ${TEST_NAME}
    PRIVATE ${PROJECT_BINARY_DIR} ${EICRECON_SOURCE_DIR}/src
            ${PROJECT_SOURCE_DIR} ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS}
            ${PROJECT_BINARY_DIR}/include)

  target_link_libraries(
    ${TEST_NAME}
    PRIVATE log_library
            EDM4EIC::edm4eic
            EDM4HEP::edm4hep
            ${JANA_LIB}
            podio::podio
            podio::podioRootIO
            ${ROOT_LIBRARIES}
            Catch2::Catch2WithMain
            ${CMAKE_DL_LIBS})

  # Install executable
  install(TARGETS ${TEST_NAME} DESTINATION bin)

  add_test(NAME t_${TEST_NAME}
           COMMAND env LLVM_PROFILE_FILE=${TEST_NAME}.profraw
                   $<TARGET_FILE:${TEST_NAME}>)

endif()
