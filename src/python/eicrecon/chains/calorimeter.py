# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""The standard calorimeter chain, as a reusable building block.

digitization -> hit reconstruction -> {island, truth} proto-clusters
             -> center-of-gravity clusters -> cluster shapes

Collection names follow the convention of the C++ plugins (e.g. B0ECAL.cc), so ported
detectors produce identically named collections.
"""

from ..factories.calorimetry import (
    CalorimeterClusterRecoCoG,
    CalorimeterClusterShape,
    CalorimeterHitDigi,
    CalorimeterHitReco,
    CalorimeterIslandCluster,
    CalorimeterTruthClustering,
)


def calorimeter_chain(app, name, *, sim_hits=None, digi, reco, island, cog, shape,
                      truth_cog=None, truth_shape=None):
    """Add the standard chain for calorimeter `name` (e.g. 'B0ECal') to `app`.

    Config dicts are passed unchanged to the factories; unknown fields raise GraphError.
    Truth clustering is added when `truth_cog` and `truth_shape` are given.
    """
    n = name
    sim_hits = sim_hits or f"{n}Hits"

    app.add(CalorimeterHitDigi, f"{n}RawHits",
            ["EventHeader", sim_hits],
            [f"{n}RawHits", f"{n}RawHitLinks", f"{n}RawHitAssociations"],
            **digi)
    app.add(CalorimeterHitReco, f"{n}RecHits",
            [f"{n}RawHits"],
            [f"{n}RecHits"],
            **reco)
    if truth_cog is not None:
        app.add(CalorimeterTruthClustering, f"{n}TruthProtoClusters",
                [f"{n}RecHits", f"{n}RawHitLinks"],
                [f"{n}TruthProtoClusters"])
    app.add(CalorimeterIslandCluster, f"{n}IslandProtoClusters",
            [f"{n}RecHits"],
            [f"{n}IslandProtoClusters"],
            **island)
    app.add(CalorimeterClusterRecoCoG, f"{n}ClustersWithoutShapes",
            [f"{n}IslandProtoClusters", f"{n}RawHitLinks", f"{n}RawHitAssociations"],
            [f"{n}ClustersWithoutShapes", f"{n}ClusterLinksWithoutShapes",
             f"{n}ClusterAssociationsWithoutShapes"],
            **cog)
    app.add(CalorimeterClusterShape, f"{n}Clusters",
            [f"{n}ClustersWithoutShapes", f"{n}ClusterLinksWithoutShapes"],
            [f"{n}Clusters", f"{n}ClusterLinks", f"{n}ClusterAssociations"],
            **shape)
    if truth_cog is not None:
        app.add(CalorimeterClusterRecoCoG, f"{n}TruthClustersWithoutShapes",
                [f"{n}TruthProtoClusters", f"{n}RawHitLinks", f"{n}RawHitAssociations"],
                [f"{n}TruthClustersWithoutShapes", f"{n}TruthClusterLinksWithoutShapes",
                 f"{n}TruthClusterAssociationsWithoutShapes"],
                **truth_cog)
        app.add(CalorimeterClusterShape, f"{n}TruthClusters",
                [f"{n}TruthClustersWithoutShapes", f"{n}TruthClusterLinksWithoutShapes"],
                [f"{n}TruthClusters", f"{n}TruthClusterLinks", f"{n}TruthClusterAssociations"],
                **(truth_shape or {}))
