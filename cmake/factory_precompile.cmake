# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2024, 2025 Contributors to the EICrecon project
#
# CMake helper for generating precompiled JOmniFactory template instantiations
#
# This implements the extern template pattern to eliminate redundant template
# instantiation across multiple plugin files, reducing build time by ~80-120 seconds.
#
# Usage in src/factories/*/CMakeLists.txt:
#   generate_factory_precompile_sources(${PLUGIN_NAME} ${CMAKE_CURRENT_SOURCE_DIR})

# Main function to generate precompiled factory sources
function(generate_factory_precompile_sources TARGET_NAME SOURCE_DIR)
  # Get subsystem name for generated file comments
  get_filename_component(SUBSYSTEM ${SOURCE_DIR} NAME)

  # Create output directory for generated files
  set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
  file(MAKE_DIRECTORY ${GEN_DIR})

  # Find all factory headers in the source directory
  file(GLOB FACTORY_HEADERS "${SOURCE_DIR}/*_factory.h")

  if(NOT FACTORY_HEADERS)
    message(STATUS "No factory headers found in ${SUBSYSTEM}, skipping precompile generation")
    return()
  endif()

  # Initialize lists for generated content
  set(FORWARD_DECLS "")
  set(EXTERN_TEMPLATES "")
  set(FACTORY_INCLUDES "")
  set(EXPLICIT_INSTANTIATIONS "")
  set(CONFIG_INCLUDES "")

  # Track unique config types to avoid duplicate forward declarations
  set(SEEN_CONFIGS "")

  # Parse each factory header
  foreach(HEADER ${FACTORY_HEADERS})
    # Read the header file content
    file(READ ${HEADER} HEADER_CONTENT)

    # Extract factory class name and config type from JOmniFactory inheritance
    # Pattern: namespace eicrecon { class FactoryName_factory : public JOmniFactory<FactoryName_factory, ConfigType> }
    # First check if we're in eicrecon namespace
    string(REGEX MATCH "namespace[ \t\n]+eicrecon" HAS_NAMESPACE "${HEADER_CONTENT}")

    # Try pattern WITH config type: JOmniFactory<Factory, Config>
    string(REGEX MATCH "class[ \t\n]+([a-zA-Z0-9_]+_factory)[ \t\n]*:[ \t\n]*public[ \t\n]+JOmniFactory<[ \t\n]*[a-zA-Z0-9_]+_factory[ \t\n]*,[ \t\n]*([a-zA-Z0-9_]+)[ \t\n]*>"
           MATCH_WITH_CONFIG "${HEADER_CONTENT}")

    # Try pattern WITHOUT config type: JOmniFactory<Factory>
    string(REGEX MATCH "class[ \t\n]+([a-zA-Z0-9_]+_factory)[ \t\n]*:[ \t\n]*public[ \t\n]+JOmniFactory<[ \t\n]*[a-zA-Z0-9_]+_factory[ \t\n]*>"
           MATCH_WITHOUT_CONFIG "${HEADER_CONTENT}")

    if(CMAKE_MATCH_1 AND CMAKE_MATCH_2)
      # Pattern with config matched
      set(FACTORY_CLASS ${CMAKE_MATCH_1})
      set(CONFIG_TYPE ${CMAKE_MATCH_2})

      # If in eicrecon namespace, use fully qualified names
      if(HAS_NAMESPACE)
        set(FACTORY_CLASS "eicrecon::${FACTORY_CLASS}")
        # Config is also in eicrecon namespace ONLY if it's defined in the factory header
        # or algorithm header within the eicrecon namespace block
        # For now, check if the config type exists in the algorithms directory
        file(GLOB_RECURSE CONFIG_HEADERS "${PROJECT_SOURCE_DIR}/src/algorithms/*/${CONFIG_TYPE}.h")
        if(CONFIG_HEADERS)
          # Found config header, check if it's in eicrecon namespace
          file(READ "${CONFIG_HEADERS}" CONFIG_CONTENT)
          string(REGEX MATCH "namespace[ \t\n]+eicrecon" CONFIG_HAS_NAMESPACE "${CONFIG_CONTENT}")
          if(CONFIG_HAS_NAMESPACE)
            set(CONFIG_TYPE "eicrecon::${CONFIG_TYPE}")
          endif()
        else()
          # No config header found, assume it's in eicrecon namespace (like NoConfig)
          set(CONFIG_TYPE "eicrecon::${CONFIG_TYPE}")
        endif()
      endif()
    elseif(CMAKE_MATCH_1)
      # Pattern without config matched - use EmptyConfig
      set(FACTORY_CLASS ${CMAKE_MATCH_1})
      set(CONFIG_TYPE "EmptyConfig")

      if(HAS_NAMESPACE)
        set(FACTORY_CLASS "eicrecon::${FACTORY_CLASS}")
        set(CONFIG_TYPE "eicrecon::${CONFIG_TYPE}")
      endif()
    else()
      # No match, skip this header
      message(WARNING "Could not parse factory pattern in ${HEADER}")
      continue()
    endif()

      get_filename_component(HEADER_NAME ${HEADER} NAME)

      # Get relative path from src/factories/ for the include
      file(RELATIVE_PATH REL_HEADER ${CMAKE_SOURCE_DIR}/src/factories ${HEADER})

      # Add factory forward declaration
      string(APPEND FORWARD_DECLS "class ${FACTORY_CLASS};\n")

      # Add config forward declaration or include (only once per unique config)
      if(NOT CONFIG_TYPE IN_LIST SEEN_CONFIGS)
        list(APPEND SEEN_CONFIGS ${CONFIG_TYPE})
        if(CONFIG_TYPE MATCHES "NoConfig")
          # NoConfig is a special marker, not a real type - don't forward declare
          # It will be handled by the JOmniFactory include
        elseif(CONFIG_TYPE MATCHES "EmptyConfig")
          # EmptyConfig is defined in JOmniFactory.h, don't forward declare
        else()
          # Forward declare custom config struct
          string(APPEND FORWARD_DECLS "struct ${CONFIG_TYPE};\n")
        endif()
      endif()

      # Add extern template declaration
      string(APPEND EXTERN_TEMPLATES "extern template class JOmniFactory<${FACTORY_CLASS}, ${CONFIG_TYPE}>;\n")

      # Add factory header include for .cc file using path relative to src/factories/
      string(APPEND FACTORY_INCLUDES "#include \"factories/${REL_HEADER}\"\n")

      # Add explicit instantiation for .cc file
      string(APPEND EXPLICIT_INSTANTIATIONS "template class JOmniFactory<${FACTORY_CLASS}, ${CONFIG_TYPE}>;\n")

      message(STATUS "  Found factory: ${FACTORY_CLASS} with config ${CONFIG_TYPE}")
  endforeach()

  # Generate factories.h header file
  set(FACTORIES_H_CONTENT
"// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, 2025 Contributors to the EICrecon project
//
// AUTO-GENERATED by CMake - DO NOT EDIT
// Precompiled factory instantiations for ${SUBSYSTEM}
// Source: cmake/factory_precompile.cmake
//
// This header provides extern template declarations to avoid redundant
// template instantiation across multiple plugin files, reducing build time.

#pragma once

// Common service headers that factories may need
#include \"services/algorithms_init/AlgorithmsInit_service.h\"
#include \"extensions/jana/JOmniFactory.h\"

// Include factory headers - needed for complete types
${FACTORY_INCLUDES}
// Forward declaration of JOmniFactory template
template <typename AlgoT, typename ConfigT> class JOmniFactory;

// Extern template declarations - tells compiler these are instantiated in factories.cc
${EXTERN_TEMPLATES}")

  # Generate factories.cc source file
  set(FACTORIES_CC_CONTENT
"// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, 2025 Contributors to the EICrecon project
//
// AUTO-GENERATED by CMake - DO NOT EDIT
// Explicit instantiations for ${SUBSYSTEM} factories
// Source: cmake/factory_precompile.cmake

#include \"factories.h\"

// Include factory headers
${FACTORY_INCLUDES}
// Include JOmniFactory implementation
#include <extensions/jana/JOmniFactory.h>

// Explicit template instantiations - compile each template exactly once
${EXPLICIT_INSTANTIATIONS}")

  # Write generated files
  set(FACTORIES_H "${GEN_DIR}/factories.h")
  set(FACTORIES_CC "${GEN_DIR}/factories.cc")

  file(WRITE ${FACTORIES_H} "${FACTORIES_H_CONTENT}")
  file(WRITE ${FACTORIES_CC} "${FACTORIES_CC_CONTENT}")

  message(STATUS "Generated precompiled factory sources for ${SUBSYSTEM}:")
  message(STATUS "  ${FACTORIES_H}")
  message(STATUS "  ${FACTORIES_CC}")

  # For header-only factory plugins, create a small library to compile the precompiled instantiations
  # The library will be linked by any plugin that uses these factories
  set(PRECOMPILE_LIB "${TARGET_NAME}_precompiled")

  if(NOT TARGET ${PRECOMPILE_LIB})
    add_library(${PRECOMPILE_LIB} STATIC ${FACTORIES_CC})

    # Use the same include setup as regular plugins
    target_include_directories(${PRECOMPILE_LIB}
      PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
        ${GEN_DIR}
        ${SOURCE_DIR}  # Add source directory so factory headers are found
    )
    target_include_directories(${PRECOMPILE_LIB} SYSTEM PUBLIC ${JANA_INCLUDE_DIR})
    target_link_libraries(${PRECOMPILE_LIB} PUBLIC ${JANA_LIB} podio::podio podio::podioRootIO)

    # Use the standard plugin macros to add external dependencies
    # These will no-op if already found, or find and link the packages
    plugin_add_acts(${PRECOMPILE_LIB})
    plugin_add_dd4hep(${PRECOMPILE_LIB})
    plugin_add_onnxruntime(${PRECOMPILE_LIB})
    plugin_add_event_model(${PRECOMPILE_LIB})
    plugin_add_algorithms(${PRECOMPILE_LIB})

    # Ensure Acts version macros are defined (needed for conditional compilation)
    if(Acts_FOUND)
      target_compile_definitions(${PRECOMPILE_LIB} PRIVATE
        Acts_VERSION_MAJOR=${Acts_VERSION_MAJOR}
        Acts_VERSION_MINOR=${Acts_VERSION_MINOR}
      )
    endif()

    # Link against ALL algorithm and service libraries to ensure all includes and dependencies are available
    # This is necessary because factory headers can transitively include any algorithm or service
    # Use a list to make it easier to maintain
    set(POSSIBLE_DEPENDENCIES
      # Algorithm libraries
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
      # Service libraries
      log_library
      evaluator_library
      particle_service_library
      pid_lut_library
      dd4hep_library
      richgeo_library
      algorithms_init_library
    )

    foreach(DEP ${POSSIBLE_DEPENDENCIES})
      if(TARGET ${DEP})
        target_link_libraries(${PRECOMPILE_LIB} PRIVATE ${DEP})
      endif()
    endforeach()

    # Enable PIC for static library
    set_property(TARGET ${PRECOMPILE_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)

    # Add factory headers as dependencies so CMake reconfigures if they change
    set_property(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      APPEND
      PROPERTY CMAKE_CONFIGURE_DEPENDS ${FACTORY_HEADERS}
    )

    message(STATUS "Created precompiled library: ${PRECOMPILE_LIB}")
  endif()

endfunction()
