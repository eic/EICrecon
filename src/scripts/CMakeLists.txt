
find_package(ROOT)
find_package(podio)

if(DEFINED ENV{DETECTOR_PATH})
    set(DETECTOR_PATH $ENV{DETECTOR_PATH})
endif()
if(DEFINED ENV{BEAMLINE_PATH})
    set(BEAMLINE_PATH $ENV{BEAMLINE_PATH})
endif()


configure_file(eicrecon-this.sh.in  eicrecon-this.sh   @ONLY)
configure_file(eicrecon-this.csh.in eicrecon-this.csh  @ONLY)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/eicrecon-this.sh   DESTINATION bin)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/eicrecon-this.csh  DESTINATION bin)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/eicmkplugin.py     DESTINATION bin)
