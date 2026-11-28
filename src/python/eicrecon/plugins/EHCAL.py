# SPDX-License-Identifier: LGPL-3.0-or-later
"""EHCAL wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "EHCAL"


def configure(app, plugin=PLUGIN):
    T = bindings("EHCAL")
    HcalEndcapN_capADC = 32768
    HcalEndcapN_dyRangeADC = 200 * u.MeV
    HcalEndcapN_pedMeanADC = 10
    HcalEndcapN_pedSigmaADC = 2
    HcalEndcapN_resolutionTDC = 10 * u.picosecond

    with app.plugin(plugin):
        app.add(T.CalorimeterHitDigi, 'HcalEndcapNRawHits',
                ['EventHeader', 'HcalEndcapNHits'],
                ['HcalEndcapNRawHits', 'HcalEndcapNRawHitLinks', 'HcalEndcapNRawHitAssociations'],
                capADC=HcalEndcapN_capADC,
                capTime=100,
                dyRangeADC=HcalEndcapN_dyRangeADC,
                pedMeanADC=HcalEndcapN_pedMeanADC,
                pedSigmaADC=HcalEndcapN_pedSigmaADC,
                resolutionTDC=HcalEndcapN_resolutionTDC,
                readout="HcalEndcapNHits")
        app.add(T.CalorimeterHitReco, 'HcalEndcapNRecHits',
                ['HcalEndcapNRawHits'],
                ['HcalEndcapNRecHits'],
                capADC=HcalEndcapN_capADC,
                dyRangeADC=HcalEndcapN_dyRangeADC,
                pedMeanADC=HcalEndcapN_pedMeanADC,
                pedSigmaADC=HcalEndcapN_pedSigmaADC,
                resolutionTDC=HcalEndcapN_resolutionTDC,
                thresholdValue=41.0,
                sampFrac="0.0095",
                readout="HcalEndcapNHits")
        app.add(T.CalorimeterHitsMerger, 'HcalEndcapNMergedHits',
                ['HcalEndcapNRecHits'],
                ['HcalEndcapNMergedHits'],
                readout="HcalEndcapNHits",
                fieldTransformations=["layer:4", "slice:0"])
        app.add(T.CalorimeterTruthClustering, 'HcalEndcapNTruthProtoClusters',
                ['HcalEndcapNMergedHits', 'HcalEndcapNRawHitLinks'],
                ['HcalEndcapNTruthProtoClusters'])
        app.add(T.CalorimeterIslandCluster, 'HcalEndcapNIslandProtoClusters',
                ['HcalEndcapNMergedHits'],
                ['HcalEndcapNIslandProtoClusters'],
                sectorDist=5.0 * u.cm,
                localDistXY=[15 * u.cm, 15 * u.cm],
                splitCluster=True,
                minClusterCenterEdep=30.0 * u.MeV,
                transverseEnergyProfileMetric="globalDistEtaPhi",
                transverseEnergyProfileScale=1.)
        app.add(T.CalorimeterClusterRecoCoG, 'HcalEndcapNTruthClustersWithoutShapes',
                ['HcalEndcapNTruthProtoClusters', 'HcalEndcapNRawHitLinks', 'HcalEndcapNRawHitAssociations'],
                ['HcalEndcapNTruthClustersWithoutShapes', 'HcalEndcapNTruthClusterLinksWithoutShapes', 'HcalEndcapNTruthClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.CalorimeterClusterShape, 'HcalEndcapNTruthClusters',
                ['HcalEndcapNTruthClustersWithoutShapes', 'HcalEndcapNTruthClusterLinksWithoutShapes'],
                ['HcalEndcapNTruthClusters', 'HcalEndcapNTruthClusterLinks', 'HcalEndcapNTruthClusterAssociations'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.CalorimeterClusterRecoCoG, 'HcalEndcapNClustersWithoutShapes',
                ['HcalEndcapNIslandProtoClusters', 'HcalEndcapNRawHitLinks', 'HcalEndcapNRawHitAssociations'],
                ['HcalEndcapNClustersWithoutShapes', 'HcalEndcapNClusterLinksWithoutShapes', 'HcalEndcapNClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.CalorimeterClusterShape, 'HcalEndcapNClusters',
                ['HcalEndcapNClustersWithoutShapes', 'HcalEndcapNClusterLinksWithoutShapes'],
                ['HcalEndcapNClusters', 'HcalEndcapNClusterLinks', 'HcalEndcapNClusterAssociations'],
                energyWeight="log",
                logWeightBase=6.2)
