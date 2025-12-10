# Eigen malloc configuration detection test
#
# This detects what value of EIGEN_MALLOC_ALREADY_ALIGNED was used when Acts was
# compiled and automatically adds the appropriate compile definition to ensure
# EICrecon matches.

# Only run if Acts is available (should always be true in EICrecon)
if(TARGET ${Acts_NAMESPACE_PREFIX}Core)

  message(STATUS "Detecting Eigen malloc configuration used by Acts...")

  # Get Acts library location to pass to the detector
  get_target_property(ACTS_CORE_LOCATION ${Acts_NAMESPACE_PREFIX}Core LOCATION)

  # Use try_run to compile and run the detector at configure time We use
  # dlopen/dlsym so we don't link against Acts or include Eigen headers
  try_run(
    EIGEN_MALLOC_RUN_RESULT
    EIGEN_MALLOC_COMPILE_RESULT
    ${CMAKE_BINARY_DIR}/cmake/detect_acts_eigen_malloc
    SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/detect_acts_eigen_malloc.cpp
    LINK_LIBRARIES
    ${CMAKE_DL_LIBS}
    RUN_OUTPUT_VARIABLE EIGEN_MALLOC_OUTPUT
    COMPILE_OUTPUT_VARIABLE EIGEN_MALLOC_COMPILE_OUTPUT
    ARGS "${ACTS_CORE_LOCATION}")

  if(NOT EIGEN_MALLOC_COMPILE_RESULT)
    message(WARNING "Failed to compile Eigen malloc detection test")
    message(WARNING "Compile output: ${EIGEN_MALLOC_COMPILE_OUTPUT}")
    message(WARNING "Assuming EIGEN_MALLOC_ALREADY_ALIGNED=0 for safety")
    set(ACTS_EIGEN_MALLOC_VALUE 0)
  elseif(EIGEN_MALLOC_RUN_RESULT EQUAL 0)
    set(ACTS_EIGEN_MALLOC_VALUE 0)
    message(
      STATUS
        "Acts uses EIGEN_MALLOC_ALREADY_ALIGNED=0 (handmade_aligned_malloc)")
  elseif(EIGEN_MALLOC_RUN_RESULT EQUAL 1)
    set(ACTS_EIGEN_MALLOC_VALUE 1)
    message(STATUS "Acts uses EIGEN_MALLOC_ALREADY_ALIGNED=1 (system malloc)")
  else()
    message(
      WARNING
        "Could not detect Acts' Eigen malloc configuration (exit code: ${EIGEN_MALLOC_RUN_RESULT})"
    )
    message(WARNING "Output: ${EIGEN_MALLOC_OUTPUT}")
    message(WARNING "Assuming EIGEN_MALLOC_ALREADY_ALIGNED=0 for safety")
    set(ACTS_EIGEN_MALLOC_VALUE 0)
  endif()

  # Add the compile definition globally to match Acts
  add_compile_definitions(
    EIGEN_MALLOC_ALREADY_ALIGNED=${ACTS_EIGEN_MALLOC_VALUE})
  message(
    STATUS
      "Set EIGEN_MALLOC_ALREADY_ALIGNED=${ACTS_EIGEN_MALLOC_VALUE} globally to match Acts"
  )

  # Also build the detector as a standalone executable for manual verification
  add_executable(detect_acts_eigen_malloc
                 ${CMAKE_CURRENT_LIST_DIR}/detect_acts_eigen_malloc.cpp)
  target_link_libraries(detect_acts_eigen_malloc PRIVATE ${CMAKE_DL_LIBS})

  # Add as a test - pass Acts library location as argument
  add_test(NAME detect_acts_eigen_malloc COMMAND detect_acts_eigen_malloc
                                                 "${ACTS_CORE_LOCATION}")

  # Make it easy to run manually
  add_custom_target(
    check_eigen_malloc
    COMMAND detect_acts_eigen_malloc "${ACTS_CORE_LOCATION}"
    DEPENDS detect_acts_eigen_malloc
    COMMENT "Verifying Eigen malloc configuration matches Acts..."
    VERBATIM)

endif()
