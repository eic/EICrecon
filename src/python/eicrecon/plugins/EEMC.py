# SPDX-License-Identifier: LGPL-3.0-or-later
"""EEMC wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import calorimetry, meta

PLUGIN = "EEMC"


def configure(app, plugin=PLUGIN):
    EcalEndcapN_capADC = 16384
    EcalEndcapN_dyRangeADC = 20.0 * u.GeV
    EcalEndcapN_pedMeanADC = 20
    EcalEndcapN_pedSigmaADC = 1
    EcalEndcapN_resolutionTDC = 10 * u.picosecond

    with app.plugin(plugin):
        app.add(calorimetry.CalorimeterHitDigi, 'EcalEndcapNRawHits',
                ['EventHeader', 'EcalEndcapNHits'],
                ['EcalEndcapNRawHits', 'EcalEndcapNRawHitLinks', 'EcalEndcapNRawHitAssociations'],
                eRes=[0.0 * sqrt(u.GeV), 0.0, 0.0 * u.GeV],
                threshold=0.0 * u.MeV,
                readoutType="sipm",
                lightYield=18. / 0.25 / u.MeV,
                photonDetectionEfficiency=(16 * (3. * 3.) / (20. * 20.)) * 0.28,
                numEffectiveSipmPixels=39984 * 16,
                capADC=EcalEndcapN_capADC,
                dyRangeADC=EcalEndcapN_dyRangeADC,
                pedMeanADC=EcalEndcapN_pedMeanADC,
                pedSigmaADC=EcalEndcapN_pedSigmaADC,
                resolutionTDC=EcalEndcapN_resolutionTDC,
                readout="EcalEndcapNHits")
        app.add(calorimetry.CalorimeterHitReco, 'EcalEndcapNRecHits',
                ['EcalEndcapNRawHits'],
                ['EcalEndcapNRecHits'],
                capADC=EcalEndcapN_capADC,
                dyRangeADC=EcalEndcapN_dyRangeADC,
                pedMeanADC=EcalEndcapN_pedMeanADC,
                pedSigmaADC=EcalEndcapN_pedSigmaADC,
                resolutionTDC=EcalEndcapN_resolutionTDC,
                thresholdValue=4.0,
                sampFrac="0.96",
                readout="EcalEndcapNHits")
        app.add(calorimetry.CalorimeterTruthClustering, 'EcalEndcapNTruthProtoClusters',
                ['EcalEndcapNRecHits', 'EcalEndcapNRawHitLinks'],
                ['EcalEndcapNTruthProtoClusters'])
        app.add(calorimetry.CalorimeterIslandCluster, 'EcalEndcapNIslandProtoClusters',
                ['EcalEndcapNRecHits'],
                ['EcalEndcapNIslandProtoClusters'],
                adjacencyMatrix="(abs(row_1 - row_2) + abs(column_1 - column_2)) == 1",
                peakNeighbourhoodMatrix="max(abs(row_1 - row_2), abs(column_1 - column_2)) == 1",
                readout="EcalEndcapNHits",
                sectorDist=5.0 * u.cm,
                splitCluster=True,
                minClusterHitEdep=1.0 * u.MeV,
                minClusterCenterEdep=30.0 * u.MeV,
                transverseEnergyProfileMetric="globalDistEtaPhi",
                transverseEnergyProfileScale=0.08)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'EcalEndcapNTruthClustersWithoutShapes',
                ['EcalEndcapNTruthProtoClusters', 'EcalEndcapNRawHitLinks', 'EcalEndcapNRawHitAssociations'],
                ['EcalEndcapNTruthClustersWithoutShapes', 'EcalEndcapNTruthClusterLinksWithoutShapes', 'EcalEndcapNTruthClusterAssociationsWithoutShapes'],
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterShape, 'EcalEndcapNTruthClusters',
                ['EcalEndcapNTruthClustersWithoutShapes', 'EcalEndcapNTruthClusterLinksWithoutShapes'],
                ['EcalEndcapNTruthClusters', 'EcalEndcapNTruthClusterLinks', 'EcalEndcapNTruthClusterAssociations'],
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'EcalEndcapNClustersWithoutPIDAndShapes',
                ['EcalEndcapNIslandProtoClusters', 'EcalEndcapNRawHitLinks', 'EcalEndcapNRawHitAssociations'],
                ['EcalEndcapNClustersWithoutPIDAndShapes', 'EcalEndcapNClusterLinksWithoutPIDAndShapes', 'EcalEndcapNClusterAssociationsWithoutPIDAndShapes'],
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterShape, 'EcalEndcapNClustersWithoutPID',
                ['EcalEndcapNClustersWithoutPIDAndShapes', 'EcalEndcapNClusterLinksWithoutPIDAndShapes'],
                ['EcalEndcapNClustersWithoutPID', 'EcalEndcapNClusterLinksWithoutPID', 'EcalEndcapNClusterAssociationsWithoutPID'],
                energyWeight="log")
        app.add(calorimetry.CalorimeterParticleIDPreML, 'EcalEndcapNParticleIDPreML',
                ['EcalEndcapNClustersWithoutPID', 'EcalEndcapNClusterLinksWithoutPID'],
                ['EcalEndcapNParticleIDInput_features', 'EcalEndcapNParticleIDTarget'])
        app.add(meta.ONNXInference, 'EcalEndcapNParticleIDInference',
                ['EcalEndcapNParticleIDInput_features'],
                ['EcalEndcapNParticleIDOutput_label', 'EcalEndcapNParticleIDOutput_probability_tensor'],
                modelPath="calibrations/onnx/EcalEndcapN_pi_rejection.onnx")
        app.add(calorimetry.CalorimeterParticleIDPostML, 'EcalEndcapNParticleIDPostML',
                ['EcalEndcapNClustersWithoutPID', 'EcalEndcapNClusterLinksWithoutPID', 'EcalEndcapNParticleIDOutput_probability_tensor'],
                ['EcalEndcapNClusters', 'EcalEndcapNClusterLinks', 'EcalEndcapNClusterAssociations', 'EcalEndcapNClusterParticleIDs'])
