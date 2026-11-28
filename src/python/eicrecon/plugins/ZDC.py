# SPDX-License-Identifier: LGPL-3.0-or-later
"""ZDC wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import calorimetry

PLUGIN = "ZDC"


def configure(app, plugin=PLUGIN):
    side_length = 48.8 * u.mm

    with app.plugin(plugin):
        app.add(calorimetry.CalorimeterHitDigi, 'EcalFarForwardZDCRawHits',
                ['EventHeader', 'EcalFarForwardZDCHits'],
                ['EcalFarForwardZDCRawHits', 'EcalFarForwardZDCRawHitLinks', 'EcalFarForwardZDCRawHitAssociations'],
                capADC=32768,
                dyRangeADC=2000 * u.MeV,
                pedMeanADC=400,
                pedSigmaADC=3.2,
                resolutionTDC=10 * u.picosecond,
                readout="EcalFarForwardZDCHits")
        app.add(calorimetry.CalorimeterHitReco, 'EcalFarForwardZDCRecHits',
                ['EcalFarForwardZDCRawHits'],
                ['EcalFarForwardZDCRecHits'],
                capADC=32768,
                dyRangeADC=2000. * u.MeV,
                pedMeanADC=400,
                pedSigmaADC=3.2,
                resolutionTDC=10 * u.picosecond,
                thresholdFactor=4.0,
                readout="EcalFarForwardZDCHits")
        app.add(calorimetry.CalorimeterTruthClustering, 'EcalFarForwardZDCTruthProtoClusters',
                ['EcalFarForwardZDCRecHits', 'EcalFarForwardZDCRawHitLinks'],
                ['EcalFarForwardZDCTruthProtoClusters'])
        app.add(calorimetry.CalorimeterIslandCluster, 'EcalFarForwardZDCIslandProtoClusters',
                ['EcalFarForwardZDCRecHits'],
                ['EcalFarForwardZDCIslandProtoClusters'],
                sectorDist=5.0 * u.cm,
                localDistXY=[50 * u.cm, 50 * u.cm],
                splitCluster=True,
                minClusterHitEdep=0.1 * u.MeV,
                minClusterCenterEdep=3.0 * u.MeV,
                transverseEnergyProfileMetric="globalDistEtaPhi",
                transverseEnergyProfileScale=1.)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'EcalFarForwardZDCTruthClustersWithoutShapes',
                ['EcalFarForwardZDCTruthProtoClusters', 'EcalFarForwardZDCRawHitLinks', 'EcalFarForwardZDCRawHitAssociations'],
                ['EcalFarForwardZDCTruthClustersWithoutShapes', 'EcalFarForwardZDCTruthClusterLinksWithoutShapes', 'EcalFarForwardZDCTruthClusterAssociationsWithoutShapes'],
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterShape, 'EcalFarForwardZDCTruthClusters',
                ['EcalFarForwardZDCTruthClustersWithoutShapes', 'EcalFarForwardZDCTruthClusterLinksWithoutShapes'],
                ['EcalFarForwardZDCTruthClusters', 'EcalFarForwardZDCTruthClusterLinks', 'EcalFarForwardZDCTruthClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'EcalFarForwardZDCClustersWithoutShapes',
                ['EcalFarForwardZDCIslandProtoClusters', 'EcalFarForwardZDCRawHitLinks', 'EcalFarForwardZDCRawHitAssociations'],
                ['EcalFarForwardZDCClustersWithoutShapes', 'EcalFarForwardZDCClusterLinksWithoutShapes', 'EcalFarForwardZDCClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(calorimetry.CalorimeterClusterShape, 'EcalFarForwardZDCClusters',
                ['EcalFarForwardZDCClustersWithoutShapes', 'EcalFarForwardZDCClusterLinksWithoutShapes'],
                ['EcalFarForwardZDCClusters', 'EcalFarForwardZDCClusterLinks', 'EcalFarForwardZDCClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                logWeightBase=6.2)
        app.add(calorimetry.CalorimeterHitDigi, 'HcalFarForwardZDCRawHits',
                ['EventHeader', 'HcalFarForwardZDCHits'],
                ['HcalFarForwardZDCRawHits', 'HcalFarForwardZDCRawHitLinks', 'HcalFarForwardZDCRawHitAssociations'],
                capADC=65536,
                pedMeanADC=400,
                pedSigmaADC=2,
                resolutionTDC=10 * u.picosecond,
                readout="HcalFarForwardZDCHits")
        app.add(calorimetry.CalorimeterHitReco, 'HcalFarForwardZDCRecHits',
                ['HcalFarForwardZDCRawHits'],
                ['HcalFarForwardZDCRecHits'],
                capADC=65536,
                pedMeanADC=400,
                pedSigmaADC=2,
                resolutionTDC=10 * u.picosecond,
                thresholdFactor=3.0,
                readout="HcalFarForwardZDCHits",
                layerField="layer")
        app.add(calorimetry.HEXPLIT, 'HcalFarForwardZDCSubcellHits',
                ['HcalFarForwardZDCRecHits'],
                ['HcalFarForwardZDCSubcellHits'],
                MIP=630. * u.keV,
                Emin_in_MIPs=0.5,
                stag_type=2)
        app.add(calorimetry.ImagingTopoCluster, 'HcalFarForwardZDCImagingProtoClusters',
                ['HcalFarForwardZDCSubcellHits'],
                ['HcalFarForwardZDCImagingProtoClusters'],
                sameLayerDistXY=[side_length * 0.75, side_length * 0.75],
                diffLayerDistXY=[side_length * 0.75, side_length * 0.75],
                sectorDist=10.0 * u.cm,
                minClusterHitEdep=315.0 * u.keV,
                minClusterCenterEdep=25 * u.MeV,
                minClusterEdep=50.0 * u.MeV)
        app.add(calorimetry.CalorimeterIslandCluster, 'HcalFarForwardZDCIslandProtoClusters',
                ['HcalFarForwardZDCSubcellHits'],
                ['HcalFarForwardZDCIslandProtoClusters'],
                sectorDist=1.5 * u.cm,
                localDistXY=["0.55 * HcalFarForwardZDC_SiPMonTile_SquareSideLength", "0.55 * HcalFarForwardZDC_SiPMonTile_SquareSideLength"],
                minClusterHitEdep=100.0 * u.keV,
                minClusterCenterEdep=1.0 * u.MeV)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'HcalFarForwardZDCClustersWithoutShapes',
                ['HcalFarForwardZDCImagingProtoClusters', 'HcalFarForwardZDCRawHitLinks', 'HcalFarForwardZDCRawHitAssociations'],
                ['HcalFarForwardZDCClustersWithoutShapes', 'HcalFarForwardZDCClusterLinksWithoutShapes', 'HcalFarForwardZDCClusterAssociationsWithoutShapes'],
                energyWeight="log",
                sampFrac=0.0273,
                logWeightBaseCoeffs=[5.8, 0.65, 0.31],
                logWeightBase_Eref=50 * u.GeV)
        app.add(calorimetry.CalorimeterClusterShape, 'HcalFarForwardZDCClusters',
                ['HcalFarForwardZDCClustersWithoutShapes', 'HcalFarForwardZDCClusterLinksWithoutShapes'],
                ['HcalFarForwardZDCClusters', 'HcalFarForwardZDCClusterLinks', 'HcalFarForwardZDCClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                sampFrac=0.0273,
                logWeightBaseCoeffs=[5.8, 0.65, 0.31],
                logWeightBase_Eref=50 * u.GeV)
        app.add(calorimetry.CalorimeterTruthClustering, 'HcalFarForwardZDCTruthProtoClusters',
                ['HcalFarForwardZDCRecHits', 'HcalFarForwardZDCRawHitLinks'],
                ['HcalFarForwardZDCTruthProtoClusters'])
        app.add(calorimetry.CalorimeterIslandCluster, 'HcalFarForwardZDCIslandProtoClustersBaseline',
                ['HcalFarForwardZDCRecHits'],
                ['HcalFarForwardZDCIslandProtoClustersBaseline'],
                sectorDist=5.0 * u.cm,
                localDistXY=[50 * u.cm, 50 * u.cm],
                splitCluster=True,
                minClusterHitEdep=0.1 * u.MeV,
                minClusterCenterEdep=3.0 * u.MeV,
                transverseEnergyProfileMetric="globalDistEtaPhi",
                transverseEnergyProfileScale=1.)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'HcalFarForwardZDCTruthClustersWithoutShapes',
                ['HcalFarForwardZDCTruthProtoClusters', 'HcalFarForwardZDCRawHitLinks', 'HcalFarForwardZDCRawHitAssociations'],
                ['HcalFarForwardZDCTruthClustersWithoutShapes', 'HcalFarForwardZDCTruthClusterLinksWithoutShapes', 'HcalFarForwardZDCTruthClusterAssociationsWithoutShapes'],
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterShape, 'HcalFarForwardZDCTruthClusters',
                ['HcalFarForwardZDCTruthClustersWithoutShapes', 'HcalFarForwardZDCTruthClusterLinksWithoutShapes'],
                ['HcalFarForwardZDCTruthClusters', 'HcalFarForwardZDCTruthClusterLinks', 'HcalFarForwardZDCTruthClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'HcalFarForwardZDCClustersBaselineWithoutShapes',
                ['HcalFarForwardZDCIslandProtoClustersBaseline', 'HcalFarForwardZDCRawHitLinks', 'HcalFarForwardZDCRawHitAssociations'],
                ['HcalFarForwardZDCClustersBaselineWithoutShapes', 'HcalFarForwardZDCClusterLinksBaselineWithoutShapes', 'HcalFarForwardZDCClusterAssociationsBaselineWithoutShapes'],
                energyWeight="log",
                sampFrac=0.0273,
                logWeightBase=6.2)
        app.add(calorimetry.CalorimeterClusterShape, 'HcalFarForwardZDCClustersBaseline',
                ['HcalFarForwardZDCClustersBaselineWithoutShapes', 'HcalFarForwardZDCClusterLinksBaselineWithoutShapes'],
                ['HcalFarForwardZDCClustersBaseline', 'HcalFarForwardZDCClusterLinksBaseline', 'HcalFarForwardZDCClusterAssociationsBaseline'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                sampFrac=0.0273,
                logWeightBase=6.2)
