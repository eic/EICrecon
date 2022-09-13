# EDPM installation


**edpm** stands for **E**IC **d**evelopment  **p**acket ~~**m**anager~~ helper

**The goal** of edpm is to provide esier experience of building EIC simulation and reconstruction
framework and supporting packages on a user machine with development reasons. It is a user friendly CLI
alternative to build instructions and build scritps that compile EIC chain of packages with right flags.
Something that users can simply do `pip install edpm; edpm install package`.

Very minimal start:

Install edpm itself:

```sh
python3 -m pip install --upgrade --user ejpm
```

Install eicrecon

```sh
# 1. System prerequesties
edpm req ubuntu22           # get list of required OS packets. Use `centos8` on RHEL8  
sudo apt install ...        # install watever 'edpm req' tells you

# 2. Where to install
edpm --top-dir=<where-to>   # Directory where packets will be installed

# 3. Install
edpm install eicrecon       # install 'Geant 4 EIC' and dependencies (like vgm, hepmc)

# 4.  Source environment
source ~/.local/share/edpm/env.sh  # Use *.csh file for tcsh

# Futher help. EDPM has self descriptive help just run
edpm       # current status and installed packaged
edpm --help
```

You have ROOT and Geant4 and don’t want EJPM to build them?(Use your installations of ROOT and Geant4)

```sh
# Before running 'ejpm install g4e'
edpm set root `$ROOTSYS`    # Path to ROOT installation
edpm set geant4 <path>       # Path to Geant4 installation
```

**How to switch of branch of some packet or build threads?**

```sh
# edpm has per packet build config and global config
# all packets have 'branch' config pointing to current tag/branch
edpm config jana2 branch=master   # switch jana2 to install jana2-master branch
edpm config jana2                 # see configs for jana2
edpm config global                # see global configs, such as build threads
```

(!)EDPM is not a real packet manager and don't track dependency chain. In
current EDPM version if you re-insstall some packet to a different version,
you have to reinstall dependent packages yourself. to reinstall some package
(e.g. after edpm update or config change)

```
edpm rm package && edpm install package    # reinstall package

# sometimes on wants to just rebuild a package, without redownloading 
edpm install --forca package   # rebuild package even if it exists
```

**Control environment**

Packets in edpm are connected through environment, LD_LIBRARY_PATH etc.
Every time configuration is changed (something installed or deleted) or
***edpm env*** command is called, edpm creates
2 environment files with the current environment:

```bash
~/.local/share/edpm/env.sh    # bash
~/.local/share/edpm/env.csh    # bash
```

Examples:

1. Dynamically source output of ```edpm env``` command (recommended)

    ```bash        
    source <(edpm env)                # works on bash
    ```
2. Save output of ```edpm env``` command to a file (can be useful)

    ```bash
     edpm env sh  > your-file.sh       # get environment for bash or compatible shells
     edpm env csh > your-file.csh      # get environment for CSH/TCSH
    ```

**Use CLion and others IDE with EDPM**

EDPM is designed to be used with IDEs. In order to use with VSCode or IntelliJ types of IDE:

1. Exit IDE completely (as they use a single produce even when several instances are open)
2. Source EDPM environment
3. Run `clion` or `code` from the terminal where you sources the environment

For more complex cases such as remote or docker development edpm can generate cmake
flags. See `edpm env --help`

More documentation and source:

[https://github.com/eic/edpm](https://github.com/eic/edpm)