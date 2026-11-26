# SPDX-License-Identifier: LGPL-3.0-or-later
"""BHCAL wiring, converted from src/detectors/BHCAL/BHCAL.cc by src/python/tools/generate_wiring.py.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "BHCAL"


def configure(app, plugin=PLUGIN):
    T = bindings("BHCAL")
    HcalBarrel_capADC = 65536
    HcalBarrel_pedMeanADC = 300
    HcalBarrel_pedSigmaADC = 2
    HcalBarrel_resolutionTDC = 1 * u.picosecond

    with app.plugin(plugin):
        app.add(T.CalorimeterHitDigi, 'HcalBarrelRawHits',
                ['EventHeader', 'HcalBarrelHits'],
                ['HcalBarrelRawHits', 'HcalBarrelRawHitLinks', 'HcalBarrelRawHitAssociations'],
                threshold=0.0,
                capADC=HcalBarrel_capADC,
                capTime=100,
                pedMeanADC=HcalBarrel_pedMeanADC,
                pedSigmaADC=HcalBarrel_pedSigmaADC,
                resolutionTDC=HcalBarrel_resolutionTDC,
                readout="HcalBarrelHits")
        app.add(T.CalorimeterHitReco, 'HcalBarrelRecHits',
                ['HcalBarrelRawHits'],
                ['HcalBarrelRecHits'],
                capADC=HcalBarrel_capADC,
                pedMeanADC=HcalBarrel_pedMeanADC,
                pedSigmaADC=HcalBarrel_pedSigmaADC,
                resolutionTDC=HcalBarrel_resolutionTDC,
                thresholdValue=33,
                sampFrac="0.033",
                readout="HcalBarrelHits")
        app.add(T.CalorimeterHitsMerger, 'HcalBarrelMergedHits',
                ['HcalBarrelRecHits'],
                ['HcalBarrelMergedHits'],
                readout="HcalBarrelHits",
                fieldTransformations=["phi:phi"])
        app.add(T.CalorimeterTruthClustering, 'HcalBarrelTruthProtoClusters',
                ['HcalBarrelRecHits', 'HcalBarrelRawHitLinks'],
                ['HcalBarrelTruthProtoClusters'])
        app.add(T.CalorimeterIslandCluster, 'HcalBarrelIslandProtoClusters',
                ['HcalBarrelRecHits'],
                ['HcalBarrelIslandProtoClusters'],
                adjacencyMatrix='(  ( (abs(eta_1 - eta_2) == 1) && (abs(phi_1 - phi_2) == 0) ) ||  ( (abs(eta_1 - eta_2) == 0) && (abs(phi_1 - phi_2) == 1) ) ||  ( (abs(eta_1 - eta_2) == 0) && (abs(phi_1 - phi_2) == (320 - 1)) )) == 1',
                readout="HcalBarrelHits",
                sectorDist=5.0 * u.cm,
                minClusterHitEdep=5.0 * u.MeV,
                minClusterCenterEdep=30.0 * u.MeV,
                transverseEnergyProfileMetric="globalDistEtaPhi",
                transverseEnergyProfileScale=1.)
        app.add(T.CalorimeterClusterRecoCoG, 'HcalBarrelClustersWithoutShapes',
                ['HcalBarrelIslandProtoClusters', 'HcalBarrelRawHitLinks', 'HcalBarrelRawHitAssociations'],
                ['HcalBarrelClustersWithoutShapes', 'HcalBarrelClusterLinksWithoutShapes', 'HcalBarrelClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.CalorimeterClusterShape, 'HcalBarrelClusters',
                ['HcalBarrelClustersWithoutShapes', 'HcalBarrelClusterLinksWithoutShapes'],
                ['HcalBarrelClusters', 'HcalBarrelClusterLinks', 'HcalBarrelClusterAssociations'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.CalorimeterClusterRecoCoG, 'HcalBarrelTruthClustersWithoutShapes',
                ['HcalBarrelTruthProtoClusters', 'HcalBarrelRawHitLinks', 'HcalBarrelRawHitAssociations'],
                ['HcalBarrelTruthClustersWithoutShapes', 'HcalBarrelTruthClusterLinksWithoutShapes', 'HcalBarrelTruthClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.CalorimeterClusterShape, 'HcalBarrelTruthClusters',
                ['HcalBarrelTruthClustersWithoutShapes', 'HcalBarrelTruthClusterLinksWithoutShapes'],
                ['HcalBarrelTruthClusters', 'HcalBarrelTruthClusterLinks', 'HcalBarrelTruthClusterAssociations'],
                energyWeight="log",
                logWeightBase=6.2)
