# HELPERS TO PRINT STUFF: FANCY HEADERS, FILE NAMES, ETC

# Prints file names instead of full paths
macro(print_file_names _prefix)
    foreach (file_path ${ARGN})
        get_filename_component(file_name ${file_path} NAME)
        message(STATUS "${_prefix}   ${file_name}")
    endforeach(file_path)
endmacro()


# This macro prints a fancy header around the string. Like:
#    =========================================
#    >>>> P L U G I N :   jleic_occupancy <<<<
#    =========================================
macro(print_grand_header _header_text)
    string(LENGTH ${_header_text} HEADER_TEXT_LENGTH)
    string(RANDOM LENGTH ${HEADER_TEXT_LENGTH} ALPHABET "=" HEADER)
    message(STATUS "\n${HEADER}\n${_header_text}\n${HEADER}")
endmacro()


# This macro prints a fancy header around the string. Like:
# >>>>   P L U G I N :   hepmc_reader    <<<<
# -------------------------------------------
macro(print_header _header_text)
    string(LENGTH ${_header_text} HEADER_TEXT_LENGTH)
    string(RANDOM LENGTH ${HEADER_TEXT_LENGTH} ALPHABET "-" HEADER)
    message(STATUS "\n${_header_text}\n${HEADER}")
endmacro()

# Useful for debugging. Copied from:
# https://stackoverflow.com/questions/9298278/cmake-print-out-all-accessible-variables-in-a-script
function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif()
        endif()
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()
