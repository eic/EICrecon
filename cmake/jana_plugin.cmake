# Ensure GNU filesystem layout
include(GNUInstallDirs)

# Common macro to add plugins
macro(plugin_add _name)

    # Default to plugin without library
    set(${_name}_WITH_LIBRARY OFF)
    set(${_name}_WITH_PLUGIN ON)

    # Check if build with library
    foreach(arg IN ITEMS ${ARGN})
        if(${arg} STREQUAL "WITH_STATIC_LIBRARY")
            set(${_name}_WITH_LIBRARY ON)
            set(${_name}_LIBRARY_TYPE STATIC)
        endif()
        if(${arg} STREQUAL "WITH_SHARED_LIBRARY")
            set(${_name}_WITH_LIBRARY ON)
            set(${_name}_LIBRARY_TYPE SHARED)
        endif()
        if(${arg} STREQUAL "WITHOUT_PLUGIN")
            set(${_name}_WITH_PLUGIN OFF)
        endif()
    endforeach()

    # Include JANA by default
    find_package(JANA REQUIRED)

    # TODO: NWB: This really needs to be a dependency of JANA itself.
    # If we don't do this here, CMake will later refuse to accept that podio is
    # indeed a dependency of JANA and aggressively reorders my target_link_list
    # to reflect this misapprehension.
    # https://gitlab.kitware.com/cmake/cmake/blob/v3.13.2/Source/cmComputeLinkDepends.cxx
    find_package(podio REQUIRED)

    # include logging by default
    find_package(spdlog REQUIRED)

    # include fmt by default
    find_package(fmt 9.0.0 REQUIRED)

    # include gsl by default
    find_package(Microsoft.GSL CONFIG)

    # Define plugin
    if(${_name}_WITH_PLUGIN)
        add_library(${_name}_plugin SHARED ${PLUGIN_SOURCES})

        target_include_directories(${_name}_plugin
          PUBLIC
            $<BUILD_INTERFACE:${EICRECON_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
        )
        target_include_directories(${_name}_plugin SYSTEM PUBLIC ${JANA_INCLUDE_DIR} )
        target_include_directories(${_name}_plugin SYSTEM PUBLIC ${ROOT_INCLUDE_DIRS} )
        set_target_properties(${_name}_plugin PROPERTIES PREFIX "" OUTPUT_NAME "${_name}" SUFFIX ".so")
        target_link_libraries(${_name}_plugin ${JANA_LIB} podio::podio podio::podioRootIO spdlog::spdlog fmt::fmt)
        target_link_libraries(${_name}_plugin Microsoft.GSL::GSL)

        # Install plugin
        install(TARGETS ${_name}_plugin
            EXPORT EICreconTargets
            DESTINATION ${PLUGIN_OUTPUT_DIRECTORY}
        )
    endif(${_name}_WITH_PLUGIN)

    # Define library
    if(${_name}_WITH_LIBRARY)
        add_library(${_name}_library ${${_name}_LIBRARY_TYPE} "")
        if(${_name}_LIBRARY_TYPE STREQUAL "STATIC")
            set(suffix ".a")
        endif()
        if(${_name}_LIBRARY_TYPE STREQUAL "SHARED")
            set(suffix ".so")
        endif()
        set_target_properties(${_name}_library PROPERTIES PREFIX "lib" OUTPUT_NAME "${_name}" SUFFIX ${suffix})

        target_include_directories(${_name}_library
          PUBLIC
            $<BUILD_INTERFACE:${EICRECON_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
        )
        target_include_directories(${_name}_library SYSTEM PUBLIC ${JANA_INCLUDE_DIR} )
        target_link_libraries(${_name}_library ${JANA_LIB} podio::podio podio::podioRootIO spdlog::spdlog)
        target_link_libraries(${_name}_library ${JANA_LIB} podio::podio podio::podioRootIO fmt::fmt)
        target_link_libraries(${_name}_library Microsoft.GSL::GSL)

        # Install library
        install(TARGETS ${_name}_library
            EXPORT EICreconTargets
            DESTINATION ${PLUGIN_LIBRARY_OUTPUT_DIRECTORY}
        )
    endif(${_name}_WITH_LIBRARY)

    if(${_name}_WITH_LIBRARY AND ${_name}_WITH_PLUGIN)
        target_link_libraries(${_name}_plugin ${_name}_library)
    endif()
endmacro()


# target_link_libraries for both a plugin and a library
macro(plugin_link_libraries _name)
    if(${_name}_WITH_PLUGIN)
        target_link_libraries(${_name}_plugin ${ARGN})
    endif(${_name}_WITH_PLUGIN)

    if(${_name}_WITH_LIBRARY)
        target_link_libraries(${_name}_library ${ARGN})
    endif(${_name}_WITH_LIBRARY)
endmacro()


# target_include_directories for both a plugin and a library
macro(plugin_include_directories _name)
    if(${_name}_WITH_PLUGIN)
        target_include_directories(${_name}_plugin  ${ARGN})
    endif(${_name}_WITH_PLUGIN)

    if(${_name}_WITH_LIBRARY)
        target_include_directories(${_name}_library ${ARGN})
    endif(${_name}_WITH_LIBRARY)
endmacro()


# runs target_sources both for library and a plugin
macro(plugin_sources _name)
    # This is needed as this is a macro (see cmake macro documentation)
    set(SOURCES ${ARGN})

    # Add sources to plugin
    target_sources(${_name}_plugin PRIVATE ${SOURCES})

    if(${_name}_WITH_LIBRARY)
        # Library don't need <plugin_name>.cc in library
        set(PLUGIN_CC_FILE ${_name}.cc)
        get_filename_component(PLUGIN_CC_FILE "${CMAKE_CURRENT_LIST_DIR}/${PLUGIN_CC_FILE}" ABSOLUTE)
        list(REMOVE_ITEM SOURCES ${PLUGIN_CC_FILE})

        # Add sources to library
        target_sources(${_name}_library PRIVATE ${SOURCES})
    endif(${_name}_WITH_LIBRARY)
endmacro()


# installs headers in current directory
macro(plugin_headers_only _name)
    # get all headers
    file(GLOB HEADER_FILES CONFIGURE_DEPENDS *.h *.hh *.hpp)

    # We need plugin relative path for correct headers installation (FIXME cmake 3.20: cmake_path)
    file(RELATIVE_PATH PLUGIN_RELATIVE_PATH ${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})

    # FIXME cmake 3.23: define FILE_SET on target_sources
    install(FILES ${HEADER_FILES}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${PLUGIN_RELATIVE_PATH}
    )
endmacro()


# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
macro(plugin_glob_all _name)

    # But... GLOB here makes this file just hot pluggable
    file(GLOB LIB_SRC_FILES CONFIGURE_DEPENDS *.cc *.cpp *.c)
    if(${_name}_WITH_LIBRARY)
        file(GLOB PLUGIN_SRC_FILES CONFIGURE_DEPENDS ${_name}.cc)
    else()
        file(GLOB PLUGIN_SRC_FILES CONFIGURE_DEPENDS *.cc *.cpp *.c)
    endif()
    file(GLOB HEADER_FILES CONFIGURE_DEPENDS *.h *.hh *.hpp)

    # We need plugin relative path for correct headers installation (FIXME cmake 3.20: cmake_path)
    file(RELATIVE_PATH PLUGIN_RELATIVE_PATH ${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})

    # Add sources to plugin
    if(TARGET ${_name}_plugin)
        target_sources(${_name}_plugin PRIVATE ${PLUGIN_SRC_FILES})
    endif()

    # FIXME cmake 3.23: define FILE_SET on target_sources
    install(FILES ${HEADER_FILES}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${PLUGIN_RELATIVE_PATH}
    )

    if(${_name}_WITH_LIBRARY)
        # Library don't need <plugin_name>.cc but Plugin does
        set(PLUGIN_CC_FILE ${_name}.cc)

        # Make the path absolute as GLOB files will be absolute paths
        get_filename_component(PLUGIN_CC_FILE_ABS "${CMAKE_CURRENT_LIST_DIR}/${PLUGIN_CC_FILE}" ABSOLUTE)

        # Remove plugin.cc file from libraries
        list(REMOVE_ITEM LIB_SRC_FILES ${PLUGIN_CC_FILE_ABS})

        # Finally add sources to library
        target_sources(${_name}_library PRIVATE ${LIB_SRC_FILES})
    endif(${_name}_WITH_LIBRARY)

    # Debug output if needed
    message(VERBOSE "plugin_glob_all:${_name}: PLUGIN_CC_FILE   ${PLUGIN_CC_FILE}")
    message(VERBOSE "plugin_glob_all:${_name}: LIB_SRC_FILES    ${LIB_SRC_FILES}")
    message(VERBOSE "plugin_glob_all:${_name}: PLUGIN_SRC_FILES ${PLUGIN_SRC_FILES}")
    message(VERBOSE "plugin_glob_all:${_name}: HEADER_FILES     ${HEADER_FILES}")
    message(VERBOSE "plugin_glob_all:${_name}: PLUGIN_RLTV_PATH ${PLUGIN_RELATIVE_PATH}")

endmacro()


# Adds algorithms for a plugin
macro(plugin_add_algorithms _name)

    if(NOT algorithms_FOUND)
        find_package(algorithms REQUIRED)
    endif()

    plugin_link_libraries(${_name}
        algocore
    )

endmacro()


# Adds dd4hep for a plugin
macro(plugin_add_dd4hep _name)

    if(NOT DD4hep_FOUND)
        find_package(DD4hep REQUIRED)
    endif()

    plugin_link_libraries(${_name}
        DD4hep::DDCore
        DD4hep::DDRec
    )

endmacro()


# Adds Eigen3 for a plugin
macro(plugin_add_eigen3 _name)

    if(NOT Eigen3_FOUND)
        find_package(Eigen3 REQUIRED)
    endif()

    plugin_link_libraries(${_name}
        Eigen3::Eigen
    )

endmacro()


# Adds ACTS tracking package for a plugin
macro(plugin_add_acts _name)

    if(NOT Acts_FOUND)
        find_package(Acts REQUIRED COMPONENTS Core PluginIdentification PluginTGeo PluginJson PluginDD4hep)
        set(Acts_VERSION_MIN "20.2.0")
        set(Acts_VERSION "${Acts_VERSION_MAJOR}.${Acts_VERSION_MINOR}.${Acts_VERSION_PATCH}")
        if(${Acts_VERSION} VERSION_LESS ${Acts_VERSION_MIN}
                AND NOT "${Acts_VERSION}" STREQUAL "9.9.9")
            message(FATAL_ERROR "Acts version ${Acts_VERSION_MIN} or higher required, but ${Acts_VERSION} found")
        endif()
    endif()

    # Get ActsExamples base
    get_target_property(ActsCore_LOCATION ActsCore LOCATION)
    get_filename_component(ActsCore_PATH ${ActsCore_LOCATION} DIRECTORY)

    # Add libraries (works same as target_include_directories)
    plugin_link_libraries(${PLUGIN_NAME}
        ActsCore
        ActsPluginIdentification
        ActsPluginTGeo
        ActsPluginJson
        ActsPluginDD4hep
        ${ActsCore_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ActsExamplesFramework${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
    if(${_name}_WITH_LIBRARY)
        target_compile_definitions(${PLUGIN_NAME}_library PRIVATE "Acts_VERSION_MAJOR=${Acts_VERSION_MAJOR}")
    endif()
    if(${_name}_WITH_PLUGIN)
        target_compile_definitions(${PLUGIN_NAME}_plugin PRIVATE "Acts_VERSION_MAJOR=${Acts_VERSION_MAJOR}")
    endif()

endmacro()


# Adds IRT PID reconstruction package for a plugin
macro(plugin_add_irt _name)

    if(NOT IRT_FOUND)
        find_package(IRT REQUIRED)
    endif()

    # FIXME: IRTConfig.cmake sets INTERFACE_INCLUDE_DIRECTORIES to <prefix>/include/IRT
    # instead of <prefix>/include, allowing for short-form #include <CherenkovDetector.h>
    get_target_property(IRT_INTERFACE_INCLUDE_DIRECTORIES IRT INTERFACE_INCLUDE_DIRECTORIES)
    list(TRANSFORM IRT_INTERFACE_INCLUDE_DIRECTORIES REPLACE "/IRT$" "")
    list(REMOVE_DUPLICATES IRT_INTERFACE_INCLUDE_DIRECTORIES)
    set_target_properties(IRT PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${IRT_INTERFACE_INCLUDE_DIRECTORIES}")

    plugin_link_libraries(${PLUGIN_NAME} IRT)

endmacro()

# Adds podio, edm4hep, edm4eic for a plugin
macro(plugin_add_event_model _name)

    if(NOT podio_FOUND)
        find_package(podio REQUIRED)
    endif()

    if(NOT EDM4HEP_FOUND)
        find_package(EDM4HEP REQUIRED)
    endif()

    if(NOT EDM4EIC_FOUND)
        find_package(EDM4EIC REQUIRED)
    endif()

    # Add include directories
    plugin_include_directories(${PLUGIN_NAME}
      PUBLIC
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
    )

    # Add libraries
    # (same as target_include_directories but for both plugin and library)
    plugin_link_libraries(${PLUGIN_NAME}
        podio::podio
        EDM4EIC::edm4eic
        EDM4HEP::edm4hep
    )

endmacro()


# Adds cern ROOT for a plugin
macro(plugin_add_cern_root _name)

    if(NOT ROOT_FOUND)
        find_package(ROOT REQUIRED)
    endif()

    # Add libraries
    plugin_link_libraries(${PLUGIN_NAME}
        ROOT::Core
        ROOT::EG
    )

endmacro()


# Adds FastJet for a plugin
macro(plugin_add_fastjet _name)

    if(NOT FASTJET_FOUND)
        find_package(FastJet REQUIRED)
    endif()

    # Add include directories
    plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${FASTJET_INCLUDE_DIRS} )

    # Add libraries
    plugin_link_libraries(${PLUGIN_NAME} ${FASTJET_LIBRARIES})

endmacro()
