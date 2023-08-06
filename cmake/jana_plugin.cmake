# Common macro to add plugins and libraries
macro(eicrecon_add _name)
    # Libraries don't need <plugin_name>.cc
    set(PLUGIN_CC_FILE ${_name}.cc)

    # Glob all source files
    file(GLOB LIB_SRC_FILES CONFIGURE_DEPENDS *.cc *.cpp *.c)

    # Make the path absolute as GLOB files will be absolute paths
    get_filename_component(PLUGIN_CC_FILE_ABS "${CMAKE_CURRENT_LIST_DIR}/${PLUGIN_CC_FILE}" ABSOLUTE)

    # Remove plugin.cc file from libraries
    list(REMOVE_ITEM LIB_SRC_FILES ${PLUGIN_CC_FILE_ABS})

    # Add library if *.cc has more than just <plugin_name>.cc
    if(LIB_SRC_FILES)
        eicrecon_library_add(${_name})
    endif()

    # Add plugin if <plugin_name>.cc if found
    if(EXISTS ${PLUGIN_CC_FILE})
        eicrecon_plugin_add(${_name})
    endif()
endmacro()

# Common macro to add libraries
macro(eicrecon_library_add _name)

    project(${_name}_project)

    # Include JANA by default
    find_package(JANA REQUIRED)

    # include logging by default
    find_package(spdlog REQUIRED)

    # include fmt by default
    find_package(fmt REQUIRED)

    # Check if build with library
    foreach(arg IN ITEMS ${ARGN})
        if(${arg} STREQUAL "WITH_STATIC_LIBRARY")
            set(${_name}_WITH_STATIC_LIB ON)
        endif()
        if(${arg} STREQUAL "WITH_STATIC_LIB")       # alternative
            set(${_name}_WITH_STATIC_LIB ON)
        endif()
    endforeach()

    # Define library
    if(${_name}_WITH_STATIC_LIB)
        add_library(${_name}_library STATIC "")
        set_target_properties(${_name}_library PROPERTIES PREFIX "lib" OUTPUT_NAME "${_name}" SUFFIX ".a")
    else()
        add_library(${_name}_library SHARED "")
        set_target_properties(${_name}_library PROPERTIES PREFIX "lib" OUTPUT_NAME "${_name}" SUFFIX ".so")
    endif()

    target_include_directories(${_name}_library PUBLIC ${EICRECON_SOURCE_DIR}/src)
    target_include_directories(${_name}_library SYSTEM PUBLIC ${JANA_INCLUDE_DIR})
    target_link_libraries(${_name}_library ${JANA_LIB} spdlog::spdlog)
    target_link_libraries(${_name}_library ${JANA_LIB} fmt::fmt)

    # Install library
    install(TARGETS ${_name}_library DESTINATION ${PLUGIN_LIBRARY_OUTPUT_DIRECTORY})

endmacro()

# Common macro to add plugins
macro(eicrecon_plugin_add _name)

    project(${_name}_project)

    # Include JANA by default
    find_package(JANA REQUIRED)

    # include logging by default
    find_package(spdlog REQUIRED)

    # include fmt by default
    find_package(fmt REQUIRED)

    # Define plugin
    add_library(${_name}_plugin SHARED ${PLUGIN_SOURCES})

    target_include_directories(${_name}_plugin PUBLIC ${EICRECON_SOURCE_DIR}/src)
    target_include_directories(${_name}_plugin SYSTEM PUBLIC ${JANA_INCLUDE_DIR} )
    target_include_directories(${_name}_plugin SYSTEM PUBLIC ${ROOT_INCLUDE_DIRS} )
    set_target_properties(${_name}_plugin PROPERTIES PREFIX "" OUTPUT_NAME "${_name}" SUFFIX ".so")
    target_link_libraries(${_name}_plugin ${JANA_LIB} spdlog::spdlog)
    target_link_libraries(${_name}_plugin ${JANA_LIB} fmt::fmt)

    # Install plugin
    install(TARGETS ${_name}_plugin DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})

endmacro()


# target_link_libraries for both a plugin and a library
macro(eicrecon_link_libraries _name)
    if(TARGET ${_name}_plugin)
        target_link_libraries(${_name}_plugin ${ARGN})
    endif()

    if(TARGET ${_name}_library)
        target_link_libraries(${_name}_library ${ARGN})
    endif()
endmacro()


# target_include_directories for both a plugin and a library
macro(eicrecon_include_directories _name)
    if(TARGET ${_name}_plugin)
        target_include_directories(${_name}_plugin  ${ARGN})
    endif()

    if(TARGET ${_name}_library)
        target_include_directories(${_name}_library ${ARGN})
    endif()
endmacro()


# runs target_sources both for library and a plugin
macro(eicrecon_sources _name)
    # This is needed as this is a macro (see cmake macro documentation)
    set(SOURCES ${ARGN})

    # Add sources to plugin
    if(TARGET ${_name}_plugin)
        target_sources(${_name}_plugin PRIVATE ${SOURCES})
    endif()

    if(TARGET ${_name}_library)
        # Libraries don't need <plugin_name>.cc
        set(PLUGIN_CC_FILE ${_name}.cc)

        # Make the path absolute as GLOB files will be absolute paths
        get_filename_component(PLUGIN_CC_FILE_ABS "${CMAKE_CURRENT_LIST_DIR}/${PLUGIN_CC_FILE}" ABSOLUTE)

        # Remove plugin.cc file from libraries
        list(REMOVE_ITEM LIB_SRC_FILES ${PLUGIN_CC_FILE_ABS})

        # >oO Debug output if needed
        if(${EICRECON_VERBOSE_CMAKE})
            message(STATUS "eicrecon_sources:${_name}: LIB_SRC_FILES    ${LIB_SRC_FILES}")
        endif()

        # Add sources to library
        target_sources(${_name}_library PRIVATE ${SOURCES})
    endif()
endmacro()

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
macro(eicrecon_glob_all _name)

    # But... GLOB here makes this file just hot pluggable
    file(GLOB LIB_SRC_FILES CONFIGURE_DEPENDS *.cc *.cpp *.c)
    file(GLOB PLUGIN_SRC_FILES CONFIGURE_DEPENDS *.cc *.cpp *.c)
    file(GLOB HEADER_FILES CONFIGURE_DEPENDS *.h *.hh *.hpp)

    # Add sources to plugin
    if(TARGET ${_name}_plugin)
        # We need plugin relative path for correct headers installation
        string(REPLACE ${EICRECON_SOURCE_DIR}/src "" PLUGIN_RELATIVE_PATH ${PROJECT_SOURCE_DIR})

        # >oO Debug output if needed
        if(${EICRECON_VERBOSE_CMAKE})
            message(STATUS "eicrecon_glob_all:${_name}: PLUGIN_SRC_FILES    ${PLUGIN_SRC_FILES}")
        endif()

        # Finally add sources to plugin
        target_sources(${_name}_plugin PRIVATE ${PLUGIN_SRC_FILES})

        # Add correct headers installation
        install(FILES ${HEADER_FILES} DESTINATION include/${PLUGIN_RELATIVE_PATH})
    endif()

    if(TARGET ${_name}_library)
        # Libraries don't need <plugin_name>.cc
        set(PLUGIN_CC_FILE ${_name}.cc)

        # Make the path absolute as GLOB files will be absolute paths
        get_filename_component(PLUGIN_CC_FILE_ABS "${CMAKE_CURRENT_LIST_DIR}/${PLUGIN_CC_FILE}" ABSOLUTE)

        # Remove plugin.cc file from libraries
        list(REMOVE_ITEM LIB_SRC_FILES ${PLUGIN_CC_FILE_ABS})

        # >oO Debug output if needed
        if(${EICRECON_VERBOSE_CMAKE})
            message(STATUS "eicrecon_glob_all:${_name}: LIB_SRC_FILES    ${LIB_SRC_FILES}")
        endif()

        # Finally add sources to library
        target_sources(${_name}_library PRIVATE ${LIB_SRC_FILES})
    endif()

    # >oO Debug output if needed
    if(${EICRECON_VERBOSE_CMAKE})
        message(STATUS "eicrecon_glob_all:${_name}: PLUGIN_CC_FILE   ${PLUGIN_CC_FILE}")
        message(STATUS "eicrecon_glob_all:${_name}: LIB_SRC_FILES    ${LIB_SRC_FILES}")
        message(STATUS "eicrecon_glob_all:${_name}: PLUGIN_SRC_FILES ${PLUGIN_SRC_FILES}")
        message(STATUS "eicrecon_glob_all:${_name}: HEADER_FILES     ${HEADER_FILES}")
        message(STATUS "eicrecon_glob_all:${_name}: PLUGIN_RLTV_PATH ${PLUGIN_RELATIVE_PATH}")
    endif()

endmacro()


# Adds dd4hep for a plugin
macro(eicrecon_add_dd4hep _name)

    if(NOT DD4hep_FOUND)
        find_package(DD4hep REQUIRED)
    endif()

    eicrecon_link_libraries(${_name}
        DD4hep::DDCore
        DD4hep::DDRec
    )

endmacro()


# Adds Eigen3 for a plugin
macro(eicrecon_add_eigen3 _name)

    if(NOT Eigen3_FOUND)
        find_package(Eigen3 REQUIRED)
    endif()

    eicrecon_link_libraries(${_name}
        Eigen3::Eigen
    )

endmacro()


# Adds ACTS tracking package for a plugin
macro(eicrecon_add_acts _name)

    if(NOT Acts_FOUND)
        find_package(Acts REQUIRED COMPONENTS Core PluginIdentification PluginTGeo PluginJson PluginDD4hep)
        set(Acts_VERSION_MIN "20.2.0")
        set(Acts_VERSION "${Acts_VERSION_MAJOR}.${Acts_VERSION_MINOR}.${Acts_VERSION_PATCH}")
        if(${Acts_VERSION} VERSION_LESS ${Acts_VERSION_MIN}
                AND NOT "${Acts_VERSION}" STREQUAL "9.9.9")
            message(FATAL_ERROR "Acts version ${Acts_VERSION_MIN} or higher required, but ${Acts_VERSION} found")
        endif()
    endif()

    # Add libraries (works same as target_include_directories)
    eicrecon_link_libraries(${PLUGIN_NAME}
        ActsCore
        ActsPluginIdentification
        ActsPluginTGeo
        ActsPluginJson
        ActsPluginDD4hep
    )

endmacro()


# Adds IRT PID reconstruction package for a plugin
macro(eicrecon_add_irt _name)

    if(NOT IRT_FOUND)
        find_package(IRT REQUIRED)
    endif()

    eicrecon_link_libraries(${PLUGIN_NAME} IRT)

endmacro()

# Adds podio, edm4hep, edm4eic for a plugin
macro(eicrecon_add_event_model _name)

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
    # ${datamodel_BINARY_DIR} is an include path to datamodel_glue.h
    eicrecon_include_directories(${PLUGIN_NAME} PUBLIC ${datamodel_BINARY_DIR})

    # Add libraries
    # (same as target_include_directories but for both plugin and library)
    eicrecon_link_libraries(${PLUGIN_NAME}
        podio::podio
        EDM4EIC::edm4eic
        EDM4HEP::edm4hep
    )

endmacro()


# Adds cern ROOT for a plugin
macro(eicrecon_add_cern_root _name)

    if(NOT ROOT_FOUND)
        find_package(ROOT REQUIRED)
    endif()

    # Add libraries
    eicrecon_link_libraries(${PLUGIN_NAME}
        ROOT::Core
        ROOT::EG
    )

endmacro()


# Adds FastJet for a plugin
macro(eicrecon_add_fastjet _name)

    if(NOT FASTJET_FOUND)
        find_package(FastJet REQUIRED)
    endif()

    # Add include directories
    eicrecon_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${FASTJET_INCLUDE_DIRS} )

    # Add libraries
    eicrecon_link_libraries(${PLUGIN_NAME} ${FASTJET_LIBRARIES})

endmacro()
