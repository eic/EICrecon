# SPDX-License-Identifier: LGPL-3.0-or-later
"""FHCAL wiring, converted from the former C++ InitPlugin;
the geometry-dependent insert clustering is a deferred section, written by hand.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import calorimetry

PLUGIN = "FHCAL"


def configure(app, plugin=PLUGIN):
    HcalEndcapPInsert_capADC = 32768
    HcalEndcapPInsert_dyRangeADC = 200 * u.MeV
    HcalEndcapPInsert_pedMeanADC = 10
    HcalEndcapPInsert_pedSigmaADC = 2
    HcalEndcapPInsert_resolutionTDC = 10 * u.picosecond
    LFHCAL_capADC = 65536
    LFHCAL_pedMeanADC = 50
    LFHCAL_pedSigmaADC = 10
    LFHCAL_resolutionTDC = 10 * u.picosecond

    with app.plugin(plugin):
        app.add(calorimetry.CalorimeterHitDigi, 'HcalEndcapPInsertRawHits',
                ['EventHeader', 'HcalEndcapPInsertHits'],
                ['HcalEndcapPInsertRawHits', 'HcalEndcapPInsertRawHitLinks', 'HcalEndcapPInsertRawHitAssociations'],
                capADC=HcalEndcapPInsert_capADC,
                dyRangeADC=HcalEndcapPInsert_dyRangeADC,
                pedMeanADC=HcalEndcapPInsert_pedMeanADC,
                pedSigmaADC=HcalEndcapPInsert_pedSigmaADC,
                resolutionTDC=HcalEndcapPInsert_resolutionTDC,
                readout="HcalEndcapPInsertHits")
        app.add(calorimetry.CalorimeterHitReco, 'HcalEndcapPInsertRecHits',
                ['HcalEndcapPInsertRawHits'],
                ['HcalEndcapPInsertRecHits'],
                capADC=HcalEndcapPInsert_capADC,
                dyRangeADC=HcalEndcapPInsert_dyRangeADC,
                pedMeanADC=HcalEndcapPInsert_pedMeanADC,
                pedSigmaADC=HcalEndcapPInsert_pedSigmaADC,
                resolutionTDC=HcalEndcapPInsert_resolutionTDC,
                thresholdValue=41.0,
                readout="HcalEndcapPInsertHits",
                layerField="layer")
        app.add(calorimetry.CalorimeterHitsMerger, 'HcalEndcapPInsertMergedHits',
                ['HcalEndcapPInsertRecHits'],
                ['HcalEndcapPInsertMergedHits'],
                readout="HcalEndcapPInsertHits",
                fieldTransformations=["layer:1"])
        app.add(calorimetry.CalorimeterTruthClustering, 'HcalEndcapPInsertTruthProtoClusters',
                ['HcalEndcapPInsertMergedHits', 'HcalEndcapPInsertRawHitLinks'],
                ['HcalEndcapPInsertTruthProtoClusters'])
        # Clustering of the insert: the scheme depends on the readout segmentation
        def insert_clustering(graph):
            segmentation = graph.readout_segmentation_type("HcalEndcapPInsertHits")
            if segmentation == "NoSegmentation":
                # new insert design with physical tiles
                graph.add(calorimetry.CalorimeterIslandCluster, "HcalEndcapPInsertImagingProtoClusters",
                          ["HcalEndcapPInsertRecHits"],
                          ["HcalEndcapPInsertImagingProtoClusters"],
                          sectorDist=10.0 * u.cm,
                          dimScaledLocalDistXY=[1.5, 1.5],
                          splitCluster=False,
                          minClusterHitEdep=5.0 * u.keV,
                          minClusterCenterEdep=3.0 * u.MeV)
            else:
                # previous scheme (also when the insert readout is unavailable)
                graph.add(calorimetry.HEXPLIT, "HcalEndcapPInsertSubcellHits",
                          ["HcalEndcapPInsertRecHits"],
                          ["HcalEndcapPInsertSubcellHits"],
                          MIP=480.0 * u.keV,
                          Emin_in_MIPs=0.5)
                cell = "max(HcalEndcapPInsertCellSizeLGRight, HcalEndcapPInsertCellSizeLGLeft)"
                graph.add(calorimetry.ImagingTopoCluster, "HcalEndcapPInsertImagingProtoClusters",
                          ["HcalEndcapPInsertSubcellHits"],
                          ["HcalEndcapPInsertImagingProtoClusters"],
                          neighbourLayersRange=1,
                          sameLayerDistXY=[f"0.5 * {cell}", f"0.5 * {cell} * sin(pi / 3)"],
                          diffLayerDistXY=[f"0.25 * {cell}", f"0.25 * {cell} * sin(pi / 3)"],
                          sameLayerMode=1,  # ImagingTopoClusterConfig::ELayerMode::xy
                          sectorDist=10.0 * u.cm,
                          minClusterHitEdep=5.0 * u.keV,
                          minClusterCenterEdep=3.0 * u.MeV,
                          minClusterEdep=11.0 * u.MeV,
                          minClusterNhits=100)

        app.deferred(insert_clustering)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'HcalEndcapPInsertTruthClustersWithoutShapes',
                ['HcalEndcapPInsertTruthProtoClusters', 'HcalEndcapPInsertRawHitLinks', 'HcalEndcapPInsertRawHitAssociations'],
                ['HcalEndcapPInsertTruthClustersWithoutShapes', 'HcalEndcapPInsertTruthClusterLinksWithoutShapes', 'HcalEndcapPInsertTruthClusterAssociationsWithoutShapes'],
                energyWeight="log",
                sampFrac=0.0257,
                enableEtaBounds=True)
        app.add(calorimetry.CalorimeterClusterShape, 'HcalEndcapPInsertTruthClusters',
                ['HcalEndcapPInsertTruthClustersWithoutShapes', 'HcalEndcapPInsertTruthClusterLinksWithoutShapes'],
                ['HcalEndcapPInsertTruthClusters', 'HcalEndcapPInsertTruthClusterLinks', 'HcalEndcapPInsertTruthClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'HcalEndcapPInsertClustersWithoutShapes',
                ['HcalEndcapPInsertImagingProtoClusters', 'HcalEndcapPInsertRawHitLinks', 'HcalEndcapPInsertRawHitAssociations'],
                ['HcalEndcapPInsertClustersWithoutShapes', 'HcalEndcapPInsertClusterLinksWithoutShapes', 'HcalEndcapPInsertClusterAssociationsWithoutShapes'],
                energyWeight="log",
                sampFrac=0.0257,
                logWeightBase=6.2)
        app.add(calorimetry.CalorimeterClusterShape, 'HcalEndcapPInsertClusters',
                ['HcalEndcapPInsertClustersWithoutShapes', 'HcalEndcapPInsertClusterLinksWithoutShapes'],
                ['HcalEndcapPInsertClusters', 'HcalEndcapPInsertClusterLinks', 'HcalEndcapPInsertClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                sampFrac=0.0257,
                logWeightBase=6.2)
        app.add(calorimetry.CalorimeterHitDigi, 'LFHCALRawHits',
                ['EventHeader', 'LFHCALHits'],
                ['LFHCALRawHits', 'LFHCALRawHitLinks', 'LFHCALRawHitAssociations'],
                capADC=LFHCAL_capADC,
                capTime=100,
                pedMeanADC=LFHCAL_pedMeanADC,
                pedSigmaADC=LFHCAL_pedSigmaADC,
                resolutionTDC=LFHCAL_resolutionTDC,
                readout="LFHCALHits",
                fields=["layerz"])
        app.add(calorimetry.CalorimeterHitReco, 'LFHCALRecHits',
                ['LFHCALRawHits'],
                ['LFHCALRecHits'],
                capADC=LFHCAL_capADC,
                pedMeanADC=LFHCAL_pedMeanADC,
                pedSigmaADC=LFHCAL_pedSigmaADC,
                resolutionTDC=LFHCAL_resolutionTDC,
                thresholdValue=20,
                sampFrac='(rlayerz == 0) ? 0.019 : 0.037',
                readout="LFHCALHits",
                layerField="rlayerz")
        app.add(calorimetry.CalorimeterTruthClustering, 'LFHCALTruthProtoClusters',
                ['LFHCALRecHits', 'LFHCALRawHitLinks'],
                ['LFHCALTruthProtoClusters'])
        app.add(calorimetry.CalorimeterIslandCluster, 'LFHCALIslandProtoClusters',
                ['LFHCALRecHits'],
                ['LFHCALIslandProtoClusters'],
                adjacencyMatrix='(abs((54*2-moduleIDx_2*2-towerx_2)-(54*2-moduleIDx_1*2-towerx_1))+abs((54*2-moduleIDy_2*2-towery_2)-(54*2-moduleIDy_1*2-towery_1))+abs(rlayerz_2-rlayerz_1)==1)||((abs(rlayerz_2-rlayerz_1)==0&&abs((54*2-moduleIDx_2*2-towerx_2)-(54*2-moduleIDx_1*2-towerx_1))==1&&abs((54*2-moduleIDy_2*2-towery_2)-(54*2-moduleIDy_1*2-towery_1))==1)||(abs(rlayerz_2-rlayerz_1)==1&&abs((54*2-moduleIDx_2*2-towerx_2)-(54*2-moduleIDx_1*2-towerx_1))==0&&abs((54*2-moduleIDy_2*2-towery_2)-(54*2-moduleIDy_1*2-towery_1))==1)||(abs(rlayerz_2-rlayerz_1)==1&&abs((54*2-moduleIDx_2*2-towerx_2)-(54*2-moduleIDx_1*2-towerx_1))==1&&abs((54*2-moduleIDy_2*2-towery_2)-(54*2-moduleIDy_1*2-towery_1))==0))',
                readout="LFHCALHits",
                minClusterHitEdep=1 * u.MeV,
                minClusterCenterEdep=100.0 * u.MeV,
                transverseEnergyProfileMetric="globalDistEtaPhi",
                transverseEnergyProfileScale=1.)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'LFHCALTruthClustersWithoutShapes',
                ['LFHCALTruthProtoClusters', 'LFHCALRawHitLinks', 'LFHCALRawHitAssociations'],
                ['LFHCALTruthClustersWithoutShapes', 'LFHCALTruthClusterLinksWithoutShapes', 'LFHCALTruthClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=4.5)
        app.add(calorimetry.CalorimeterClusterShape, 'LFHCALTruthClusters',
                ['LFHCALTruthClustersWithoutShapes', 'LFHCALTruthClusterLinksWithoutShapes'],
                ['LFHCALTruthClusters', 'LFHCALTruthClusterLinks', 'LFHCALTruthClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                logWeightBase=4.5)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'LFHCALClustersWithoutShapes',
                ['LFHCALIslandProtoClusters', 'LFHCALRawHitLinks', 'LFHCALRawHitAssociations'],
                ['LFHCALClustersWithoutShapes', 'LFHCALClusterLinksWithoutShapes', 'LFHCALClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=4.5)
        app.add(calorimetry.CalorimeterClusterShape, 'LFHCALClusters',
                ['LFHCALClustersWithoutShapes', 'LFHCALClusterLinksWithoutShapes'],
                ['LFHCALClusters', 'LFHCALClusterLinks', 'LFHCALClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                logWeightBase=4.5)
