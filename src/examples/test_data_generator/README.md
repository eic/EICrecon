# Data simulation

Examples of how to generate data, that might be used for testing

## Particle gun

- [dd4hep_pgun_simulation.py](dd4hep_pgun_simulation.py) - Helper class and funcs to setup a simulation in python
- [dd4hep_pgun_simulate_test_set.py](dd4hep_pgun_simulate_test_set.py) - simulates a set of test data
- [simple_pgun_simulate.py](simple_pgun_simulate.py) - simple example of simulation of one particle gun data
- [simple_pythia_sim.py](simple_pythia_sim.py) - simple example to run pythia 8 file through dd4hep

 
## S3 upload

Download AWS cli [from here](https://aws.amazon.com/cli/)


```bash
aws configure

# To setup public and private access key to user
```

```bash
aws s3api put-object --bucket eicaidata --key dir-1/my_images.tar.bz2 --body my_images.tar.bz2
```

```
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_arches_e0.01-30GeV_alldir_1prt_10000evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_arches_e0.01-30GeV_alldir_1prt_1000evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_arches_e0.01-30GeV_alldir_1prt_100evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_arches_e0.01-30GeV_alldir_1prt_10evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_arches_e0.01-30GeV_alldir_5prt_100evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_arches_e0.01-30GeV_alldir_5prt_2000evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_brycecanyon_e0.01-30GeV_alldir_1prt_10000evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_brycecanyon_e0.01-30GeV_alldir_1prt_1000evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_brycecanyon_e0.01-30GeV_alldir_1prt_100evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_brycecanyon_e0.01-30GeV_alldir_1prt_10evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_brycecanyon_e0.01-30GeV_alldir_5prt_100evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_e-_epic_brycecanyon_e0.01-30GeV_alldir_5prt_2000evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_pi+_epic_arches_e0.01-30GeV_alldir_1prt_10000evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_pi+_epic_arches_e0.01-30GeV_alldir_1prt_1000evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_pi+_epic_arches_e0.01-30GeV_alldir_1prt_100evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_pi+_epic_arches_e0.01-30GeV_alldir_1prt_10evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_pi+_epic_brycecanyon_e0.01-30GeV_alldir_1prt_10000evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_pi+_epic_brycecanyon_e0.01-30GeV_alldir_1prt_1000evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_pi+_epic_brycecanyon_e0.01-30GeV_alldir_1prt_100evt.edm4hep.root
https://eicaidata.s3.amazonaws.com/2022-10-27_pgun_pi+_epic_brycecanyon_e0.01-30GeV_alldir_1prt_10evt.edm4hep.root
```