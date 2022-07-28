# Common macro to add plugins
macro(plugin_add _name)

    project(${_name}_project)

    # Include fmt by default because... why not?
    find_package(fmt REQUIRED)
    set(fmt_INCLUDE_DIR ${fmt_DIR}/../../../include)

    # Define library
    add_library(${_name}_library STATIC "")
    target_include_directories(${_name}_library PUBLIC ${CMAKE_SOURCE_DIR})
    target_include_directories(${_name}_library SYSTEM PRIVATE ${fmt_INCLUDE_DIR})
    set_target_properties(BEMC_library PROPERTIES PREFIX "lib" OUTPUT_NAME "${_name}" SUFFIX ".so")

    # Define plugin
    add_library(${_name}_plugin SHARED ${PLUGIN_SOURCES})
    target_include_directories(${_name}_plugin PUBLIC ${CMAKE_SOURCE_DIR})
    target_include_directories(${_name}_plugin SYSTEM PRIVATE ${fmt_INCLUDE_DIR})
    set_target_properties(${_name}_plugin PROPERTIES PREFIX "" OUTPUT_NAME "${_name}" SUFFIX ".so")

    # Install plugin and library
    install(TARGETS ${_name}_plugin DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})
    install(TARGETS ${_name}_library DESTINATION ${PLUGIN_LIBRARY_OUTPUT_DIRECTORY})

    # we don't want our plugins to start with 'lib' prefix. We do want them to end with '.so' rather than '.dylib'
    # example: we want vmeson.so but not libvmeson.so or libvmeson.dylib
    set_target_properties(${_name}_plugin PROPERTIES PREFIX "" SUFFIX ".so")
endmacro()

# target_link_libraries for both a plugin and a library
macro(plugin_link_libraries _name)
    target_link_libraries(${_name}_plugin  ${ARGN})
    target_link_libraries(${_name}_library ${ARGN})
endmacro()

# target_include_directories for both a plugin and a library
macro(plugin_include_directories _name)
    target_include_directories(${_name}_plugin  ${ARGN})
    target_include_directories(${_name}_library ${ARGN})
endmacro()




# runs target_sources both for library and a plugin
macro(plugin_sources _name)
    # This is needed as this is a macro (see cmake macro documentation)
    set(SOURCES ${ARGN})

    # Plugin don't need <plugin_name>.cc in library
    set(PLUGIN_CC_FILE ${_name}.cc)
    get_filename_component(PLUGIN_CC_FILE "${CMAKE_CURRENT_LIST_DIR}/${PLUGIN_CC_FILE}" ABSOLUTE)
    list(REMOVE_ITEM SOURCES ${PLUGIN_CC_FILE})

    target_sources(${_name}_plugin PRIVATE ${SOURCES})
    target_sources(${_name}_library PRIVATE ${SOURCES})
endmacro()

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
macro(plugin_glob_all _name)

    # But... GLOB here makes this file just hot pluggable
    file(GLOB LIB_SRC_FILES *.cc *.cpp *.c)
    file(GLOB PLUGIN_SRC_FILES *.cc *.cpp *.c)
    file(GLOB HEADER_FILES *.h *.hh *.hpp)

    # Library don't need <plugin_name>.cc but Plugin does
    set(PLUGIN_CC_FILE ${_name}.cc)

    # Make the path absolute as GLOB files will be absolute paths
    get_filename_component(PLUGIN_CC_FILE_ABS "${CMAKE_CURRENT_LIST_DIR}/${PLUGIN_CC_FILE}" ABSOLUTE)

    # Remove plugin.cc file from libraries
    list(REMOVE_ITEM LIB_SRC_FILES ${PLUGIN_CC_FILE_ABS})

    # We need plugin relative path for correct installation
    string(REPLACE ${CMAKE_SOURCE_DIR} "" PLUGIN_RELATIVE_PATH ${PROJECT_SOURCE_DIR})

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
    message(STATUS "Plugin-only file is: ${PLUGIN_CC_FILE}")
    message(STATUS "Header files:")
    print_file_names("  " ${HEADER_FILES})  # Prints header files

    # Add sources to target
    target_sources(${_name}_plugin PRIVATE ${PLUGIN_SRC_FILES})
    target_sources(${_name}_library PRIVATE ${LIB_SRC_FILES})

    #Add correct headers installation
    # Install headers for plugin

    install(FILES ${HEADER_FILES} DESTINATION include/${PLUGIN_RELATIVE_PATH}/${_name})
endmacro()