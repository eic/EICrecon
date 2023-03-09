# Use singularity


## Install singularity


Eic image require singularity \> 3.0

The [oficial installation instructions](https://sylabs.io/guides/3.0/user-guide/installation.html) have many steps.

If you use debian or ubuntu (mint, whatever compatible), there is a
[debian repo with 3.11 version at ftp.fi.debian.org](http://ftp.fi.debian.org/debian/pool/main/s/singularity-container/),
You will have to install dependencies, and need `sudo apt install containernetworking-plugins` prior the installation. 

Please don\'t install NeuroDebian repo (first in google search) as its singularity v2.6
and eic image require singularity \> 3.0.

## Install work environment

The below command automatically creates the right working environment
for detector development and running the reconstruction. It checks if
there are CVMFS images available (which is true for JLab and BNL farms)
and links them or downloads images (which is a scenario for users
laptops). It also creates eic\_shell with the right environment setup,
prepares the current dir to work with detector or etc.

```bash
curl https://eicweb.phy.anl.gov/containers/eic_container/-/raw/master/install.sh | bash
```

**install.sh** checks if it is run on BNL or JLab farms, so existing
CVMFS images are used and installation is almost instant. On local
systems singularity images will be downloaded.

It might be handy to copy install.sh locally and control where
singularity images are being copied, disable CVMFS behaviour, and other
parameters:

```bash
curl https://eicweb.phy.anl.gov/containers/eic_container/-/raw/master/install.sh
chmod +x install.sh
./install.sh --help
```

| Flag            | Description                                                        |
|-----------------|--------------------------------------------------------------------|
| -p,\--prefix    | Working directory to deploy the environment (D: /home/romanov/anl) |
| -t,\--tmpdir    | Change tmp directory (D: /tmp)                                     |
| -n,\--no-cvmfs  | Disable check for local CVMFS (D: enabled)                         |
| -c,\--container | Container version (D: jug\_xl)                                     |
| -v,\--version   | Version to install (D: nightly)                                    |
| -h,\--help      | Print this message                                                 |

Example of controlling the container version and image location:

```bash
# installs testing variant and stores image at /mnt/work/images
# (!) one has to create <prefix>/local/lib for images
mkdir -p /mnt/work/images/local/lib/
./install.sh -v testing -p /mnt/work/images
```

## Select detector

After the installation you should have an executable script named
**eic-shell** which basically just runs singularity setting the proper
environment (more information about the script is below)

### Precompiled detector

The jug\_xl container comes with precompiled detecor repository. It
could be used out of the box for simulations or even changing detector
parameters that doesn\'t require recompilation.

The precompiled detector is installed in **/opt/detector** directory.
And can be used like this:

```bash
# Setup the proper detector environemnt
source /opt/detector/setup.sh

# Run particle gun simulation
ddsim -N2 --compactFile=$DETECTOR_PATH/epic.xml --random.seed 1 --enableGun --gun.energy 2*GeV --gun.thetaMin 0*deg --gun.thetaMax 90*deg --gun.distribution uniform --outputFile ~/test.root
```


## Advanced information


### CVMFS

For farms like at BNL or JLab the images are automatically replicated to
CVMS:

```bash
/cvmfs/singularity.opensciencegrid.org/eicweb/jug_xl*

# example to run
singularity run /cvmfs/singularity.opensciencegrid.org/eicweb/jug_xl:nightly
```

### eic-shell explained

There are actually two eic-shell scripts. One is created by the install
scripts and the other lives in the container.

The one outside the container just sets ATHENA\_PREFIX and runs
singularity like:

```bash
# $PREFIX here is where you installed everything (by default where install.sh executed)
export EIC_SHELL_PREFIX=...
export SINGULARITY_BINDPATH=/home
singularity exec $PREFIX/local/lib/jug_xl-nightly.sif eic-shell $@
```

The **eic-shell** inside the container loads the proper environemnt and
SHELL look correctly

```bash
## Properly setup environment
. /etc/eic-env.sh

# What eic-env does in the end is
export LD_LIBRARY_PATH=$EPIC_PREFIX/lib:$LD_LIBRARY_PATH
export PATH=$EPIC_PREFIX/bin:$PATH
```
