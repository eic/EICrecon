# SPDX-License-Identifier: LGPL-3.0-or-later
"""BEMC wiring, converted from src/detectors/BEMC/BEMC.cc by src/python/tools/generate_wiring.py.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "BEMC"


def configure(app, plugin=PLUGIN):
    T = bindings("BEMC")
    EcalBarrelScFi_attPars = [ 0.416212, 747.39875 * eu.mm, 7521.88383 * eu.mm]
    EcalBarrelScFi_hitMergeFields = [ "fiber", "z"]
    EcalBarrelScFi_contributionMergeFields = ["fiber"]
    EcalBarrelScFi_edep_to_npe_fields = [ "layer"]
    EcalBarrelScFi_pulse_shape_params = [ 55.37, 10 * eu.ns]
    EcalBarrelImaging_capADC = 8192
    EcalBarrelImaging_dyRangeADC = 3 * u.MeV
    EcalBarrelImaging_pedMeanADC = 14
    EcalBarrelImaging_pedSigmaADC = 5

    with app.plugin(plugin):
        app.add(T.SimCalorimeterHitProcessor, 'EcalBarrelScFiPAttenuatedHits',
                ['EcalBarrelScFiHits'],
                ['EcalBarrelScFiPAttenuatedHits', 'EcalBarrelScFiPAttenuatedHitContributions'],
                attenuationParameters=EcalBarrelScFi_attPars,
                readout="EcalBarrelScFiHits",
                attenuationReferencePositionName="EcalBarrel_LightGuide_PositivePosZ",
                hitMergeFields=EcalBarrelScFi_hitMergeFields,
                contributionMergeFields=EcalBarrelScFi_contributionMergeFields,
                inversePropagationSpeed=0.00625,
                fixedTimeDelay=2.0)
        app.add(T.SimCalorimeterHitProcessor, 'EcalBarrelScFiNAttenuatedHits',
                ['EcalBarrelScFiHits'],
                ['EcalBarrelScFiNAttenuatedHits', 'EcalBarrelScFiNAttenuatedHitContributions'],
                attenuationParameters=EcalBarrelScFi_attPars,
                readout="EcalBarrelScFiHits",
                attenuationReferencePositionName="EcalBarrel_LightGuide_NegativePosZ",
                hitMergeFields=EcalBarrelScFi_hitMergeFields,
                contributionMergeFields=EcalBarrelScFi_contributionMergeFields,
                inversePropagationSpeed=0.00625,
                fixedTimeDelay=2.0)
        app.add(T.EdepToNpeConversion, 'EcalBarrelScFiPNpeHits',
                ['EventHeader', 'EcalBarrelScFiPAttenuatedHits'],
                ['EcalBarrelScFiPNpeHits', 'EcalBarrelScFiPNpeSources'],
                readout="EcalBarrelScFiHits",
                edep_to_npe_fields=EcalBarrelScFi_edep_to_npe_fields,
                edep_to_npe_filename='calibrations/bic_edepToNpe_layer.lut')
        app.add(T.EdepToNpeConversion, 'EcalBarrelScFiNNpeHits',
                ['EventHeader', 'EcalBarrelScFiNAttenuatedHits'],
                ['EcalBarrelScFiNNpeHits', 'EcalBarrelScFiNNpeSources'],
                readout="EcalBarrelScFiHits",
                edep_to_npe_fields=EcalBarrelScFi_edep_to_npe_fields,
                edep_to_npe_filename='calibrations/bic_edepToNpe_layer.lut')
        app.add(T.PulseGeneration_SimCalorimeterHit, 'EcalBarrelScFiPPulses',
                ['EcalBarrelScFiPNpeHits', 'EcalBarrelScFiPNpeSources'],
                ['EcalBarrelScFiPPulses'],
                pulse_shape_params=EcalBarrelScFi_pulse_shape_params,
                ignore_thres=1.0,
                timestep=0.5)
        app.add(T.PulseGeneration_SimCalorimeterHit, 'EcalBarrelScFiNPulses',
                ['EcalBarrelScFiNNpeHits', 'EcalBarrelScFiNNpeSources'],
                ['EcalBarrelScFiNPulses'],
                pulse_shape_params=EcalBarrelScFi_pulse_shape_params,
                ignore_thres=1.0,
                timestep=0.5)
        app.add(T.PulseCombiner, 'EcalBarrelScFiPCombinedPulses',
                ['EcalBarrelScFiPPulses'],
                ['EcalBarrelScFiPCombinedPulses'],
                minimum_separation=100.0,
                readout="EcalBarrelScFiHits",
                combine_field='grid')
        app.add(T.PulseCombiner, 'EcalBarrelScFiNCombinedPulses',
                ['EcalBarrelScFiNPulses'],
                ['EcalBarrelScFiNCombinedPulses'],
                minimum_separation=100.0,
                readout="EcalBarrelScFiHits",
                combine_field='grid')
        app.add(T.PulseNoise, 'EcalBarrelScFiPCombinedPulsesWithNoise',
                ['EventHeader', 'EcalBarrelScFiPCombinedPulses'],
                ['EcalBarrelScFiPCombinedPulsesWithNoise'],
                alpha=1.8,
                scale=0.35,
                pedestal=6.1)
        app.add(T.PulseNoise, 'EcalBarrelScFiNCombinedPulsesWithNoise',
                ['EventHeader', 'EcalBarrelScFiNCombinedPulses'],
                ['EcalBarrelScFiNCombinedPulsesWithNoise'],
                alpha=1.8,
                scale=0.35,
                pedestal=6.1)
        app.add(T.CALOROCDigitization, 'EcalBarrelScFiPCALOROCHits',
                ['EcalBarrelScFiPCombinedPulsesWithNoise'],
                ['EcalBarrelScFiPCALOROCHits', 'EcalBarrelScFiPCALOROCPulses'],
                adc_phase=10.0,
                toa_thres=13.1,
                tot_thres=200.0,
                dyRangeSingleGainADC=250.0,
                dyRangeHighGainADC=250.0,
                dyRangeLowGainADC=2500.0)
        app.add(T.CALOROCDigitization, 'EcalBarrelScFiNCALOROCHits',
                ['EcalBarrelScFiNCombinedPulsesWithNoise'],
                ['EcalBarrelScFiNCALOROCHits', 'EcalBarrelScFiNCALOROCPulses'],
                adc_phase=10.0,
                toa_thres=13.1,
                tot_thres=200.0,
                dyRangeSingleGainADC=250.0,
                dyRangeHighGainADC=250.0,
                dyRangeLowGainADC=2500.0)
        app.add(T.CalorimeterCALOROCReco, 'EcalBarrelScFiCalibration',
                ['EcalBarrelScFiPNpeHits', 'EcalBarrelScFiPCALOROCHits', 'EcalBarrelScFiNNpeHits', 'EcalBarrelScFiNCALOROCHits'],
                ['EcalBarrelScFiRecHits', 'EcalBarrelScFiRawHits', 'EcalBarrelScFiRawHitLinks', 'EcalBarrelScFiRawHitAssociations'],
                readout="EcalBarrelScFiHits",
                layerField="layer",
                sectorField="sector",
                localDetFields=["system", "sector"],
                maskPos="xy",
                maskPosFields=["fiber", "z"],
                edep_to_npe_filename='calibrations/bic_edepToNpe_layer.lut',
                edep_to_npe_fields=EcalBarrelScFi_edep_to_npe_fields,
                timeWalkCor=True,
                highGainDR=1023,
                gainRatio=10.0,
                attenuationReferencePositionNamePos="EcalBarrel_LightGuide_PositivePosZ",
                attenuationReferencePositionNameNeg="EcalBarrel_LightGuide_NegativePosZ",
                attenuationParameters=EcalBarrelScFi_attPars,
                timeWalkCorrectionParameters=[-13.7915, 33.5238, 3.15088, -0.313885],
                lightSpeedParameters=[83.3221, -417.8],
                slope=13.02)
        app.add(T.CalorimeterIslandCluster, 'EcalBarrelScFiProtoClusters',
                ['EcalBarrelScFiRecHits'],
                ['EcalBarrelScFiProtoClusters'],
                sectorDist=80. * u.mm,
                localDistXZ=[80 * u.mm, 80 * u.mm],
                minClusterHitEdep=5.0 * u.MeV,
                minClusterCenterEdep=100.0 * u.MeV)
        app.add(T.CalorimeterClusterRecoCoG, 'EcalBarrelScFiClustersWithoutShapes',
                ['EcalBarrelScFiProtoClusters', 'EcalBarrelScFiRawHitLinks', 'EcalBarrelScFiRawHitAssociations'],
                ['EcalBarrelScFiClustersWithoutShapes', 'EcalBarrelScFiClusterLinksWithoutShapes', 'EcalBarrelScFiClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.CalorimeterClusterShape, 'EcalBarrelScFiClusters',
                ['EcalBarrelScFiClustersWithoutShapes', 'EcalBarrelScFiClusterLinksWithoutShapes'],
                ['EcalBarrelScFiClusters', 'EcalBarrelScFiClusterLinks', 'EcalBarrelScFiClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.ImagingTopoCluster, 'EcalBarrelScFiProtoTopoClusters',
                ['EcalBarrelScFiRecHits'],
                ['EcalBarrelScFiProtoTopoClusters'],
                neighbourLayersRange=2,
                sameLayerDistXYZ=[80.0 * u.mm, 80.0 * u.mm, 40.0 * u.mm],
                diffLayerDistXYZ=[80.0 * u.mm, 80.0 * u.mm, 40.0 * u.mm],
                sameLayerMode=3,
                diffLayerMode=3,
                sectorDist=5.0 * u.cm,
                minClusterEdep=100 * u.MeV,
                minClusterNhits=4)
        app.add(T.CalorimeterClusterRecoCoG, 'EcalBarrelScFiTopoClustersWithoutShapes',
                ['EcalBarrelScFiProtoTopoClusters', 'EcalBarrelScFiRawHitLinks', 'EcalBarrelScFiRawHitAssociations'],
                ['EcalBarrelScFiTopoClustersWithoutShapes', 'EcalBarrelScFiTopoClusterLinksWithoutShapes', 'EcalBarrelScFiTopoClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.CalorimeterClusterShape, 'EcalBarrelScFiTopoClusters',
                ['EcalBarrelScFiTopoClustersWithoutShapes', 'EcalBarrelScFiTopoClusterLinksWithoutShapes'],
                ['EcalBarrelScFiTopoClusters', 'EcalBarrelScFiTopoClusterLinks', 'EcalBarrelScFiTopoClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.SimCalorimeterHitProcessor, 'EcalBarrelImagingProcessedHits',
                ['EcalBarrelImagingHits'],
                ['EcalBarrelImagingProcessedHits', 'EcalBarrelImagingProcessedHitContributions'],
                readout="EcalBarrelImagingHits")
        app.add(T.CalorimeterHitDigi, 'EcalBarrelImagingRawHits',
                ['EventHeader', 'EcalBarrelImagingProcessedHits'],
                ['EcalBarrelImagingRawHits', 'EcalBarrelImagingRawHitLinks', 'EcalBarrelImagingRawHitAssociations'],
                eRes=[0.0 * sqrt(u.GeV), 0.02, 0.0 * u.GeV],
                capADC=EcalBarrelImaging_capADC,
                dyRangeADC=EcalBarrelImaging_dyRangeADC,
                pedMeanADC=EcalBarrelImaging_pedMeanADC,
                pedSigmaADC=EcalBarrelImaging_pedSigmaADC,
                resolutionTDC=3.25e-09,
                readout="EcalBarrelImagingHits")
        app.add(T.CalorimeterHitReco, 'EcalBarrelImagingRecHits',
                ['EcalBarrelImagingRawHits'],
                ['EcalBarrelImagingRecHits'],
                capADC=EcalBarrelImaging_capADC,
                dyRangeADC=EcalBarrelImaging_dyRangeADC,
                pedMeanADC=EcalBarrelImaging_pedMeanADC,
                pedSigmaADC=EcalBarrelImaging_pedSigmaADC,
                resolutionTDC=3.25e-09,
                thresholdValue=41,
                sampFrac="0.00429453",
                readout="EcalBarrelImagingHits",
                layerField="layer",
                sectorField="sector")
        app.add(T.ImagingTopoCluster, 'EcalBarrelImagingProtoClusters',
                ['EcalBarrelImagingRecHits'],
                ['EcalBarrelImagingProtoClusters'],
                neighbourLayersRange=2,
                sameLayerDistTZ=[2.0 * u.mm, 2 * u.mm],
                sameLayerMode=2,
                diffLayerMode=0,
                sectorDist=3.0 * u.cm,
                minClusterEdep=100 * u.MeV)
        app.add(T.ImagingClusterReco, 'EcalBarrelImagingClustersWithoutShapes',
                ['EcalBarrelImagingProtoClusters', 'EcalBarrelImagingRawHitLinks', 'EcalBarrelImagingRawHitAssociations'],
                ['EcalBarrelImagingClustersWithoutShapes', 'EcalBarrelImagingClusterLinksWithoutShapes', 'EcalBarrelImagingClusterAssociationsWithoutShapes', 'EcalBarrelImagingLayers'],
                trackStopLayer=6)
        app.add(T.CalorimeterClusterShape, 'EcalBarrelImagingClusters',
                ['EcalBarrelImagingClustersWithoutShapes', 'EcalBarrelImagingClusterLinksWithoutShapes'],
                ['EcalBarrelImagingClusters', 'EcalBarrelImagingClusterLinks', 'EcalBarrelImagingClusterAssociations'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.EnergyPositionClusterMerger, 'EcalBarrelClustersWithoutShapes',
                ['EcalBarrelScFiClusters', 'EcalBarrelScFiClusterAssociations', 'EcalBarrelImagingClusters', 'EcalBarrelImagingClusterAssociations'],
                ['EcalBarrelClustersWithoutShapes', 'EcalBarrelClusterLinksWithoutShapes', 'EcalBarrelClusterAssociationsWithoutShapes'])
        app.add(T.CalorimeterClusterShape, 'EcalBarrelClusters',
                ['EcalBarrelClustersWithoutShapes', 'EcalBarrelClusterLinksWithoutShapes'],
                ['EcalBarrelClusters', 'EcalBarrelClusterLinks', 'EcalBarrelClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.TruthEnergyPositionClusterMerger, 'EcalBarrelTruthClustersWithoutShapes',
                ['MCParticles', 'EcalBarrelScFiClusters', 'EcalBarrelScFiClusterAssociations', 'EcalBarrelImagingClusters', 'EcalBarrelImagingClusterAssociations'],
                ['EcalBarrelTruthClustersWithoutShapes', 'EcalBarrelTruthClusterLinksWithoutShapes', 'EcalBarrelTruthClusterAssociationsWithoutShapes'])
        app.add(T.CalorimeterClusterShape, 'EcalBarrelTruthClusters',
                ['EcalBarrelTruthClustersWithoutShapes', 'EcalBarrelTruthClusterLinksWithoutShapes'],
                ['EcalBarrelTruthClusters', 'EcalBarrelTruthClusterLinks', 'EcalBarrelTruthClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                logWeightBase=6.2)
