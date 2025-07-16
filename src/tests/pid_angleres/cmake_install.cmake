# Install script for directory: /eic/u/mposik/Detector1/EPIC/EICrecon/src/tests/pid_angleres

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/eic/u/mposik/Detector1/EPIC/EICrecon")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/EICrecon/plugins/pid_angleres.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/EICrecon/plugins/pid_angleres.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/EICrecon/plugins/pid_angleres.so"
         RPATH "/eic/u/mposik/Detector1/EPIC/EICrecon/lib:/eic/u/mposik/Detector1/EPIC/EICrecon/lib/EICrecon/plugins:/opt/local/lib/root:/opt/software/linux-debian12-x86_64_v2/gcc-12.2.0/root-6.32.08-zuuhje3qysrygdknl2owlq5x3eqsfxmo/lib/root:/opt/software/linux-debian12-x86_64_v2/gcc-12.2.0/xerces-c-3.3.0-pg24w45sv2tqw3hlvgjdzauabzq3aoel/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/EICrecon/plugins" TYPE SHARED_LIBRARY FILES "/eic/u/mposik/Detector1/EPIC/EICrecon/src/tests/pid_angleres/pid_angleres.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/EICrecon/plugins/pid_angleres.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/EICrecon/plugins/pid_angleres.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/EICrecon/plugins/pid_angleres.so"
         OLD_RPATH "/opt/local/lib/root:/opt/software/linux-debian12-x86_64_v2/gcc-12.2.0/root-6.32.08-zuuhje3qysrygdknl2owlq5x3eqsfxmo/lib/root:/opt/software/linux-debian12-x86_64_v2/gcc-12.2.0/xerces-c-3.3.0-pg24w45sv2tqw3hlvgjdzauabzq3aoel/lib:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/eic/u/mposik/Detector1/EPIC/EICrecon/lib:/eic/u/mposik/Detector1/EPIC/EICrecon/lib/EICrecon/plugins:/opt/local/lib/root:/opt/software/linux-debian12-x86_64_v2/gcc-12.2.0/root-6.32.08-zuuhje3qysrygdknl2owlq5x3eqsfxmo/lib/root:/opt/software/linux-debian12-x86_64_v2/gcc-12.2.0/xerces-c-3.3.0-pg24w45sv2tqw3hlvgjdzauabzq3aoel/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/EICrecon/plugins/pid_angleres.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/eic/u/mposik/Detector1/EPIC/EICrecon/src/tests/pid_angleres/CMakeFiles/pid_angleres_plugin.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/EICrecon/tests/pid_angleres" TYPE FILE FILES "/eic/u/mposik/Detector1/EPIC/EICrecon/src/tests/pid_angleres/PidAngleRes_processor.h")
endif()

