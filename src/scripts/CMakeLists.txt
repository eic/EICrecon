configure_file(eicrecon-this.sh.in eicrecon-this.sh @ONLY)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/eicrecon-this.sh DESTINATION bin)
