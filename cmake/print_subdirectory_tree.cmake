macro(print_subdirectory_tree)
    get_property(lvl1_dirs DIRECTORY PROPERTY SUBDIRECTORIES)
    foreach(lvl1_dir ${lvl1_dirs})
        # Here we have basic directories like : programs, plugins, etc.
        get_filename_component(lvl1_dir_name ${lvl1_dir} NAME)
        message(STATUS "${lvl1_dir_name}")

        # Go deeper)
        get_property(lvl2_dirs DIRECTORY ${lvl1_dir} PROPERTY SUBDIRECTORIES)
        foreach(lvl2_dir ${lvl2_dirs})
            get_filename_component(lvl2_dir_name ${lvl2_dir} NAME)
            message(STATUS "   ${lvl2_dir_name}")

            #if it is plugins directory go even deeper
            if ("${lvl1_dir_name}" STREQUAL "plugins")
                get_property(lvl3_dirs DIRECTORY ${lvl2_dir} PROPERTY SUBDIRECTORIES)
                foreach(lvl3_dir ${lvl3_dirs})
                    get_filename_component(lvl3_dir_name ${lvl3_dir} NAME)
                    message(STATUS "      ${lvl3_dir_name}")
                endforeach()
            endif()
        endforeach()
    endforeach()

endmacro()
