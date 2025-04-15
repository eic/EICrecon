
```
#
# Define installation area; start the container; define environment variables
#
export SANDBOX=/DATA00/ayk/ePIC

cd $SANDBOX

# curl --location https://get.epic-eic.org | bash

./eic-shell

. environ.sh
```

```
#
# Install EDM4eic, IRT 2.0, epic & EICrecon
#
cmake -S EDM4eic -B EDM4eic/build -DBUILD_DATA_MODEL=ON -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -Wno-dev
cmake --build EDM4eic/build -j8
cmake --install EDM4eic/build

# '-b pfrich': IRT 2.0;
git clone -b pfrich https://github.com/eic/irt.git
#cmake -S irt -B irt/build -DCMAKE_BUILD_TYPE=Debug -DDELPHES=OFF -DEVALUATION=OFF -DIRT_ROOT_IO=ON -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX
cmake -S irt -B irt/build -DCMAKE_BUILD_TYPE=Debug -DDELPHES=OFF -DEVALUATION=OFF -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -Wno-dev
cmake --build irt/build -j8
cmake --install irt/build

cmake -S epic -B epic/build -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -Wno-dev
cmake --build epic/build -j8
cmake --install epic/build

cmake -S EICrecon -B EICrecon/build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_FIND_DEBUG_MODE=OFF -DEICRECON_VERBOSE_CMAKE=ON -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -Wno-dev
cmake --build EICrecon/build -j8
cmake --install EICrecon/build
```

```
export LD_LIBRARY_PATH=${SANDBOX}/prefix/lib:${LD_LIBRARY_PATH}
export DETECTOR_PATH=${SANDBOX}/prefix/share/epic
export JANA_PLUGIN_PATH=${SANDBOX}/prefix/lib/EICrecon/plugins:$JANA_PLUGIN_PATH
```

