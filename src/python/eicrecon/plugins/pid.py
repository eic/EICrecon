# SPDX-License-Identifier: LGPL-3.0-or-later
"""pid wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "pid"


def configure(app, plugin=PLUGIN):
    T = bindings("pid")
    with app.plugin(plugin):
        app.add(T.MatchToRICHPID, 'ChargedParticlesWithAssociations',
                ['ReconstructedChargedWithoutPIDParticles', 'ReconstructedChargedWithoutPIDParticleLinks', 'DRICHMergedIrtCherenkovParticleID'],
                ['ReconstructedChargedRealPIDParticles', 'ReconstructedChargedRealPIDParticleLinks', 'ReconstructedChargedRealPIDParticleAssociations', 'ReconstructedChargedRealPIDParticleIDs'])
