# Arrow IPC Stream Reader Test
#
# This is a proof-of-concept test that verifies the Arrow stream reader
# framework is properly integrated into EICrecon.
#
# Full end-to-end testing will be enabled once: 1. DD4hep PR #1658 (Arrow
# writer) is merged 2. podio >= 1.8 with Arrow support is available in eic-shell

# Add the test script
add_test(NAME arrow_stream_test
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_arrow_stream.sh)

# Set test properties
set_tests_properties(
  arrow_stream_test PROPERTIES LABELS "arrow;stream;proof-of-concept" TIMEOUT
                               60)

# Set environment variables if needed
set_tests_properties(
  arrow_stream_test PROPERTIES ENVIRONMENT
                               "EIC_SHELL_PREFIX=${CMAKE_INSTALL_PREFIX}")
