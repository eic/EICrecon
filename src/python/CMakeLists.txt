# Python frontend (see docs/design/python_frontend.md).
#
# Installs the eicrecon Python package to <prefix>/python/eicrecon: graph
# definitions (eicrecon/plugins/*.py), command line and application, the _core
# extension module (JApplication handle, wiring capture, units) and _calorimetry
# (calorimetry factory types for custom graphs). The binding modules
# _wiring_<plugin> are built in the plugin directories (plugin_add_python_wiring
# in cmake/jana_plugin.cmake). The eicrecon launcher goes to <prefix>/bin.
# Python, pybind11 and Boost are found in the top-level CMakeLists.txt.

pybind11_add_module(eicrecon_python_core MODULE bindings/core.cc)
set_target_properties(eicrecon_python_core PROPERTIES OUTPUT_NAME "_core")
target_include_directories(eicrecon_python_core
                           PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(eicrecon_python_core SYSTEM
                           PRIVATE ${JANA_INCLUDE_DIR})
target_compile_definitions(
  eicrecon_python_core
  PRIVATE "EICRECON_APP_VERSION_STR=\"${EICRECON_VERSION_FULL}\""
          "JANA_VERSION_MAJOR=${JANA_VERSION_MAJOR}"
          "JANA_VERSION_MINOR=${JANA_VERSION_MINOR}"
          "JANA_VERSION_PATCH=${JANA_VERSION_PATCH}")
target_link_libraries(
  eicrecon_python_core
  PRIVATE ${JANA_LIB}
          ROOT::Core
          Boost::headers
          DD4hep::DDCore
          EDM4EIC::edm4eic
          podio::podio
          dd4hep_library
          log_library
          Microsoft.GSL::GSL)

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

# Sanitizer builds: python3 is not instrumented, so the sanitizer runtime has to
# be preloaded before the instrumented modules are imported. The launcher does
# that itself.
set(EICRECON_SANITIZER_PRELOAD "")
if(USE_ASAN
   OR USE_UBSAN
   OR USE_TSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(USE_TSAN)
      set(_sanitizer_runtime_names libclang_rt.tsan-x86_64.so)
    elseif(USE_ASAN)
      set(_sanitizer_runtime_names libclang_rt.asan-x86_64.so)
    else()
      set(_sanitizer_runtime_names libclang_rt.ubsan_standalone-x86_64.so)
    endif()
  else()
    set(_sanitizer_runtime_names)
    if(USE_ASAN)
      list(APPEND _sanitizer_runtime_names libasan.so)
    endif()
    if(USE_TSAN)
      list(APPEND _sanitizer_runtime_names libtsan.so)
    endif()
    if(USE_UBSAN)
      list(APPEND _sanitizer_runtime_names libubsan.so)
    endif()
  endif()
  set(_sanitizer_runtimes)
  foreach(_name ${_sanitizer_runtime_names})
    execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=${_name}
      OUTPUT_VARIABLE _path
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    list(APPEND _sanitizer_runtimes ${_path})
  endforeach()
  list(JOIN _sanitizer_runtimes ":" EICRECON_SANITIZER_PRELOAD)
endif()

configure_file(scripts/eicrecon.in ${CMAKE_CURRENT_BINARY_DIR}/eicrecon @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/eicrecon
        DESTINATION ${CMAKE_INSTALL_BINDIR})

# Unit test of the graph layer and the command line (no event processing),
# against a package assembled in the build tree: Python sources plus all
# extension modules
get_property(_wiring_targets GLOBAL PROPERTY EICRECON_PYTHON_WIRING_TARGETS)
set(_wiring_files)
foreach(_target ${_wiring_targets})
  list(APPEND _wiring_files $<TARGET_FILE:${_target}>)
endforeach()
add_custom_target(
  eicrecon_python_test_package ALL
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/test_package
  COMMAND ${CMAKE_COMMAND} -E make_directory
          ${CMAKE_CURRENT_BINARY_DIR}/test_package/eicrecon
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/eicrecon
    ${CMAKE_CURRENT_BINARY_DIR}/test_package/eicrecon
  COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:eicrecon_python_core>
    $<TARGET_FILE:eicrecon_python_calorimetry> ${_wiring_files}
    ${CMAKE_CURRENT_BINARY_DIR}/test_package/eicrecon
  DEPENDS eicrecon_python_core eicrecon_python_calorimetry ${_wiring_targets}
  COMMENT "Assembling the eicrecon Python package for tests")
if(NOT USE_TSAN)
  set(_py_test_env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/test_package")
  set(_py_test_env_modification)
  if(EICRECON_SANITIZER_PRELOAD)
    list(APPEND _py_test_env "LD_PRELOAD=${EICRECON_SANITIZER_PRELOAD}")
    # Extend ASAN_OPTIONS from the environment (CI suppressions and options)
    # instead of replacing it:
    #
    # * prepended, so the environment can override it: report ODR violations
    #   only for globals of different sizes (detect_odr_violation=1, as in CI).
    #   C++17 inline variables of the standard library (e.g.
    #   std::piecewise_construct) are instrumented in each extension module and
    #   would otherwise be reported.
    # * appended, so that they take effect: python3 is not instrumented (no link
    #   order check) and leaks of the interpreter are not of interest here.
    list(APPEND _py_test_env_modification
         "ASAN_OPTIONS=string_prepend:detect_odr_violation=1:"
         "ASAN_OPTIONS=string_append::detect_leaks=0:verify_asan_link_order=0")
  endif()
  add_test(
    NAME t_python_frontend
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_graph.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set_tests_properties(
    t_python_frontend
    PROPERTIES ENVIRONMENT "${_py_test_env}" ENVIRONMENT_MODIFICATION
               "${_py_test_env_modification}")
endif()
