cmake_minimum_required(VERSION 3.16)

# DD4hep relies on LD_LIBRARY_PATH at run time to find .components files
# which themselves refer to symbols and plugins (shared libraries) that
# contain them. It opens these plugins with libdl so they symbols do not
# need to be linked in here at compile time. All of this is to say we
# should not link the ip6 of epic gemetry libraries here.

# Automatically set plugin name the same as the directory name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
print_header(">>>>   P L U G I N :   ${PLUGIN_NAME}    <<<<")       # Fancy printing


