# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Minimal graph: only the B0 ECal chain, with only the C++ plugins it needs.

    eicrecon --graph src/python/examples/b0ecal_only.py \
        -Ppodio:output_file=b0ecal.edm4eic.root sim.edm4hep.root

Of the C++ algorithm libraries, only libalgorithms_calorimetry.so (pulled in by
eicrecon.factories.calorimetry) is loaded; of the C++ plugins, only services and I/O.
"""

from eicrecon.plugins import B0ECAL

# "acts" is needed although no tracking runs: AlgorithmsInit_service acquires ACTSGeo_service.
# Undeclared plugin dependencies like this one are an open problem (see the design doc).
SERVICES = ["log", "dd4hep", "evaluator", "acts", "algorithms_init", "podio"]

# Output: the reconstructed collections and every collection their relations point to
# (simulated hits, their contributions and the MC particles), so that no relation dangles.
OUTPUTS = ["EventHeader", "MCParticles", "B0ECalHits", "B0ECalHitsContributions"] + [
    f"B0ECal{c}"
    for c in [
        "RawHits", "RawHitLinks", "RawHitAssociations", "RecHits",
        "TruthProtoClusters", "IslandProtoClusters",
        "Clusters", "ClusterLinks", "ClusterAssociations",
        "TruthClusters", "TruthClusterLinks", "TruthClusterAssociations",
    ]
]


def configure(app):
    for name in SERVICES:
        app.add_plugin(name)
    B0ECAL.configure(app)
    app.params.setdefault("podio:output_collections", ",".join(OUTPUTS))
