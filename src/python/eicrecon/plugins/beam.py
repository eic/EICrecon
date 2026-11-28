# SPDX-License-Identifier: LGPL-3.0-or-later
"""beam wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import meta

PLUGIN = "beam"


def configure(app, plugin=PLUGIN):
    with app.plugin(plugin):
        app.add(meta.SubDivideCollection_MCParticle, 'BeamParticles',
                ['MCParticles'],
                ['MCBeamElectronsFromMCParticles', 'MCBeamProtonsFromMCParticles', 'MCBeamNeutronsFromMCParticles', 'MCScatteredElectrons', 'MCScatteredProtons', 'MCScatteredNeutrons'],
                function=meta.ValueSplit_GeneratorStatus_PDG([[4, 11], [4, 2212], [4, 2112], [1, 11], [1, 2212], [1, 2112]]))
        app.add(meta.CollectionCollector_MCParticle_true, 'MCBeamHadronsFromMCParticles',
                ['MCBeamProtonsFromMCParticles', 'MCBeamNeutronsFromMCParticles'],
                ['MCBeamHadronsFromMCParticles'])
        app.add(meta.Cloner_MCParticle, 'MCBeamElectrons',
                ['MCBeamElectronsFromMCParticles'],
                ['MCBeamElectrons'],
                cloneRelations=False)
        app.add(meta.Cloner_MCParticle, 'MCBeamHadrons',
                ['MCBeamHadronsFromMCParticles'],
                ['MCBeamHadrons'],
                cloneRelations=False)
