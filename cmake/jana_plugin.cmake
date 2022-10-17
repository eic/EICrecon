# Common macro to add plugins
macro(plugin_add _name)

    project(${_name}_project)

    # Check if build with library
    foreach(arg IN ITEMS ${ARGN})
        if(${arg} STREQUAL "WITH_STATIC_LIBRARY")
            set(${_name}_WITH_STATIC_LIB ON)
        endif()
        if(${arg} STREQUAL "WITH_STATIC_LIB")       # alternative
            set(${_name}_WITH_STATIC_LIB ON)
        endif()
    endforeach()

    # Include JANA by default
    find_package(JANA REQUIRED)

    # include logging by default
    find_package(spdlog REQUIRED)
    find_package(fmt REQUIRED)
    set(fmt_INCLUDE_DIR ${fmt_DIR}/../../../include)

    # include ROOT by default
    find_package(ROOT REQUIRED)

    # Define plugin
    add_library(${_name}_plugin SHARED ${PLUGIN_SOURCES})
    target_include_directories(${_name}_plugin PUBLIC ${CMAKE_SOURCE_DIR}/src)
    target_include_directories(${_name}_plugin SYSTEM PUBLIC ${JANA_INCLUDE_DIR} )
    target_include_directories(${_name}_plugin SYSTEM PUBLIC ${ROOT_INCLUDE_DIRS} )
    target_include_directories(${_name}_plugin SYSTEM PUBLIC ${fmt_INCLUDE_DIR} )
    set_target_properties(${_name}_plugin PROPERTIES PREFIX "" OUTPUT_NAME "${_name}" SUFFIX ".so")
    target_link_libraries(${_name}_plugin ${JANA_LIB} spdlog::spdlog)

    # Install plugin
    install(TARGETS ${_name}_plugin DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})


    if(${_name}_WITH_STATIC_LIB)
        # Define library
        add_library(${_name}_library STATIC "")
	    target_include_directories(${_name}_library PUBLIC ${CMAKE_SOURCE_DIR}/src)
        target_include_directories(${_name}_library SYSTEM PUBLIC ${JANA_INCLUDE_DIR} )
        target_include_directories(${_name}_library SYSTEM PUBLIC ${fmt_INCLUDE_DIR} )
        set_target_properties(${_name}_library PROPERTIES PREFIX "lib" OUTPUT_NAME "${_name}" SUFFIX ".a")
        target_link_libraries(${_name}_library ${JANA_LIB} spdlog::spdlog)

        # Install plugin
        install(TARGETS ${_name}_library DESTINATION ${PLUGIN_LIBRARY_OUTPUT_DIRECTORY})
    endif()     # WITH_STATIC_LIB
endmacro()


# target_link_libraries for both a plugin and a library
macro(plugin_link_libraries _name)

    #foreach(arg IN ITEMS ${ARGN})
    #    target_link_libraries(${_name}_plugin ${arg})
    target_link_libraries(${_name}_plugin ${ARGN})
    #endforeach()



    if(${_name}_WITH_STATIC_LIB)
        target_link_libraries(${_name}_library ${ARGN})
    endif()     # WITH_STATIC_LIB
endmacro()


# target_include_directories for both a plugin and a library
macro(plugin_include_directories _name)
    target_include_directories(${_name}_plugin  ${ARGN})

    if(${_name}_WITH_STATIC_LIB)
        target_include_directories(${_name}_library ${ARGN})
    endif()     # WITH_STATIC_LIB
endmacro()


# runs target_sources both for library and a plugin
macro(plugin_sources _name)
    # This is needed as this is a macro (see cmake macro documentation)
    set(SOURCES ${ARGN})

    # Add sources to plugin
    target_sources(${_name}_plugin PRIVATE ${SOURCES})

    if(${_name}_WITH_STATIC_LIB)
        # Library don't need <plugin_name>.cc in library
        set(PLUGIN_CC_FILE ${_name}.cc)
        get_filename_component(PLUGIN_CC_FILE "${CMAKE_CURRENT_LIST_DIR}/${PLUGIN_CC_FILE}" ABSOLUTE)
        list(REMOVE_ITEM SOURCES ${PLUGIN_CC_FILE})

        # Add sources to library
        target_sources(${_name}_library PRIVATE ${SOURCES})
    endif()     # WITH_STATIC_LIB
endmacro()

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
macro(plugin_glob_all _name)

    # But... GLOB here makes this file just hot pluggable
    file(GLOB LIB_SRC_FILES *.cc *.cpp *.c)
    file(GLOB PLUGIN_SRC_FILES *.cc *.cpp *.c)
    file(GLOB HEADER_FILES *.h *.hh *.hpp)

    # We need plugin relative path for correct headers installation
    string(REPLACE ${CMAKE_SOURCE_DIR}/src "" PLUGIN_RELATIVE_PATH ${PROJECT_SOURCE_DIR})

    # Add sources to plugin
    target_sources(${_name}_plugin PRIVATE ${PLUGIN_SRC_FILES})

    #Add correct headers installation
    # Install headers for plugin
    install(FILES ${HEADER_FILES} DESTINATION include/${PLUGIN_RELATIVE_PATH})

    if(${_name}_WITH_STATIC_LIB)
        # Library don't need <plugin_name>.cc but Plugin does
        set(PLUGIN_CC_FILE ${_name}.cc)

        # Make the path absolute as GLOB files will be absolute paths
        get_filename_component(PLUGIN_CC_FILE_ABS "${CMAKE_CURRENT_LIST_DIR}/${PLUGIN_CC_FILE}" ABSOLUTE)

        # Remove plugin.cc file from libraries
        list(REMOVE_ITEM LIB_SRC_FILES ${PLUGIN_CC_FILE_ABS})

        # >oO Debug output if needed
        if(${EICRECON_VERBOSE_CMAKE})
            message(STATUS "plugin_glob_all:${_name}: LIB_SRC_FILES    ${LIB_SRC_FILES}")
        endif()

        # Finally add sources to library
        target_sources(${_name}_library PRIVATE ${LIB_SRC_FILES})
    endif()     # WITH_STATIC_LIB

    # >oO Debug output if needed
    if(${EICRECON_VERBOSE_CMAKE})
        message(STATUS "plugin_glob_all:${_name}: PLUGIN_CC_FILE   ${PLUGIN_CC_FILE}")
        message(STATUS "plugin_glob_all:${_name}: LIB_SRC_FILES    ${LIB_SRC_FILES}")
        message(STATUS "plugin_glob_all:${_name}: PLUGIN_SRC_FILES ${PLUGIN_SRC_FILES}")
        message(STATUS "plugin_glob_all:${_name}: HEADER_FILES     ${HEADER_FILES}")
        message(STATUS "plugin_glob_all:${_name}: PLUGIN_RLTV_PATH ${PLUGIN_RELATIVE_PATH}")
    endif()

    # To somehow control GLOB lets at least PRINT files we are going to compile:
    message(STATUS "Source files:")
    print_file_names("  " ${PLUGIN_SRC_FILES})    # Prints source files
    message(STATUS "Plugin-main file is: ${PLUGIN_CC_FILE}")
    message(STATUS "Header files:")
    print_file_names("  " ${HEADER_FILES})  # Prints header files

endmacro()


# dd4hep - plugin_add_dd4hep
macro(plugin_add_dd4hep _name)

    if(NOT DD4hep_FOUND)
        find_package(DD4hep REQUIRED)
    endif()

    plugin_include_directories(${_name} SYSTEM PUBLIC ${DD4hep_INCLUDE_DIRS})
    plugin_link_libraries(${_name} DD4hep::DDCore DD4hep::DDRec)

endmacro()


# ACTS - plugin_add_dd4hep
macro(plugin_add_acts _name)

    if(NOT Acts_FOUND)
        find_package(Acts REQUIRED COMPONENTS Core PluginIdentification PluginTGeo PluginDD4hep)
        set(Acts_VERSION_MIN "19.0.0")
        set(Acts_VERSION "${Acts_VERSION_MAJOR}.${Acts_VERSION_MINOR}.${Acts_VERSION_PATCH}")
        if(${Acts_VERSION} VERSION_LESS ${Acts_VERSION_MIN}
                AND NOT "${Acts_VERSION}" STREQUAL "9.9.9")
            message(FATAL_ERROR "Acts version ${Acts_VERSION_MIN} or higher required, but ${Acts_VERSION} found")
        endif()

        set(Acts_INCLUDE_DIRS ${Acts_DIR}/../../../include ${ActsDD4hep_DIR}/../../../include )
    endif()

    # Add include directories (works same as target_include_directories)
    plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${Acts_INCLUDE_DIRS})

    # Add libraries (works same as target_include_directories)
    plugin_link_libraries(${PLUGIN_NAME} ActsCore ActsPluginIdentification ActsPluginTGeo ActsPluginDD4hep)
endmacro()