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
