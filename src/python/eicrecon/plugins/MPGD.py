# SPDX-License-Identifier: LGPL-3.0-or-later
"""MPGD wiring, ported by hand from the former C++ InitPlugin.

The barrel MPGDs are digitized either as pixels (SiliconTrackerDigi) or as 2D strips
(MPGDTrackerDigi), depending on the geometry (constants <name>_2DStrip) or on the parameter
MPGD:SiFactoryPattern (hexadecimal: bit 0 InnerMPGDBarrel, bit 1 MPGDOuterBarrel; a set bit
selects pixel digitization). That choice is a deferred section; the endcaps are static.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from ..units import dd4hep as u
from ..units import edm4eic as eu
from . import bindings

PLUGIN = "MPGD"

MPGD_NAMES = ["InnerMPGDBarrel", "MPGDOuterBarrel"]


def configure(app, plugin=PLUGIN):
    T = bindings("MPGD")
    mpgdTimeResolution = 20 * eu.ns

    def barrel(graph):
        pattern = 0x3  # full-scale SiliconTrackerDigi
        for i, name in enumerate(MPGD_NAMES):
            if graph.constant_int(name + "_2DStrip") == 1:
                pattern &= ~(1 << i)
        pattern_str = graph.parameter(
            "MPGD:SiFactoryPattern", "",
            'Hexadecimal Pattern of MPGDs digitized via "SiliconTrackerDigi"')
        if pattern_str:
            try:
                pattern = int(pattern_str, 16)
            except ValueError as e:
                raise ValueError(f'Option "MPGD:SiFactoryPattern": Error ("{e}") parsing input '
                                 f"string: '{pattern_str}'.") from None

        def strip_resolution(phi_z):
            name = ["MMumResolutionPhi", "MMumResolutionZ"][phi_z]
            value = graph.constant_double(name)
            return 150 * u.um if value is None else value

        def strip_numbers(readout, names):
            numbers = []
            for name in names:
                n = graph.constant_int(name)
                if n is None:
                    raise ValueError(f'MPGD "{readout}": Error retrieving #channels from XML: '
                                     f'no "{name}" constant found.')
                numbers.append(n)
            return numbers

        # "MPGDBarrel" (CyMBaL)
        if pattern & 0x1:
            graph.add(T.SiliconTrackerDigi, "MPGDBarrelRawHits",
                      ["EventHeader", "MPGDBarrelHits"],
                      ["MPGDBarrelRawHits", "MPGDBarrelRawHitLinks",
                       "MPGDBarrelRawHitAssociations"],
                      threshold=100 * u.eV,
                      timeResolution=mpgdTimeResolution)
            graph.add(T.TrackerHitReconstruction, "MPGDBarrelRecHits",
                      ["MPGDBarrelRawHits"],
                      ["MPGDBarrelRecHits"],
                      timeResolution=mpgdTimeResolution)
        else:
            resolutions = [strip_resolution(0), strip_resolution(1)]
            graph.add(T.MPGDTrackerDigi, "MPGDBarrelRawHits",
                      ["EventHeader", "MPGDBarrelHits"],
                      ["MPGDBarrelRawHits", "MPGDBarrelRawHitLinks",
                       "MPGDBarrelRawHitAssociations"],
                      readout="MPGDBarrelHits",
                      threshold=100 * u.eV,
                      timeResolution=mpgdTimeResolution,
                      gain=10000,
                      stripNumbers=strip_numbers("MPGDBarrelHits",
                                                 ["MMnStripsPhi", "MMnStripsZ"]),
                      stripResolutions=resolutions,
                      hasDeadZone=True)
            graph.add(T.MPGDHitReconstruction, "MPGDBarrelRecHits",
                      ["MPGDBarrelRawHits"],
                      ["MPGDBarrelRecHits"],
                      readout="MPGDBarrelHits",
                      timeResolution=mpgdTimeResolution,
                      stripResolutions=resolutions)

        # "OuterMPGDBarrel"
        if pattern & 0x2:
            graph.add(T.SiliconTrackerDigi, "OuterMPGDBarrelRawHits",
                      ["EventHeader", "OuterMPGDBarrelHits"],
                      ["OuterMPGDBarrelRawHits", "OuterMPGDBarrelRawHitLinks",
                       "OuterMPGDBarrelRawHitAssociations"],
                      threshold=100 * u.eV,
                      timeResolution=mpgdTimeResolution)
            graph.add(T.TrackerHitReconstruction, "OuterMPGDBarrelRecHits",
                      ["OuterMPGDBarrelRawHits"],
                      ["OuterMPGDBarrelRecHits"],
                      timeResolution=mpgdTimeResolution)
        else:
            n = strip_numbers("OuterMPGDBarrelHits", ["MPGDOuterBarrelnStrips"])[0]
            graph.add(T.MPGDTrackerDigi, "OuterMPGDBarrelRawHits",
                      ["EventHeader", "OuterMPGDBarrelHits"],
                      ["OuterMPGDBarrelRawHits", "OuterMPGDBarrelRawHitLinks",
                       "OuterMPGDBarrelRawHitAssociations"],
                      readout="OuterMPGDBarrelHits",
                      threshold=100 * u.eV,
                      timeResolution=mpgdTimeResolution,
                      gain=10000,
                      stripNumbers=[n, n],
                      stripResolutions=[150 * u.um, 150 * u.um],
                      hasDeadZone=False)
            graph.add(T.MPGDHitReconstruction, "OuterMPGDBarrelRecHits",
                      ["OuterMPGDBarrelRawHits"],
                      ["OuterMPGDBarrelRecHits"],
                      readout="OuterMPGDBarrelHits",
                      timeResolution=mpgdTimeResolution,
                      stripResolutions=[150 * u.um, 150 * u.um])

    with app.plugin(plugin):
        app.deferred(barrel)

        # "BackwardMPGDEndcap" and "ForwardMPGDEndcap"
        for name in ["BackwardMPGDEndcap", "ForwardMPGDEndcap"]:
            app.add(T.SiliconTrackerDigi, f"{name}RawHits",
                    ["EventHeader", f"{name}Hits"],
                    [f"{name}RawHits", f"{name}RawHitLinks", f"{name}RawHitAssociations"],
                    threshold=100 * u.eV,
                    timeResolution=mpgdTimeResolution)
            app.add(T.TrackerHitReconstruction, f"{name}RecHits",
                    [f"{name}RawHits"],
                    [f"{name}RecHits"],
                    timeResolution=mpgdTimeResolution)
