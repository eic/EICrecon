cmake_minimum_required(VERSION 3.16)

# Automatically set plugin name the same as the directory name
# Don't forget string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in its name
get_filename_component(TOOL_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

print_header(">>>>   T O O L :   ${TOOL_NAME}    <<<<")       # Fancy printing

project(${TOOL_NAME})

#
#add_custom_target(${TOOL_NAME}_tgt
#        ALL
#        OUTPUT  launch_eicrecon_reco_flags.py         # Treated as relative to CMAKE_CURRENT_BINARY_DIR
#        COMMAND python3 ${PROJECT_SOURCE_DIR}/gen_launch_script.py
#        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
#        DEPENDS ${PROJECT_SOURCE_DIR}/gen_launch_script.py ${PROJECT_SOURCE_DIR}/launch_script.in.py
#        COMMENT "-------- ${TOOL_NAME}_tgt -----------"
#)

install(FILES
        reco_flags.py
        RENAME run_eicrecon_reco_flags.py
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

