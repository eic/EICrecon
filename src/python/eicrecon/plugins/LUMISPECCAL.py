# SPDX-License-Identifier: LGPL-3.0-or-later
"""LUMISPECCAL wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import calorimetry

PLUGIN = "LUMISPECCAL"


def configure(app, plugin=PLUGIN):
    with app.plugin(plugin):
        app.add(calorimetry.CalorimeterHitDigi, 'EcalLumiSpecRawHits',
                ['EventHeader', 'EcalLumiSpecHits'],
                ['EcalLumiSpecRawHits', 'EcalLumiSpecRawHitLinks', 'EcalLumiSpecRawHitAssociations'],
                eRes=[0.0 * sqrt(u.GeV), 0.02, 0.0 * u.GeV],
                capADC=16384,
                dyRangeADC=20 * u.GeV,
                pedMeanADC=100,
                pedSigmaADC=1,
                resolutionTDC=10 * u.picosecond,
                readout="EcalLumiSpecHits")
        app.add(calorimetry.CalorimeterHitReco, 'EcalLumiSpecRecHits',
                ['EcalLumiSpecRawHits'],
                ['EcalLumiSpecRecHits'],
                capADC=16384,
                dyRangeADC=20. * u.GeV,
                pedMeanADC=100,
                pedSigmaADC=1,
                resolutionTDC=10 * u.picosecond,
                thresholdValue=2.0,
                readout="EcalLumiSpecHits")
        app.add(calorimetry.CalorimeterTruthClustering, 'EcalLumiSpecTruthProtoClusters',
                ['EcalLumiSpecRecHits', 'EcalLumiSpecRawHitLinks'],
                ['EcalLumiSpecTruthProtoClusters'])
        app.add(calorimetry.CalorimeterIslandCluster, 'EcalLumiSpecIslandProtoClusters',
                ['EcalLumiSpecRecHits'],
                ['EcalLumiSpecIslandProtoClusters'],
                adjacencyMatrix='(sector_1 == sector_2) && ((abs(floor(module_1 / 10) - floor(module_2 / 10)) + abs(fmod(module_1, 10) - fmod(module_2, 10))) == 1)',
                readout="EcalLumiSpecHits",
                splitCluster=True,
                minClusterHitEdep=1.0 * u.MeV,
                minClusterCenterEdep=30.0 * u.MeV,
                transverseEnergyProfileMetric="localDistXY",
                transverseEnergyProfileScale=10. * u.mm)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'EcalLumiSpecClustersWithoutShapes',
                ['EcalLumiSpecIslandProtoClusters', 'EcalLumiSpecRawHitLinks', 'EcalLumiSpecRawHitAssociations'],
                ['EcalLumiSpecClustersWithoutShapes', 'EcalLumiSpecClusterLinksWithoutShapes', 'EcalLumiSpecClusterAssociationsWithoutShapes'],
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterShape, 'EcalLumiSpecClusters',
                ['EcalLumiSpecClustersWithoutShapes', 'EcalLumiSpecClusterLinksWithoutShapes'],
                ['EcalLumiSpecClusters', 'EcalLumiSpecClusterLinks', 'EcalLumiSpecClusterAssociations'],
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'EcalLumiSpecTruthClustersWithoutShapes',
                ['EcalLumiSpecTruthProtoClusters', 'EcalLumiSpecRawHitLinks', 'EcalLumiSpecRawHitAssociations'],
                ['EcalLumiSpecTruthClustersWithoutShapes', 'EcalLumiSpecTruthClusterLinksWithoutShapes', 'EcalLumiSpecTruthClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=4.6)
        app.add(calorimetry.CalorimeterClusterShape, 'EcalLumiSpecTruthClusters',
                ['EcalLumiSpecTruthClustersWithoutShapes', 'EcalLumiSpecTruthClusterLinksWithoutShapes'],
                ['EcalLumiSpecTruthClusters', 'EcalLumiSpecTruthClusterLinks', 'EcalLumiSpecTruthClusterAssociations'],
                energyWeight="log",
                logWeightBase=4.6)
