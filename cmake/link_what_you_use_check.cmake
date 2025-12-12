# Custom LINK_WHAT_YOU_USE check script for EICrecon which wraps the
# default `ldd -u -r` check to log output to a file

# Arguments passed by CMake: cmake -D OUTPUT_DIR=... -P script.cmake <binary_file>
#
# The binary file is the last argument after all -D flags

# Find the last argument (the binary file) using CMAKE_ARGC
if(CMAKE_ARGC GREATER 0)
  math(EXPR LAST_IDX "${CMAKE_ARGC} - 1")
  set(BINARY_FILE "${CMAKE_ARGV${LAST_IDX}}")
else()
  set(BINARY_FILE "")
endif()

if(NOT BINARY_FILE
   OR BINARY_FILE STREQUAL ""
   OR BINARY_FILE MATCHES "^-")
  message(FATAL_ERROR "No binary file specified or malformed arguments")
endif()

# Get absolute path of binary
get_filename_component(BINARY_FILE "${BINARY_FILE}" ABSOLUTE)

# Use the passed output directory
if(NOT DEFINED OUTPUT_DIR OR OUTPUT_DIR STREQUAL "")
  get_filename_component(OUTPUT_DIR "${BINARY_FILE}" DIRECTORY)
  set(OUTPUT_DIR "${OUTPUT_DIR}/link_what_you_use")
endif()

# Create output directory
file(MAKE_DIRECTORY "${OUTPUT_DIR}")

# Get binary name for log file
get_filename_component(BINARY_NAME "${BINARY_FILE}" NAME)
set(LOG_FILE "${OUTPUT_DIR}/${BINARY_NAME}.lwyu.log")

# Run ldd -u -r on the binary
execute_process(
  COMMAND ldd -u -r "${BINARY_FILE}"
  RESULT_VARIABLE LDD_RESULT
  OUTPUT_VARIABLE LDD_OUTPUT
  ERROR_VARIABLE LDD_ERROR
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)

# Write output to log file
file(WRITE "${LOG_FILE}" "LINK_WHAT_YOU_USE check for: ${BINARY_FILE}\n")
string(TIMESTAMP CURRENT_TIME "%Y-%m-%d %H:%M:%S")
file(APPEND "${LOG_FILE}" "Date: ${CURRENT_TIME}\n")
file(APPEND "${LOG_FILE}" "==========================================\n\n")

if(LDD_OUTPUT)
  file(APPEND "${LOG_FILE}" "${LDD_OUTPUT}\n")
endif()
if(LDD_ERROR)
  file(APPEND "${LOG_FILE}" "${LDD_ERROR}\n")
endif()
