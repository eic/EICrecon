#!/bin/bash
#source /scratch2/DD4hep/build/thisdd4hep.sh
#cmake -B build -S . -DCMAKE_INSTALL_PREFIX=$ATHENA_PREFIX -DCMAKE_CXX_STANDARD=17

#cmake -B build -S .  #-DCMAKE_INSTALL_PREFIX=/home/simong/EIC/epic/install
cmake --build build -j8
cmake --install build 
