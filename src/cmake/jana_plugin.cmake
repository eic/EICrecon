# Common macro to add plugins
macro(add_plugin _name)

    project(_name_project)

    # Find dependencies
    find_package(fmt REQUIRED)
    set(fmt_INCLUDE_DIR ${fmt_DIR}/../../../include)

    # Define library
    add_library(${_name}_library STATIC "")
    target_include_directories(${_name}_library INTERFACE ${CMAKE_SOURCE_DIR})
    #set_target_properties(BEMC_library PROPERTIES PREFIX "lib" OUTPUT_NAME "BEMC" SUFFIX ".so")

    # Define plugin
    add_library(${_name}_plugin SHARED ${PLUGIN_SOURCES})
    target_include_directories(${_name}_plugin PUBLIC ${CMAKE_SOURCE_DIR})
    set_target_properties(${_name}_plugin PROPERTIES PREFIX "" OUTPUT_NAME "${_name}" SUFFIX ".so")

    # Install plugin and library
    install(TARGETS ${_name}_plugin DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})
    install(TARGETS ${_name}_library DESTINATION ${PLUGIN_LIBRARY_OUTPUT_DIRECTORY})

    # Install headers for plugin
    file(GLOB ALL_HEADERS "*.h*")
    install(FILES ${ALL_HEADERS} DESTINATION include/detectors/${_name})




    # we don't want our plugins to start with 'lib' prefix. We do want them to end with '.so' rather than '.dylib'
    # example: we want vmeson.so but not libvmeson.so or lybvmeson.dylib
    set_target_properties(${_name} PROPERTIES PREFIX "" SUFFIX ".so")

    install(TARGETS ${_name}
            DESTINATION plugins
            PUBLIC_HEADER DESTINATION include/plugins)
endmacro()