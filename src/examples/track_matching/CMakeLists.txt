cmake_minimum_required(VERSION 3.16)

project(combined_benchmarks VERSION 0.0.0)

# Make C++17 a default
if(NOT "${CMAKE_CXX_STANDARD}")
    set(CMAKE_CXX_STANDARD 17)
endif()

# Enable -fPIC for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Install to the top directory by default
if( ${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT} )
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR} CACHE PATH "Install in top directory by default" FORCE)
endif()

# Add CMake additional functionality:
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)   # Find Find<Modules>.cmake

# include logging by default
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(EDM4EIC REQUIRED)
find_package(podio REQUIRED)
find_package(DD4hep REQUIRED)
set(fmt_INCLUDE_DIR ${fmt_DIR}/../../../include)

# include ROOT by default
find_package(ROOT REQUIRED)

set(COMMON_TARGET_INCLUDES
        SYSTEM PUBLIC
        ${podio_INCLUDE_DIR}
        ${EDM4HEP_INCLUDE_DIR}
        ${EDM4EIC_INCLUDE_DIR}
        ${ROOT_INCLUDE_DIRS}
        ${fmt_INCLUDE_DIR}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/executables
        ${CMAKE_SOURCE_DIR}/common_bench/include
        )

set(COMMON_TARGET_LIBRARIES spdlog::spdlog EDM4HEP::edm4hep DD4hep::DDCore EDM4EIC::edm4eic ROOT::Core ${ROOT_LIBRARIES})

add_executable(reco_particles_track_matching
        reco_particles_track_matching.cc
        )
target_include_directories(reco_particles_track_matching ${COMMON_TARGET_INCLUDES})
target_link_libraries(reco_particles_track_matching ${COMMON_TARGET_LIBRARIES})
install(TARGETS reco_particles_track_matching)
