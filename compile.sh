if [[ -n $1 ]]; then
    rm -rf build
fi
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=install && cmake --build build -j10; cmake --install build 
