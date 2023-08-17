import sys

import numpy as np
from pyHepMC3 import HepMC3

# generates 2 electrons from different vertices relevant for the lumi Pair Spectrometer
outfilename = sys.argv[1]
mass = 0.511e-3
pz = -10 # GeV
E = np.sqrt(mass**2 + pz**2)

y1 = 110 # y position of gun 1 in mm
y2 = 180 # y position of gun 2 in mm
z = -64000 # z position close to entrance of lumi Pair Spectrometer CALs in mm

writer = HepMC3.WriterAscii( outfilename )

for ix in range(100):

    particle_in1 = HepMC3.GenParticle(HepMC3.FourVector(0, 0, pz, E))
    particle_in1.set_pdg_id(11)
    particle_in1.set_status(3)
    particle_out1 = HepMC3.GenParticle(HepMC3.FourVector(0, 0, pz, E))
    particle_out1.set_pdg_id(11)
    particle_out1.set_status(1)

    particle_in2 = HepMC3.GenParticle(HepMC3.FourVector(0, 0, pz, E))
    particle_in2.set_pdg_id(11)
    particle_in2.set_status(3)
    particle_out2 = HepMC3.GenParticle(HepMC3.FourVector(0, 0, pz, E))
    particle_out2.set_pdg_id(11)
    particle_out2.set_status(1)

    vertex1 = HepMC3.GenVertex(HepMC3.FourVector(0., y1, z, 0.))
    vertex1.add_particle_in( particle_in1 )
    vertex1.add_particle_out( particle_out1 )

    vertex2 = HepMC3.GenVertex(HepMC3.FourVector(0., y2, z, 0.))
    vertex2.add_particle_in( particle_in2 )
    vertex2.add_particle_out( particle_out2 )

    event = HepMC3.GenEvent( HepMC3.Units.GEV, HepMC3.Units.MM )
    event.add_vertex( vertex1 )
    event.add_vertex( vertex2 )

    writer.write_event( event )
