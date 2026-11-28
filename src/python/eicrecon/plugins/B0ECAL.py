# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""B0 electromagnetic calorimeter: Python port of the former C++ InitPlugin.

Values use dd4hep units, like the C++ wiring, so the config structs are bit-identical.
"""

from math import sqrt

from ..factories import calorimetry
from ..units import dd4hep as u

PLUGIN = "B0ECAL"


def configure(app, plugin=PLUGIN):
    with app.plugin(plugin):
        app.add(calorimetry.CalorimeterHitDigi, "B0ECalRawHits",
                ["EventHeader", "B0ECalHits"],
                ["B0ECalRawHits", "B0ECalRawHitLinks", "B0ECalRawHitAssociations"],
                # Stochastic term from the light yield in PbWO4, N_photons = 145.75 / GeV / mm,
                # for 6x6 mm2 sensors with PDE=0.18 (a=1/sqrt(145.75*36*0.18))
                eRes=[0.0326 * sqrt(u.GeV), 0.00, 0.0 * u.GeV],
                tRes=0.0 * u.ns,
                threshold=5.0 * u.MeV,
                capADC=16384,
                dyRangeADC=170 * u.GeV,
                pedMeanADC=100,
                pedSigmaADC=1,
                resolutionTDC=1e-11,
                corrMeanScale="1.0",
                readout="B0ECalHits")
        app.add(calorimetry.CalorimeterHitReco, "B0ECalRecHits",
                ["B0ECalRawHits"],
                ["B0ECalRecHits"],
                capADC=16384,
                dyRangeADC=170.0 * u.GeV,
                pedMeanADC=100,
                pedSigmaADC=1,
                resolutionTDC=1e-11,
                thresholdFactor=0.0,
                thresholdValue=1.0,  # threshold of 10 photons = 10 MeV = 1 ADC
                sampFrac="0.998",
                readout="B0ECalHits")
        app.add(calorimetry.CalorimeterTruthClustering, "B0ECalTruthProtoClusters",
                ["B0ECalRecHits", "B0ECalRawHitLinks"],
                ["B0ECalTruthProtoClusters"])
        app.add(calorimetry.CalorimeterIslandCluster, "B0ECalIslandProtoClusters",
                ["B0ECalRecHits"],
                ["B0ECalIslandProtoClusters"],
                sectorDist=5.0 * u.cm,
                dimScaledLocalDistXY=[1.8, 1.8],
                splitCluster=False,
                minClusterHitEdep=1.0 * u.MeV,
                minClusterCenterEdep=30.0 * u.MeV,
                transverseEnergyProfileMetric="globalDistEtaPhi",
                transverseEnergyProfileScale=1.0)
        app.add(calorimetry.CalorimeterClusterRecoCoG, "B0ECalClustersWithoutShapes",
                ["B0ECalIslandProtoClusters", "B0ECalRawHitLinks", "B0ECalRawHitAssociations"],
                ["B0ECalClustersWithoutShapes", "B0ECalClusterLinksWithoutShapes",
                 "B0ECalClusterAssociationsWithoutShapes"],
                energyWeight="log",
                sampFrac=1.0,
                logWeightBase=3.6,
                enableEtaBounds=False)
        app.add(calorimetry.CalorimeterClusterShape, "B0ECalClusters",
                ["B0ECalClustersWithoutShapes", "B0ECalClusterLinksWithoutShapes"],
                ["B0ECalClusters", "B0ECalClusterLinks", "B0ECalClusterAssociations"],
                energyWeight="log",
                logWeightBase=3.6)
        app.add(calorimetry.CalorimeterClusterRecoCoG, "B0ECalTruthClustersWithoutShapes",
                ["B0ECalTruthProtoClusters", "B0ECalRawHitLinks", "B0ECalRawHitAssociations"],
                ["B0ECalTruthClustersWithoutShapes", "B0ECalTruthClusterLinksWithoutShapes",
                 "B0ECalTruthClusterAssociationsWithoutShapes"],
                energyWeight="log",
                sampFrac=1.0,
                logWeightBase=6.2,
                enableEtaBounds=False)
        app.add(calorimetry.CalorimeterClusterShape, "B0ECalTruthClusters",
                ["B0ECalTruthClustersWithoutShapes", "B0ECalTruthClusterLinksWithoutShapes"],
                ["B0ECalTruthClusters", "B0ECalTruthClusterLinks",
                 "B0ECalTruthClusterAssociations"],
                energyWeight="log",
                logWeightBase=6.2)
