# Python frontend (see docs/design/python_frontend.md).
#
# Installs the eicrecon Python package to <prefix>/python/eicrecon: pure-Python
# graph definitions and CLI, the _core extension module (JApplication handle,
# units) and the _calorimetry extension module (calorimetry factory types). The
# eicrecon-py launcher goes to <prefix>/bin.

find_package(Python 3.9 COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG QUIET)
find_package(Boost 1.84 QUIET) # Boost.PFR with field names (header-only)
if(NOT Python_FOUND
   OR NOT pybind11_FOUND
   OR NOT Boost_FOUND)
  message(
    WARNING
      "Python frontend (eicrecon-py) not built: needs Python development files "
      "(found: ${Python_FOUND}), pybind11 (found: ${pybind11_FOUND}) and Boost >= 1.84 "
      "(found: ${Boost_FOUND}). Set -DEICRECON_PYTHON=OFF to silence this warning."
  )
  return()
endif()

pybind11_add_module(eicrecon_python_core MODULE bindings/core.cc)
set_target_properties(eicrecon_python_core PROPERTIES OUTPUT_NAME "_core")
target_include_directories(eicrecon_python_core
                           PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(eicrecon_python_core SYSTEM
                           PRIVATE ${JANA_INCLUDE_DIR})
target_compile_definitions(
  eicrecon_python_core
  PRIVATE EICRECON_APP_VERSION=${EICRECON_VERSION_FULL}
          "JANA_VERSION_MAJOR=${JANA_VERSION_MAJOR}"
          "JANA_VERSION_MINOR=${JANA_VERSION_MINOR}"
          "JANA_VERSION_PATCH=${JANA_VERSION_PATCH}")
target_link_libraries(
  eicrecon_python_core PRIVATE ${JANA_LIB} ROOT::Core Boost::headers
                               DD4hep::DDCore EDM4EIC::edm4eic podio::podio)

pybind11_add_module(eicrecon_python_calorimetry MODULE bindings/calorimetry.cc)
set_target_properties(eicrecon_python_calorimetry PROPERTIES OUTPUT_NAME
                                                             "_calorimetry")
target_include_directories(eicrecon_python_calorimetry
                           PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(eicrecon_python_calorimetry SYSTEM
                           PRIVATE ${JANA_INCLUDE_DIR})
target_link_libraries(
  eicrecon_python_calorimetry
  PRIVATE ${JANA_LIB}
          Boost::headers
          algorithms_calorimetry_library
          log_library
          DD4hep::DDCore
          DD4hep::DDRec
          EDM4EIC::edm4eic
          EDM4HEP::edm4hep
          podio::podio
          spdlog::spdlog)
target_compile_definitions(
  eicrecon_python_calorimetry
  PRIVATE "JANA_VERSION_MAJOR=${JANA_VERSION_MAJOR}"
          "JANA_VERSION_MINOR=${JANA_VERSION_MINOR}"
          "JANA_VERSION_PATCH=${JANA_VERSION_PATCH}")

foreach(_target eicrecon_python_core eicrecon_python_calorimetry)
  # pybind11_add_module defaults to -fvisibility=hidden. That gives each
  # extension module a private copy of header-defined singletons (e.g.
  # algorithms::GeoSvc::instance(), used in algorithm default member
  # initializers), which are then never initialized by the algorithms_init
  # plugin. Use default visibility, like the JANA plugins.
  set_target_properties(
    ${_target}
    PROPERTIES CXX_VISIBILITY_PRESET default
               VISIBILITY_INLINES_HIDDEN OFF
               INSTALL_RPATH
               "${CMAKE_INSTALL_PREFIX}/${PLUGIN_LIBRARY_OUTPUT_DIRECTORY}")
  install(TARGETS ${_target}
          DESTINATION ${EICRECON_PYTHON_INSTALL_DIR}/eicrecon)
endforeach()

install(
  DIRECTORY eicrecon
  DESTINATION ${EICRECON_PYTHON_INSTALL_DIR}
  FILES_MATCHING
  PATTERN "*.py")
install(PROGRAMS scripts/eicrecon-py DESTINATION ${CMAKE_INSTALL_BINDIR})

# Unit test of the pure-Python graph layer and the command line (no event
# processing). Python itself is not instrumented, so sanitizer builds preload
# the sanitizer runtime.
set(_py_test_env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/test_package")
if(USE_ASAN OR USE_UBSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(_sanitizer_runtime_names libclang_rt.asan-x86_64.so)
  else()
    set(_sanitizer_runtime_names libasan.so libubsan.so)
  endif()
  set(_sanitizer_runtimes)
  foreach(_name ${_sanitizer_runtime_names})
    execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=${_name}
      OUTPUT_VARIABLE _path
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    list(APPEND _sanitizer_runtimes ${_path})
  endforeach()
  list(JOIN _sanitizer_runtimes ":" _sanitizer_preload)
  list(APPEND _py_test_env "LD_PRELOAD=${_sanitizer_preload}"
       "ASAN_OPTIONS=detect_leaks=0:verify_asan_link_order=0")
endif()
if(NOT USE_TSAN)
  # Assemble an importable package in the build tree: Python sources plus
  # extension modules
  add_custom_target(
    eicrecon_python_test_package ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_CURRENT_BINARY_DIR}/test_package/eicrecon
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/eicrecon
      ${CMAKE_CURRENT_BINARY_DIR}/test_package/eicrecon
    COMMAND
      ${CMAKE_COMMAND} -E copy $<TARGET_FILE:eicrecon_python_core>
      $<TARGET_FILE:eicrecon_python_calorimetry>
      ${CMAKE_CURRENT_BINARY_DIR}/test_package/eicrecon
    DEPENDS eicrecon_python_core eicrecon_python_calorimetry
    COMMENT "Assembling the eicrecon Python package for tests")
  add_test(
    NAME t_python_frontend
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_graph.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set_tests_properties(t_python_frontend PROPERTIES ENVIRONMENT
                                                    "${_py_test_env}")
endif()
