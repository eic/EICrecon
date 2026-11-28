# Python frontend prototype (see docs/design/python_frontend.md).
#
# Installs the eicrecon Python package to <prefix>/python/eicrecon: pure-Python
# graph definitions and CLI, the _core extension module (JApplication handle,
# units) and the _calorimetry extension module (calorimetry factory types). The
# eicrecon-py launcher goes to <prefix>/bin.

find_package(
  Python 3.9
  COMPONENTS Interpreter Development.Module
  REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(Boost 1.84 REQUIRED) # Boost.PFR with field names (header-only)

pybind11_add_module(eicrecon_python_core MODULE bindings/core.cc)
set_target_properties(eicrecon_python_core PROPERTIES OUTPUT_NAME "_core")
target_include_directories(eicrecon_python_core
                           PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(eicrecon_python_core SYSTEM
                           PRIVATE ${JANA_INCLUDE_DIR})
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
