# SPDX-License-Identifier: LGPL-3.0-or-later
"""FEMC wiring, converted from the former C++ InitPlugin;
the geometry-dependent digitization is a deferred section, written by hand.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "FEMC"


def configure(app, plugin=PLUGIN):
    T = bindings("FEMC")
    EcalEndcapP_capADC = 16384
    EcalEndcapP_dyRangeADC = 100 * u.GeV
    EcalEndcapP_pedMeanADC = 200
    EcalEndcapP_pedSigmaADC = 2.4576
    EcalEndcapP_resolutionTDC = 10 * u.picosecond
    EcalEndcapP_sampFrac = 0.029043  # updated with ratio to ScFi model
    EcalEndcapP_corrMeanScale = repr(1.0 / EcalEndcapP_sampFrac)  # std::format("{}", ...)
    EcalEndcapP_nPhotonPerGeV = 1500
    EcalEndcapP_PhotonCollectionEff = 0.285
    EcalEndcapP_totalPixel = 4 * 159565

    with app.plugin(plugin):
        # The digitization depends on the calorimeter model in the geometry
        def digitization(graph):
            # EcalEndcapP_Homogeneous_ScFi: <= 1 homogeneous, 2 ScFi; absent: legacy homogeneous
            flag = graph.constant_int("EcalEndcapP_Homogeneous_ScFi") or 0
            common = dict(
                tRes=0.0,
                threshold=0.0,  # 15 MeV threshold for a single tower is applied on ADC at reco
                readoutType="sipm",
                photonDetectionEfficiency=EcalEndcapP_PhotonCollectionEff,
                numEffectiveSipmPixels=EcalEndcapP_totalPixel,
                capADC=EcalEndcapP_capADC,
                capTime=100,  # given in ns, 4 samples in HGCROC
                dyRangeADC=EcalEndcapP_dyRangeADC,
                pedMeanADC=EcalEndcapP_pedMeanADC,
                pedSigmaADC=EcalEndcapP_pedSigmaADC,
                resolutionTDC=EcalEndcapP_resolutionTDC,
                readout="EcalEndcapPHits",
            )
            if flag <= 1:
                config = dict(
                    common,
                    eRes=[0.11333 * sqrt(u.GeV), 0.03, 0.0 * u.GeV],  # 11.333%/sqrt(E) + 3%
                    lightYield=EcalEndcapP_nPhotonPerGeV / EcalEndcapP_PhotonCollectionEff,
                    corrMeanScale="1.0",
                )
            elif flag == 2:
                config = dict(
                    common,
                    eRes=[0.0, 0.022, 0.0],  # just constant term 2.2% based on MC data
                    lightYield=EcalEndcapP_nPhotonPerGeV / EcalEndcapP_PhotonCollectionEff
                    / EcalEndcapP_sampFrac,
                    corrMeanScale=EcalEndcapP_corrMeanScale,
                    fields=["fiber_x", "fiber_y"],
                )
            else:
                return
            graph.add(T.CalorimeterHitDigi, "EcalEndcapPRawHits",
                      ["EventHeader", "EcalEndcapPHits"],
                      ["EcalEndcapPRawHits", "EcalEndcapPRawHitLinks",
                       "EcalEndcapPRawHitAssociations"],
                      **config)

        app.deferred(digitization)
        app.add(T.CalorimeterHitReco, 'EcalEndcapPRecHits',
                ['EcalEndcapPRawHits'],
                ['EcalEndcapPRecHits'],
                capADC=EcalEndcapP_capADC,
                dyRangeADC=EcalEndcapP_dyRangeADC,
                pedMeanADC=EcalEndcapP_pedMeanADC,
                pedSigmaADC=EcalEndcapP_pedSigmaADC,
                resolutionTDC=EcalEndcapP_resolutionTDC,
                thresholdValue=3,
                sampFrac="1.00",
                readout="EcalEndcapPHits")
        app.add(T.CalorimeterTruthClustering, 'EcalEndcapPTruthProtoClusters',
                ['EcalEndcapPRecHits', 'EcalEndcapPRawHitLinks'],
                ['EcalEndcapPTruthProtoClusters'])
        app.add(T.CalorimeterIslandCluster, 'EcalEndcapPIslandProtoClusters',
                ['EcalEndcapPRecHits'],
                ['EcalEndcapPIslandProtoClusters'],
                sectorDist=5.0 * u.cm,
                dimScaledLocalDistXY=[1.5, 1.5],
                minClusterCenterEdep=60.0 * u.MeV,
                transverseEnergyProfileMetric="dimScaledLocalDistXY",
                transverseEnergyProfileScale=1.)
        app.add(T.CalorimeterClusterRecoCoG, 'EcalEndcapPTruthClustersWithoutShapes',
                ['EcalEndcapPTruthProtoClusters', 'EcalEndcapPRawHitLinks', 'EcalEndcapPRawHitAssociations'],
                ['EcalEndcapPTruthClustersWithoutShapes', 'EcalEndcapPTruthClusterLinksWithoutShapes', 'EcalEndcapPTruthClusterAssociationsWithoutShapes'],
                energyWeight="log",
                enableEtaBounds=True)
        app.add(T.CalorimeterClusterShape, 'EcalEndcapPTruthClusters',
                ['EcalEndcapPTruthClustersWithoutShapes', 'EcalEndcapPTruthClusterLinksWithoutShapes'],
                ['EcalEndcapPTruthClusters', 'EcalEndcapPTruthClusterLinks', 'EcalEndcapPTruthClusterAssociations'],
                energyWeight="log")
        app.add(T.CalorimeterClusterRecoCoG, 'EcalEndcapPClustersWithoutShapes',
                ['EcalEndcapPIslandProtoClusters', 'EcalEndcapPRawHitLinks', 'EcalEndcapPRawHitAssociations'],
                ['EcalEndcapPClustersWithoutShapes', 'EcalEndcapPClusterLinksWithoutShapes', 'EcalEndcapPClusterAssociationsWithoutShapes'],
                energyWeight="log")
        app.add(T.CalorimeterClusterShape, 'EcalEndcapPClusters',
                ['EcalEndcapPClustersWithoutShapes', 'EcalEndcapPClusterLinksWithoutShapes'],
                ['EcalEndcapPClusters', 'EcalEndcapPClusterLinks', 'EcalEndcapPClusterAssociations'],
                energyWeight="log")
