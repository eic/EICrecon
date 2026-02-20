
find_program(RUN_CLANG_TIDY run-clang-tidy-19)
find_program(CLANG_APPLY_REPLACEMENTS clang-apply-replacements-19)
find_program(CLANG_FORMAT clang-format-19)

if(RUN_CLANG_TIDY
   AND CLANG_APPLY_REPLACEMENTS
   AND CLANG_FORMAT)

  add_custom_target(
    check-tidy
    COMMAND ${RUN_CLANG_TIDY} -p ${CMAKE_BINARY_DIR} -config-file
            ${CMAKE_SOURCE_DIR}/.clang-tidy -warnings-as-errors=*
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running clang-tidy"
    VERBATIM)

  add_custom_target(
    tidy
    COMMAND
      ${RUN_CLANG_TIDY} -p ${CMAKE_BINARY_DIR} -config-file
      ${CMAKE_SOURCE_DIR}/.clang-tidy -export-fixes
      ${CMAKE_BINARY_DIR}/clang-tidy-fixes/
    COMMAND
      ${CLANG_APPLY_REPLACEMENTS} -style-config
      ${CMAKE_SOURCE_DIR}/.clang-format --remove-change-desc-files
      ${CMAKE_BINARY_DIR}/clang-tidy-fixes/
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running clang-tidy"
    DEPENDS tidy
    VERBATIM)

  add_custom_target(
    check-format
    COMMAND git ls-files "*.cpp" "*.hpp" "*.cc" "*.cxx" "*.h" "*.hh" | xargs
            ${CLANG_FORMAT} -i --dry-run --Werror
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running clang-format on tracked sources"
    VERBATIM)

  add_custom_target(
    format
    COMMAND git ls-files "*.cpp" "*.hpp" "*.cc" "*.cxx" "*.h" "*.hh" | xargs
            ${CLANG_FORMAT} -i
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running clang-format on tracked sources"
    VERBATIM)

  add_custom_target(
    style
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target format
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target tidy
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target check-tidy
    USES_TERMINAL)
else()
  message(
    WARNING
      "clang-tidy, clang-apply-replacements, or clang-format not found: 'style', 'tidy', and 'format' targets disabled"
  )
endif()
