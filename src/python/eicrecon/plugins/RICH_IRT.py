# SPDX-License-Identifier: LGPL-3.0-or-later
"""RICH-IRT wiring, ported by hand from the former C++ InitPlugin.

For each RICH known to IRT 2 that is present in the geometry, the track propagation surfaces
and the IRT configuration are computed in C++ (GetRichIrtConfigs in
src/python/bindings/pid/RichIrtConfig.h, from the geometry and the detector's JSON file),
through pid.rich_configs in a deferred section.
"""

from ..factories import pid, tracking

PLUGIN = "RICH-IRT"

# RICH detectors handled by IRT 2: ePIC dual-radiator RICH and proximity-focusing RICH
RICHES = ["DRICH", "PFRICH"]


def configure(app, plugin=PLUGIN):
    if not hasattr(pid, "IrtInterface"):
        # built without IRT 2 >= 2.2: nothing to wire
        return

    def wiring(graph):
        for rich in RICHES:
            cfgs = pid.rich_configs(graph.ctx.app, rich)
            if cfgs is None:
                continue
            tracks = f"{rich}Tracks"
            # charged particle track projections
            graph.add(tracking.TrackPropagation, tracks,
                      ["CentralCKFTracks", "CentralCKFActsTrackStates", "CentralCKFActsTracks"],
                      [tracks],
                      **cfgs["track"])
            # a unified IRT 2.1 algorithm
            graph.add(pid.IrtInterface, f"{rich}IrtInterface",
                      ["EventHeader", "MCParticles", "CentralCKFTracks", "CentralCKFTrackLinks",
                       tracks, f"{rich}Hits"],
                      [f"{rich}IrtRadiatorInfo", f"{rich}IrtParticles"],
                      **cfgs["irt"])

    with app.plugin(plugin):
        app.deferred(wiring)
