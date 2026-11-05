# Arrow IPC Stream Reader Test
#
# Verifies that the installed podio plugin contains the Arrow stream reader.
# Only added when EICRECON_HAS_ARROW is enabled (see top-level CMakeLists.txt).

# Add the test script
add_test(NAME arrow_stream_test
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_arrow_stream.sh)

# Set test properties
set_tests_properties(arrow_stream_test PROPERTIES LABELS "arrow;stream" TIMEOUT
                                                  60)

# Point the test at the installed eicrecon and plugins
set_tests_properties(
  arrow_stream_test
  PROPERTIES ENVIRONMENT "EICRECON_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
