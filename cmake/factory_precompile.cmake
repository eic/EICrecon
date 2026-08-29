# SPDX-License-Identifier: LGPL-3.0-or-later Copyright (C) 2024, 2025
# Contributors to the EICrecon project
#
# CMake helper for generating precompiled JOmniFactory template instantiations
#
# This implements the extern template pattern to eliminate redundant template
# instantiation across multiple plugin files, reducing build time by ~80-120
# seconds.
#
# Usage in src/factories/*/CMakeLists.txt:
# generate_factory_precompile_sources(${PLUGIN_NAME}
# ${CMAKE_CURRENT_SOURCE_DIR})

# Parse one factory header and return class/config names plus relative include
# path.
function(_parse_factory_header HEADER OUT_FACTORY_CLASS OUT_CONFIG_TYPE
         OUT_REL_HEADER OUT_MATCHED)
  file(READ ${HEADER} HEADER_CONTENT)
  string(REGEX MATCH "namespace[ \t\n]+eicrecon" HAS_NAMESPACE
               "${HEADER_CONTENT}")

  set(MATCH_WITH_CONFIG "")
  string(
    REGEX
      MATCH
      "class[ \t\n]+([a-zA-Z0-9_]+_factory)[ \t\n]*:[ \t\n]*public[ \t\n]+JOmniFactory<[ \t\n]*[a-zA-Z0-9_]+_factory[ \t\n]*,[ \t\n]*([a-zA-Z0-9_]+)[ \t\n]*>"
      MATCH_WITH_CONFIG
      "${HEADER_CONTENT}")
  set(FACTORY_WITH_CONFIG "${CMAKE_MATCH_1}")
  set(CONFIG_WITH_CONFIG "${CMAKE_MATCH_2}")

  set(MATCH_WITHOUT_CONFIG "")
  string(
    REGEX
      MATCH
      "class[ \t\n]+([a-zA-Z0-9_]+_factory)[ \t\n]*:[ \t\n]*public[ \t\n]+JOmniFactory<[ \t\n]*[a-zA-Z0-9_]+_factory[ \t\n]*>"
      MATCH_WITHOUT_CONFIG
      "${HEADER_CONTENT}")
  set(FACTORY_NO_CONFIG "${CMAKE_MATCH_1}")

  if(FACTORY_WITH_CONFIG AND CONFIG_WITH_CONFIG)
    set(FACTORY_CLASS "${FACTORY_WITH_CONFIG}")
    set(CONFIG_TYPE "${CONFIG_WITH_CONFIG}")
  elseif(FACTORY_NO_CONFIG)
    set(FACTORY_CLASS "${FACTORY_NO_CONFIG}")
    set(CONFIG_TYPE "EmptyConfig")
  else()
    message(WARNING "Could not parse factory pattern in ${HEADER}")
    set(${OUT_MATCHED}
        FALSE
        PARENT_SCOPE)
    return()
  endif()

  if(HAS_NAMESPACE)
    set(FACTORY_CLASS "eicrecon::${FACTORY_CLASS}")
    if(CONFIG_TYPE STREQUAL "EmptyConfig")
      # EmptyConfig lives in the global namespace (from JOmniFactory.h). Leave
      # it unqualified so explicit instantiations remain valid.
    elseif(CONFIG_TYPE STREQUAL "NoConfig")
      # NoConfig is defined in eicrecon namespace (from WithPodConfig.h)
      set(CONFIG_TYPE "eicrecon::NoConfig")
    else()
      file(GLOB_RECURSE CONFIG_HEADERS
           "${PROJECT_SOURCE_DIR}/src/algorithms/*/${CONFIG_TYPE}.h")
      if(CONFIG_HEADERS)
        list(GET CONFIG_HEADERS 0 CONFIG_HEADER)
        file(READ "${CONFIG_HEADER}" CONFIG_CONTENT)
        # Track config header changes for reconfiguration
        set_property(
          DIRECTORY
          APPEND
          PROPERTY CMAKE_CONFIGURE_DEPENDS "${CONFIG_HEADER}")
        string(REGEX MATCH "namespace[ \t\n]+eicrecon" CONFIG_HAS_NAMESPACE
                     "${CONFIG_CONTENT}")
        if(CONFIG_HAS_NAMESPACE)
          set(CONFIG_TYPE "eicrecon::${CONFIG_TYPE}")
        endif()
      else()
        set(CONFIG_TYPE "eicrecon::${CONFIG_TYPE}")
      endif()
    endif()
  endif()

  file(RELATIVE_PATH REL_HEADER ${CMAKE_SOURCE_DIR}/src/factories ${HEADER})
  set(${OUT_FACTORY_CLASS}
      "${FACTORY_CLASS}"
      PARENT_SCOPE)
  set(${OUT_CONFIG_TYPE}
      "${CONFIG_TYPE}"
      PARENT_SCOPE)
  set(${OUT_REL_HEADER}
      "${REL_HEADER}"
      PARENT_SCOPE)
  set(${OUT_MATCHED}
      TRUE
      PARENT_SCOPE)
endfunction()

# Create the static library that compiles and owns explicit template
# instantiations.
function(_create_factory_precompile_library PRECOMPILE_LIB FACTORIES_CC GEN_DIR
         SOURCE_DIR FACTORY_HEADERS)
  if(TARGET ${PRECOMPILE_LIB})
    return()
  endif()

  add_library(${PRECOMPILE_LIB} STATIC ${FACTORIES_CC})
  target_include_directories(
    ${PRECOMPILE_LIB}
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
           ${GEN_DIR} ${SOURCE_DIR})
  target_include_directories(${PRECOMPILE_LIB} SYSTEM
                             PRIVATE ${JANA_INCLUDE_DIR})
  target_link_libraries(${PRECOMPILE_LIB} PUBLIC ${JANA_LIB} podio::podio
                                                 podio::podioRootIO)

  if(TARGET Acts::Core)
    get_target_property(ActsCore_LOCATION Acts::Core LOCATION)
    get_filename_component(ActsCore_PATH ${ActsCore_LOCATION} DIRECTORY)
    target_link_libraries(
      ${PRECOMPILE_LIB}
      PUBLIC
        Acts::Core
        Acts::PluginDD4hep
        Acts::PluginJson
        $<TARGET_NAME_IF_EXISTS:Acts::PluginEDM4hep>
        $<TARGET_NAME_IF_EXISTS:Acts::PluginPodio>
        ${ActsCore_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ActsExamplesFramework${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
    target_compile_definitions(
      ${PRECOMPILE_LIB} PUBLIC Acts_VERSION_MAJOR=${Acts_VERSION_MAJOR}
                                Acts_VERSION_MINOR=${Acts_VERSION_MINOR})
  endif()

  if(DD4hep_FOUND)
    target_link_libraries(${PRECOMPILE_LIB} PUBLIC DD4hep::DDCore DD4hep::DDRec)
  endif()

  if(TARGET onnxruntime::onnxruntime)
    target_link_libraries(${PRECOMPILE_LIB} PUBLIC onnxruntime::onnxruntime)
  endif()

  if(TARGET EDM4EIC::edm4eic AND TARGET EDM4HEP::edm4hep)
    target_link_libraries(${PRECOMPILE_LIB} PUBLIC EDM4EIC::edm4eic
                                                   EDM4HEP::edm4hep)
  endif()

  # Add IRT for RICH geometry (needed by PhotoMultiplierHitDigi_factory) Must be
  # PUBLIC because generated factories.h includes headers that depend on IRT
  if(NOT IRT_FOUND)
    find_package(IRT ${IRT_VERSION_MIN} QUIET)
  endif()
  if(TARGET IRT)
    # Fix IRT include directories (same as plugin_add_irt)
    get_target_property(IRT_INTERFACE_INCLUDE_DIRECTORIES IRT
                        INTERFACE_INCLUDE_DIRECTORIES)
    if(IRT_INTERFACE_INCLUDE_DIRECTORIES)
      list(TRANSFORM IRT_INTERFACE_INCLUDE_DIRECTORIES REPLACE "/IRT$" "")
      list(REMOVE_DUPLICATES IRT_INTERFACE_INCLUDE_DIRECTORIES)
      set_target_properties(
        IRT PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                       "${IRT_INTERFACE_INCLUDE_DIRECTORIES}")
    endif()
    target_link_libraries(${PRECOMPILE_LIB} PUBLIC IRT)
  endif()

  if(TARGET algorithms::algocore)
    target_link_libraries(${PRECOMPILE_LIB} PRIVATE algorithms::algocore)
  endif()

  set(POSSIBLE_DEPENDENCIES
      algorithms_onnx_library
      algorithms_tracking_library
      algorithms_interfaces_library
      algorithms_calorimetry_library
      algorithms_digi_library
      algorithms_fardetectors_library
      algorithms_meta_library
      algorithms_pid_library
      algorithms_pid_lut_library
      algorithms_reco_library
      algorithms_particle_flow_library
      log_library
      evaluator_library
      particle_service_library
      pid_lut_library
      dd4hep_library
      richgeo_library
      algorithms_init_library)

  foreach(DEP ${POSSIBLE_DEPENDENCIES})
    if(TARGET ${DEP})
      target_link_libraries(${PRECOMPILE_LIB} PRIVATE ${DEP})
    endif()
  endforeach()

  set_property(TARGET ${PRECOMPILE_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)
  set_property(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS ${FACTORY_HEADERS})

  message(STATUS "Created precompiled library: ${PRECOMPILE_LIB}")
endfunction()

# Main function to generate precompiled factory sources
function(generate_factory_precompile_sources TARGET_NAME SOURCE_DIR)
  get_filename_component(SUBSYSTEM ${SOURCE_DIR} NAME)
  set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
  file(MAKE_DIRECTORY ${GEN_DIR})

  file(GLOB FACTORY_HEADERS CONFIGURE_DEPENDS "${SOURCE_DIR}/*_factory.h")
  if(NOT FACTORY_HEADERS)
    message(
      STATUS
        "No factory headers found in ${SUBSYSTEM}, skipping precompile generation"
    )
    return()
  endif()

  set(FORWARD_DECLS "")
  set(EXTERN_TEMPLATES "")
  set(FACTORY_INCLUDES "")
  set(EXPLICIT_INSTANTIATIONS "")
  set(SEEN_CONFIGS "")

  foreach(HEADER ${FACTORY_HEADERS})
    # Always include every factory header so plugins can use
    # JOmniFactoryGeneratorT
    file(RELATIVE_PATH REL_HEADER_PATH ${CMAKE_SOURCE_DIR}/src/factories
         ${HEADER})
    string(APPEND FACTORY_INCLUDES
           "#include \"factories/${REL_HEADER_PATH}\"\n")

    _parse_factory_header("${HEADER}" FACTORY_CLASS CONFIG_TYPE REL_HEADER
                          FACTORY_MATCHED)
    if(NOT FACTORY_MATCHED)
      # Template factories or other unparsable factories: include but don't
      # extern-instantiate
      continue()
    endif()

    string(APPEND FORWARD_DECLS "class ${FACTORY_CLASS};\n")
    if(NOT CONFIG_TYPE IN_LIST SEEN_CONFIGS)
      list(APPEND SEEN_CONFIGS "${CONFIG_TYPE}")
      if(NOT CONFIG_TYPE MATCHES "NoConfig|EmptyConfig")
        string(APPEND FORWARD_DECLS "struct ${CONFIG_TYPE};\n")
      endif()
    endif()

    string(
      APPEND EXTERN_TEMPLATES
      "extern template class JOmniFactory<${FACTORY_CLASS}, ${CONFIG_TYPE}>;\n")
    string(APPEND EXPLICIT_INSTANTIATIONS
           "template class JOmniFactory<${FACTORY_CLASS}, ${CONFIG_TYPE}>;\n")
    message(
      STATUS "  Found factory: ${FACTORY_CLASS} with config ${CONFIG_TYPE}")
  endforeach()

  set(FACTORIES_H "${GEN_DIR}/factories.h")
  set(FACTORIES_CC "${GEN_DIR}/factories.cc")
  configure_file("${PROJECT_SOURCE_DIR}/cmake/factory_precompile_factories.h.in"
                 "${FACTORIES_H}" @ONLY)
  configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/factory_precompile_factories.cc.in"
    "${FACTORIES_CC}" @ONLY)

  message(STATUS "Generated precompiled factory sources for ${SUBSYSTEM}:")
  message(STATUS "  ${FACTORIES_H}")
  message(STATUS "  ${FACTORIES_CC}")

  set(PRECOMPILE_LIB "${TARGET_NAME}_precompiled")
  _create_factory_precompile_library(
    "${PRECOMPILE_LIB}" "${FACTORIES_CC}" "${GEN_DIR}" "${SOURCE_DIR}"
    "${FACTORY_HEADERS}")
endfunction()
