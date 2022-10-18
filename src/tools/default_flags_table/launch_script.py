desc = """

THIS IS A TEMPLATE

This script is a simple demonstration of how to run eicrecon with available digitization/reconstruction flag
Currently this script is created manually (TODO automatic generation) so if flags here is outdated, please file an issue

    python3 full_flags_run.py input_file.edm4hep.root output_name_no_ext
    
Script should successfully run and create files:

    output_name_no_ext.edm4eic.root    # with output flat tree
    output_name_no_ext.ana.root        # with histograms and other things filled by monitoring plugins
    
One can add -n/--nevents file with the number of events to process    

"""

import subprocess
from datetime import datetime
import argparse


parser = argparse.ArgumentParser(description=desc)
parser.add_argument('input_file', help="Input file name")
parser.add_argument('output_base_name', help="Output files names (no file extensions here)")
parser.add_argument('-n', '--nevents', default="0", help="Number of events to process")
args = parser.parse_args()

run_command = [
    f"eicrecon",
    f"-Pplugins=dump_flags",
    f"-Pdump_flags:python=all_flags_dump_from_run.py",
    f"-Pjana:debug_plugin_loading=1",
    f"-Pjana:nevents={args.nevents}",
    f"-Pacts:MaterialMap=calibrations/materials-map.cbor",
    f"-Ppodio:output_file={args.output_base_name}.tree.edm4eic.root",
    f"-Phistsfile={args.output_base_name}.ana.root",
    f"{args.input_file}",
]

# (!) NEXT LINE WILL BE REPLACED BY TEMPLATE #
reco_parameters_flags = [
"-PBEMC:EcalBarrelRawHits:capacityADC=16384",     # *
"-PBEMC:EcalBarrelRawHits:dynamicRangeADC=20000.0",     # *
"-PBEMC:EcalBarrelRawHits:energyResolutions=0.0,0.02,0.0",     # *
"-PBEMC:EcalBarrelRawHits:geoServiceName=ActsGeometryProvider",
"-PBEMC:EcalBarrelRawHits:pedestalMean=100",     # *
"-PBEMC:EcalBarrelRawHits:pedestalSigma=1",     # *
"-PBEMC:EcalBarrelRawHits:resolutionTDC=1e-11",
"-PBEMC:EcalBarrelRawHits:scaleResponse=1",
"-PBEMC:EcalBarrelRawHits:timeResolution=0",
"-PBEMC:EcalBarrelRecHits:capacityADC=16384",     # *
"-PBEMC:EcalBarrelRecHits:dynamicRangeADC=20000.0",     # *
"-PBEMC:EcalBarrelRecHits:pedestalMean=100",     # *
"-PBEMC:EcalBarrelRecHits:pedestalSigma=1",     # *
"-PBEMC:EcalBarrelRecHits:resolutionTDC=1e-11",
"-PBEMC:EcalBarrelRecHits:samplingFraction=0.10856976476514045",     # *
"-PBEMC:EcalBarrelRecHits:thresholdFactor=3",     # *
"-PBEMC:EcalBarrelRecHits:thresholdValue=3",     # *
"-PBEMC:EcalBarrelIslandProtoClusters:dimScaledLocalDistXY=1.8,1.8",
"-PBEMC:EcalBarrelIslandProtoClusters:minClusterCenterEdep=30.0",     # *
"-PBEMC:EcalBarrelIslandProtoClusters:minClusterHitEdep=1.0",     # *
"-PBEMC:EcalBarrelIslandProtoClusters:sectorDist=50.0",     # *
"-PBEMC:EcalBarrelIslandProtoClusters:splitCluster=0",     # *
"-PBEMC:EcalBarrelClusters:depthCorrection=0",
"-PBEMC:EcalBarrelClusters:enableEtaBounds=1",     # *
"-PBEMC:EcalBarrelClusters:energyWeight=log",
"-PBEMC:EcalBarrelClusters:input_protoclust_tag=EcalBarrelIslandProtoClusters",
"-PBEMC:EcalBarrelClusters:input_simhit_tag=EcalBarrelHits",
"-PBEMC:EcalBarrelClusters:logWeightBase=6.2",     # *
"-PBEMC:EcalBarrelClusters:samplingFraction=0.10856976476514045",     # *
"-PBEMC:EcalBarrelTruthClusters:depthCorrection=0",
"-PBEMC:EcalBarrelTruthClusters:enableEtaBounds=1",     # *
"-PBEMC:EcalBarrelTruthClusters:energyWeight=log",
"-PBEMC:EcalBarrelTruthClusters:input_protoclust_tag=EcalBarrelTruthProtoClusters",
"-PBEMC:EcalBarrelTruthClusters:input_simhit_tag=EcalBarrelHits",
"-PBEMC:EcalBarrelTruthClusters:logWeightBase=6.2",     # *
"-PBEMC:EcalBarrelTruthClusters:samplingFraction=0.10856976476514045",     # *
"-PEEMC:EcalEndcapNRawHits:capacityADC=16384",     # *
"-PEEMC:EcalEndcapNRawHits:dynamicRangeADC=20000.0",     # *
"-PEEMC:EcalEndcapNRawHits:energyResolutions=0.0,0.02,0.0",     # *
"-PEEMC:EcalEndcapNRawHits:geoServiceName=ActsGeometryProvider",
"-PEEMC:EcalEndcapNRawHits:pedestalMean=100",     # *
"-PEEMC:EcalEndcapNRawHits:pedestalSigma=1",
"-PEEMC:EcalEndcapNRawHits:resolutionTDC=1e-11",
"-PEEMC:EcalEndcapNRawHits:scaleResponse=1",
"-PEEMC:EcalEndcapNRawHits:timeResolution=0",
"-PEEMC:EcalEndcapNRecHits:capacityADC=16384",     # *
"-PEEMC:EcalEndcapNRecHits:dynamicRangeADC=20000.0",     # *
"-PEEMC:EcalEndcapNRecHits:geoServiceName=geoServiceName",
"-PEEMC:EcalEndcapNRecHits:pedestalMean=100",     # *
"-PEEMC:EcalEndcapNRecHits:pedestalSigma=1",     # *
"-PEEMC:EcalEndcapNRecHits:readout=EcalEndcapNHits",
"-PEEMC:EcalEndcapNRecHits:resolutionTDC=1e-11",
"-PEEMC:EcalEndcapNRecHits:samplingFraction=0.998",     # *
"-PEEMC:EcalEndcapNRecHits:sectorField=sector",
"-PEEMC:EcalEndcapNRecHits:thresholdFactor=4",     # *
"-PEEMC:EcalEndcapNRecHits:thresholdValue=3",     # *
"-PEEMC:EcalEndcapNIslandProtoClusters:dimScaledLocalDistXY=1.8,1.8",     # *
"-PEEMC:EcalEndcapNIslandProtoClusters:minClusterCenterEdep=1.0",     # *
"-PEEMC:EcalEndcapNIslandProtoClusters:minClusterHitEdep=30.0",     # *
"-PEEMC:EcalEndcapNIslandProtoClusters:sectorDist=50.0",     # *
"-PEEMC:EcalEndcapNIslandProtoClusters:splitCluster=0",     # *
"-PEEMC:EcalEndcapNTruthClusters:depthCorrection=0",
"-PEEMC:EcalEndcapNTruthClusters:enableEtaBounds=0",
"-PEEMC:EcalEndcapNTruthClusters:energyWeight=log",
"-PEEMC:EcalEndcapNTruthClusters:logWeightBase=4.6",     # *
"-PEEMC:EcalEndcapNTruthClusters:samplingFraction=0.03",     # *
"-PEEMC:EcalEndcapNClusters:depthCorrection=0",
"-PEEMC:EcalEndcapNClusters:dimScaledLocalDistXY=1.8,1.8",     # *
"-PEEMC:EcalEndcapNClusters:enableEtaBounds=0",
"-PEEMC:EcalEndcapNClusters:energyWeight=log",
"-PEEMC:EcalEndcapNClusters:logWeightBase=3.6",
"-PEEMC:EcalEndcapNClusters:minClusterCenterEdep=0.03",
"-PEEMC:EcalEndcapNClusters:minClusterHitEdep=0.001",
"-PEEMC:EcalEndcapNClusters:samplingFraction=1",
"-PEEMC:EcalEndcapNClusters:sectorDist=5",
"-PEEMC:EcalEndcapNClusters:splitCluster=0",
"-PEEMC:EcalEndcapPRawHits:capacityADC=16384",     # *
"-PEEMC:EcalEndcapPRawHits:dynamicRangeADC=3000.0",     # *
"-PEEMC:EcalEndcapPRawHits:energyResolutions=0.00316,0.0015,0.0",     # *
"-PEEMC:EcalEndcapPRawHits:fieldRefNumbers=1,1",     # *
"-PEEMC:EcalEndcapPRawHits:geoServiceName=ActsGeometryProvider",
"-PEEMC:EcalEndcapPRawHits:pedestalMean=100",     # *
"-PEEMC:EcalEndcapPRawHits:pedestalSigma=0.7",     # *
"-PEEMC:EcalEndcapPRawHits:resolutionTDC=1e-11",
"-PEEMC:EcalEndcapPRawHits:scaleResponse=1",
"-PEEMC:EcalEndcapPRawHits:timeResolution=0",
"-PEEMC:EcalEndcapPRecHits:capacityADC=16384",     # *
"-PEEMC:EcalEndcapPRecHits:dynamicRangeADC=3000.0",     # *
"-PEEMC:EcalEndcapPRecHits:geoServiceName=geoServiceName",
"-PEEMC:EcalEndcapPRecHits:layerField=layer",
"-PEEMC:EcalEndcapPRecHits:pedestalMean=100",     # *
"-PEEMC:EcalEndcapPRecHits:pedestalSigma=0.7",     # *
"-PEEMC:EcalEndcapPRecHits:readout=EcalEndcapPHits",
"-PEEMC:EcalEndcapPRecHits:resolutionTDC=1e-11",
"-PEEMC:EcalEndcapPRecHits:samplingFraction=0.03",     # *
"-PEEMC:EcalEndcapPRecHits:sectorField=sector",
"-PEEMC:EcalEndcapPRecHits:thresholdFactor=5.0",     # *
"-PEEMC:EcalEndcapPRecHits:thresholdValue=2",     # *
"-PEEMC:EcalEndcapPIslandProtoClusters:dimScaledLocalDistXY=1.5,1.5",
"-PEEMC:EcalEndcapPIslandProtoClusters:localDistXY=10,10",     # * [mm]
"-PEEMC:EcalEndcapPIslandProtoClusters:minClusterCenterEdep=10.0",     # *
"-PEEMC:EcalEndcapPIslandProtoClusters:minClusterHitEdep=0",
"-PEEMC:EcalEndcapPIslandProtoClusters:sectorDist=5",
"-PEEMC:EcalEndcapPIslandProtoClusters:splitCluster=0",
"-PEEMC:EcalEndcapPTruthClusters:depthCorrection=0",
"-PEEMC:EcalEndcapPTruthClusters:enableEtaBounds=1",     # *
"-PEEMC:EcalEndcapPTruthClusters:energyWeight=log",
"-PEEMC:EcalEndcapPTruthClusters:logWeightBase=6.2",     # *
"-PEEMC:EcalEndcapPTruthClusters:samplingFraction=1",
"-PEEMC:EcalEndcapPClusters:depthCorrection=0",
"-PEEMC:EcalEndcapPClusters:dimScaledLocalDistXY=1.8,1.8",
"-PEEMC:EcalEndcapPClusters:enableEtaBounds=0",
"-PEEMC:EcalEndcapPClusters:energyWeight=log",
"-PEEMC:EcalEndcapPClusters:logWeightBase=3.6",
"-PEEMC:EcalEndcapPClusters:minClusterCenterEdep=0.03",
"-PEEMC:EcalEndcapPClusters:minClusterHitEdep=0.001",
"-PEEMC:EcalEndcapPClusters:samplingFraction=1",
"-PEEMC:EcalEndcapPClusters:sectorDist=5",
"-PEEMC:EcalEndcapPClusters:splitCluster=0",
"-PHCAL:HcalBarrelRawHits:capacityADC=256",     # *
"-PHCAL:HcalBarrelRawHits:dynamicRangeADC=20.0",     # *
"-PHCAL:HcalBarrelRawHits:fieldRefNumbers=1,0",     # *
"-PHCAL:HcalBarrelRawHits:geoServiceName=ActsGeometryProvider",
"-PHCAL:HcalBarrelRawHits:pedestalMean=20",     # *
"-PHCAL:HcalBarrelRawHits:pedestalSigma=0.3",     # *
"-PHCAL:HcalBarrelRawHits:resolutionTDC=1e-11",
"-PHCAL:HcalBarrelRawHits:scaleResponse=1",
"-PHCAL:HcalBarrelRawHits:timeResolution=0",
"-PHCAL:HcalBarrelRecHits:capacityADC=256",     # *
"-PHCAL:HcalBarrelRecHits:dynamicRangeADC=20.0",     # *
"-PHCAL:HcalBarrelRecHits:geoServiceName=geoServiceName",
"-PHCAL:HcalBarrelRecHits:layerField=layer",
"-PHCAL:HcalBarrelRecHits:pedestalMean=20",     # *
"-PHCAL:HcalBarrelRecHits:pedestalSigma=0.3",     # *
"-PHCAL:HcalBarrelRecHits:readout=HcalBarrelHits",
"-PHCAL:HcalBarrelRecHits:resolutionTDC=1e-11",
"-PHCAL:HcalBarrelRecHits:samplingFraction=0.038",
"-PHCAL:HcalBarrelRecHits:sectorField=module",
"-PHCAL:HcalBarrelRecHits:thresholdFactor=5",     # *
"-PHCAL:HcalBarrelRecHits:thresholdValue=1",     # *
"-PHCAL:HcalBarrelClusters:depthCorrection=0",
"-PHCAL:HcalBarrelClusters:enableEtaBounds=0",
"-PHCAL:HcalBarrelClusters:energyWeight=log",
"-PHCAL:HcalBarrelClusters:input_protoclust_tag=HcalBarrelIslandProtoClusters",
"-PHCAL:HcalBarrelClusters:input_simhit_tag=HcalBarrelHits",
"-PHCAL:HcalBarrelClusters:logWeightBase=6.2",
"-PHCAL:HcalBarrelClusters:samplingFraction=1",
"-PHCAL:HcalBarrelIslandProtoClusters:dimScaledLocalDistXY=5,5",
"-PHCAL:HcalBarrelIslandProtoClusters:localDistXY=150,150",     # * [mm]
"-PHCAL:HcalBarrelIslandProtoClusters:minClusterCenterEdep=0.003",
"-PHCAL:HcalBarrelIslandProtoClusters:minClusterHitEdep=30.0",     # *
"-PHCAL:HcalBarrelIslandProtoClusters:sectorDist=5",
"-PHCAL:HcalBarrelIslandProtoClusters:splitCluster=0",     # *
"-PHCAL:HcalBarrelMergedHits:fields=layer,slice",
"-PHCAL:HcalBarrelMergedHits:input_tag=HcalBarrelRecHits",
"-PHCAL:HcalBarrelMergedHits:refs=1,0",
"-PHCAL:HcalBarrelTruthClusters:depthCorrection=0",
"-PHCAL:HcalBarrelTruthClusters:enableEtaBounds=0",
"-PHCAL:HcalBarrelTruthClusters:energyWeight=log",
"-PHCAL:HcalBarrelTruthClusters:input_protoclust_tag=HcalBarrelTruthProtoClusters",
"-PHCAL:HcalBarrelTruthClusters:input_simhit_tag=HcalBarrelHits",
"-PHCAL:HcalBarrelTruthClusters:logWeightBase=6.2",
"-PHCAL:HcalBarrelTruthClusters:samplingFraction=1",
"-PHCAL:HcalEndcapNRawHits:capacityADC=256",     # *
"-PHCAL:HcalEndcapNRawHits:dynamicRangeADC=20.0",     # *
"-PHCAL:HcalEndcapNRawHits:geoServiceName=ActsGeometryProvider",
"-PHCAL:HcalEndcapNRawHits:pedestalMean=20",     # *
"-PHCAL:HcalEndcapNRawHits:pedestalSigma=0.3",
"-PHCAL:HcalEndcapNRawHits:resolutionTDC=1e-11",
"-PHCAL:HcalEndcapNRawHits:scaleResponse=1",
"-PHCAL:HcalEndcapNRawHits:timeResolution=0",
"-PHCAL:HcalEndcapNRecHits:capacityADC=256",     # *
"-PHCAL:HcalEndcapNRecHits:dynamicRangeADC=20.0",     # *
"-PHCAL:HcalEndcapNRecHits:geoServiceName=geoServiceName",
"-PHCAL:HcalEndcapNRecHits:pedestalMean=20",     # *
"-PHCAL:HcalEndcapNRecHits:pedestalSigma=0.3",     # *
"-PHCAL:HcalEndcapNRecHits:readout=HcalEndcapNHits",
"-PHCAL:HcalEndcapNRecHits:resolutionTDC=1e-11",
"-PHCAL:HcalEndcapNRecHits:samplingFraction=0.998",
"-PHCAL:HcalEndcapNRecHits:sectorField=sector",
"-PHCAL:HcalEndcapNRecHits:thresholdFactor=4",
"-PHCAL:HcalEndcapNRecHits:thresholdValue=1",     # *
"-PHCAL:HcalEndcapNIslandProtoClusters:dimScaledLocalDistXY=1.5,1.5",
"-PHCAL:HcalEndcapNIslandProtoClusters:localDistXY=150,150",     # * [mm]
"-PHCAL:HcalEndcapNIslandProtoClusters:minClusterCenterEdep=30.0",     # *
"-PHCAL:HcalEndcapNIslandProtoClusters:minClusterHitEdep=0",
"-PHCAL:HcalEndcapNIslandProtoClusters:sectorDist=5",
"-PHCAL:HcalEndcapNIslandProtoClusters:splitCluster=1",
"-PHCAL:HcalEndcapNTruthClusters:depthCorrection=0",
"-PHCAL:HcalEndcapNTruthClusters:enableEtaBounds=0",
"-PHCAL:HcalEndcapNTruthClusters:energyWeight=log",
"-PHCAL:HcalEndcapNTruthClusters:input_protoclust_tag=HcalEndcapNTruthProtoClusters",
"-PHCAL:HcalEndcapNTruthClusters:input_simhit_tag=HcalEndcapNHits",
"-PHCAL:HcalEndcapNTruthClusters:logWeightBase=6.2",     # *
"-PHCAL:HcalEndcapNTruthClusters:samplingFraction=1",
"-PHCAL:HcalEndcapNClusters:depthCorrection=0",
"-PHCAL:HcalEndcapNClusters:enableEtaBounds=0",
"-PHCAL:HcalEndcapNClusters:energyWeight=log",
"-PHCAL:HcalEndcapNClusters:input_protoclust_tag=HcalEndcapNIslandProtoClusters",
"-PHCAL:HcalEndcapNClusters:input_simhit_tag=HcalEndcapNHits",
"-PHCAL:HcalEndcapNClusters:logWeightBase=6.2",     # *
"-PHCAL:HcalEndcapNClusters:samplingFraction=1",
"-PHCAL:HcalEndcapPRawHits:capacityADC=1024",     # *
"-PHCAL:HcalEndcapPRawHits:dynamicRangeADC=3600.0",     # *
"-PHCAL:HcalEndcapPRawHits:geoServiceName=ActsGeometryProvider",
"-PHCAL:HcalEndcapPRawHits:pedestalMean=20",     # *
"-PHCAL:HcalEndcapPRawHits:pedestalSigma=0.8",     # *
"-PHCAL:HcalEndcapPRawHits:resolutionTDC=1e-11",
"-PHCAL:HcalEndcapPRawHits:scaleResponse=1",
"-PHCAL:HcalEndcapPRawHits:timeResolution=0",
"-PHCAL:HcalEndcapPRecHits:capacityADC=1024",     # *
"-PHCAL:HcalEndcapPRecHits:dynamicRangeADC=3600.0",     # *
"-PHCAL:HcalEndcapPRecHits:geoServiceName=geoServiceName",
"-PHCAL:HcalEndcapPRecHits:pedestalMean=20",     # *
"-PHCAL:HcalEndcapPRecHits:pedestalSigma=0.8",     # *
"-PHCAL:HcalEndcapPRecHits:readout=HcalEndcapNHits",
"-PHCAL:HcalEndcapPRecHits:resolutionTDC=1e-11",
"-PHCAL:HcalEndcapPRecHits:samplingFraction=0.025",
"-PHCAL:HcalEndcapPRecHits:sectorField=sector",
"-PHCAL:HcalEndcapPRecHits:thresholdFactor=5",     # *
"-PHCAL:HcalEndcapPRecHits:thresholdValue=3",     # *
"-PHCAL:HcalEndcapPIslandProtoClusters:dimScaledLocalDistXY=1.5,1.5",
"-PHCAL:HcalEndcapPIslandProtoClusters:localDistXY=150,150",     # * [mm]
"-PHCAL:HcalEndcapPIslandProtoClusters:minClusterCenterEdep=30.0",     # *
"-PHCAL:HcalEndcapPIslandProtoClusters:minClusterHitEdep=0",
"-PHCAL:HcalEndcapPIslandProtoClusters:sectorDist=5",
"-PHCAL:HcalEndcapPIslandProtoClusters:splitCluster=1",
"-PHCAL:HcalEndcapPTruthClusters:depthCorrection=0",
"-PHCAL:HcalEndcapPTruthClusters:enableEtaBounds=0",
"-PHCAL:HcalEndcapPTruthClusters:energyWeight=log",
"-PHCAL:HcalEndcapPTruthClusters:input_protoclust_tag=HcalEndcapNTruthProtoClusters",
"-PHCAL:HcalEndcapPTruthClusters:input_simhit_tag=HcalEndcapNHits",
"-PHCAL:HcalEndcapPTruthClusters:logWeightBase=6.2",     # *
"-PHCAL:HcalEndcapPTruthClusters:samplingFraction=0.025",     # *
"-PHCAL:HcalEndcapPClusters:depthCorrection=0",
"-PHCAL:HcalEndcapPClusters:enableEtaBounds=0",
"-PHCAL:HcalEndcapPClusters:energyWeight=log",
"-PHCAL:HcalEndcapPClusters:input_protoclust_tag=HcalEndcapNIslandProtoClusters",
"-PHCAL:HcalEndcapPClusters:input_simhit_tag=HcalEndcapNHits",
"-PHCAL:HcalEndcapPClusters:logWeightBase=6.2",     # *
"-PHCAL:HcalEndcapPClusters:samplingFraction=0.025",     # *
"-PHCAL:HcalEndcapPInsertRawHits:capacityADC=32768",     # *
"-PHCAL:HcalEndcapPInsertRawHits:dynamicRangeADC=200.0",     # *
"-PHCAL:HcalEndcapPInsertRawHits:geoServiceName=ActsGeometryProvider",
"-PHCAL:HcalEndcapPInsertRawHits:pedestalMean=400",     # *
"-PHCAL:HcalEndcapPInsertRawHits:pedestalSigma=10",     # *
"-PHCAL:HcalEndcapPInsertRawHits:resolutionTDC=1e-11",
"-PHCAL:HcalEndcapPInsertRawHits:scaleResponse=1",
"-PHCAL:HcalEndcapPInsertRawHits:timeResolution=0",
"-PHCAL:HcalEndcapPInsertRecHits:capacityADC=32768",     # *
"-PHCAL:HcalEndcapPInsertRecHits:dynamicRangeADC=200.0",     # *
"-PHCAL:HcalEndcapPInsertRecHits:geoServiceName=geoServiceName",
"-PHCAL:HcalEndcapPInsertRecHits:pedestalMean=400",     # *
"-PHCAL:HcalEndcapPInsertRecHits:pedestalSigma=10",     # *
"-PHCAL:HcalEndcapPInsertRecHits:readout=HcalEndcapNHits",
"-PHCAL:HcalEndcapPInsertRecHits:resolutionTDC=1e-11",
"-PHCAL:HcalEndcapPInsertRecHits:samplingFraction=0.998",
"-PHCAL:HcalEndcapPInsertRecHits:sectorField=sector",
"-PHCAL:HcalEndcapPInsertRecHits:thresholdFactor=4",
"-PHCAL:HcalEndcapPInsertRecHits:thresholdValue=0",     # *
"-PHCAL:HcalEndcapPInsertIslandProtoClusters:dimScaledLocalDistXY=1.5,1.5",
"-PHCAL:HcalEndcapPInsertIslandProtoClusters:minClusterCenterEdep=0.03",
"-PHCAL:HcalEndcapPInsertIslandProtoClusters:minClusterHitEdep=0",
"-PHCAL:HcalEndcapPInsertIslandProtoClusters:sectorDist=5",
"-PHCAL:HcalEndcapPInsertIslandProtoClusters:splitCluster=1",
"-PHCAL:HcalEndcapPInsertTruthClusters:depthCorrection=0",
"-PHCAL:HcalEndcapPInsertTruthClusters:enableEtaBounds=0",
"-PHCAL:HcalEndcapPInsertTruthClusters:energyWeight=log",
"-PHCAL:HcalEndcapPInsertTruthClusters:input_protoclust_tag=HcalEndcapNTruthProtoClusters",
"-PHCAL:HcalEndcapPInsertTruthClusters:input_simhit_tag=HcalEndcapNHits",
"-PHCAL:HcalEndcapPInsertTruthClusters:logWeightBase=6.2",     # *
"-PHCAL:HcalEndcapPInsertTruthClusters:samplingFraction=1",
"-PHCAL:HcalEndcapPInsertClusters:depthCorrection=0",
"-PHCAL:HcalEndcapPInsertClusters:enableEtaBounds=0",
"-PHCAL:HcalEndcapPInsertClusters:energyWeight=log",
"-PHCAL:HcalEndcapPInsertClusters:input_protoclust_tag=HcalEndcapNIslandProtoClusters",
"-PHCAL:HcalEndcapPInsertClusters:input_simhit_tag=HcalEndcapNHits",
"-PHCAL:HcalEndcapPInsertClusters:logWeightBase=6.2",     # *
"-PHCAL:HcalEndcapPInsertClusters:samplingFraction=1",
"-PZDC:ZDCEcalRawHits:capacityADC=8096",
"-PZDC:ZDCEcalRawHits:dynamicRangeADC=0.1",
"-PZDC:ZDCEcalRawHits:geoServiceName=ActsGeometryProvider",
"-PZDC:ZDCEcalRawHits:pedestalMean=400",
"-PZDC:ZDCEcalRawHits:pedestalSigma=3.2",
"-PZDC:ZDCEcalRawHits:resolutionTDC=1e-11",
"-PZDC:ZDCEcalRawHits:scaleResponse=1",
"-PZDC:ZDCEcalRawHits:timeResolution=0",
"-PZDC:ZDCEcalRecHits:capacityADC=8096",
"-PZDC:ZDCEcalRecHits:dynamicRangeADC=0.1",
"-PZDC:ZDCEcalRecHits:geoServiceName=geoServiceName",
"-PZDC:ZDCEcalRecHits:pedestalMean=400",
"-PZDC:ZDCEcalRecHits:pedestalSigma=3.2",
"-PZDC:ZDCEcalRecHits:readout=ZDCEcalHits",
"-PZDC:ZDCEcalRecHits:resolutionTDC=1e-11",
"-PZDC:ZDCEcalRecHits:samplingFraction=1",     # *
"-PZDC:ZDCEcalRecHits:sectorField=sector",
"-PZDC:ZDCEcalRecHits:thresholdFactor=4",
"-PZDC:ZDCEcalRecHits:thresholdValue=0",
"-PZDC:ZDCEcalClusters:depthCorrection=0",
"-PZDC:ZDCEcalClusters:enableEtaBounds=0",
"-PZDC:ZDCEcalClusters:energyWeight=log",
"-PZDC:ZDCEcalClusters:input_protoclust_tag=ZDCEcalIslandProtoClusters",
"-PZDC:ZDCEcalClusters:input_simhit_tag=ZDCEcalHits",
"-PZDC:ZDCEcalClusters:logWeightBase=6.2",     # *
"-PZDC:ZDCEcalClusters:samplingFraction=1",     # *
"-PZDC:ZDCEcalIslandProtoClusters:dimScaledLocalDistXY=5,5",
"-PZDC:ZDCEcalIslandProtoClusters:localDistXY=50,50",     # * [mm]
"-PZDC:ZDCEcalIslandProtoClusters:minClusterCenterEdep=3.0",     # *
"-PZDC:ZDCEcalIslandProtoClusters:minClusterHitEdep=0.1",     # *
"-PZDC:ZDCEcalIslandProtoClusters:sectorDist=5",
"-PZDC:ZDCEcalIslandProtoClusters:splitCluster=1",
"-PZDC:ZDCEcalTruthClusters:depthCorrection=0",
"-PZDC:ZDCEcalTruthClusters:enableEtaBounds=0",
"-PZDC:ZDCEcalTruthClusters:energyWeight=log",
"-PZDC:ZDCEcalTruthClusters:input_protoclust_tag=ZDCEcalTruthProtoClusters",
"-PZDC:ZDCEcalTruthClusters:input_simhit_tag=ZDCEcalHits",
"-PZDC:ZDCEcalTruthClusters:logWeightBase=3.6",     # *
"-PZDC:ZDCEcalTruthClusters:samplingFraction=1",     # *
"-PBTRK:BarrelTrackerHit:TimeResolution=10",     # threshold
"-PBTRK:BarrelTrackerRawHit:Threshold=0",     # EDep threshold for hits to pass through, [GeV]
"-PBTRK:BarrelTrackerRawHit:TimeResolution=8",     # * Time resolution gauss smearing [ns]
"-PBVTX:BarrelVertexHit:TimeResolution=10",     # threshold
"-PBVTX:BarrelVertexRawHit:Threshold=0",     # EDep threshold for hits to pass through, [GeV]
"-PBVTX:BarrelVertexRawHit:TimeResolution=8",     # * Time resolution gauss smearing [ns]
"-PECTRK:EndcapTrackerHit:TimeResolution=10",     # threshold
"-PECTRK:EndcapTrackerRawHit:Threshold=0",     # EDep threshold for hits to pass through, [GeV]
"-PECTRK:EndcapTrackerRawHit:TimeResolution=8",     # * Time resolution gauss smearing [ns]
"-PMPGD:MPGDTrackerHit:TimeResolution=10",     # threshold
"-PMPGD:MPGDTrackerRawHit:Threshold=0",     # EDep threshold for hits to pass through, [GeV]
"-PMPGD:MPGDTrackerRawHit:TimeResolution=8",     # * Time resolution gauss smearing [ns]
"-PECTOF:TOFEndcapTrackerHit:TimeResolution=0.026",     # threshold
"-PECTOF:TOFEndcapRawHit:Threshold=0",     # * EDep threshold for hits to pass through, [GeV]
"-PECTOF:TOFEndcapRawHit:TimeResolution=0.026",     # * Time resolution gauss smearing [ns]
"-PBTOF:TOFBarrelTrackerHit:TimeResolution=0.026",     # * threshold
"-PBTOF:TOFBarrelRawHit:Threshold=0",     # * EDep threshold for hits to pass through, [GeV]
"-PBTOF:TOFBarrelRawHit:TimeResolution=0.026",     # * Time resolution gauss smearing [ns]
"-PReco:GeneratedParticles:MomentumSmearing=0",     # Gaussian momentum smearing value
]


run_command.extend(reco_parameters_flags)

# RUN EICrecon
start_time = datetime.now()
subprocess.run(" ".join(run_command), shell=True, check=True)
end_time = datetime.now()

# Print execution time
print("Start date and time : {}".format(start_time.strftime('%Y-%m-%d %H:%M:%S')))
print("End date and time   : {}".format(end_time.strftime('%Y-%m-%d %H:%M:%S')))
print("Execution real time : {}".format(end_time - start_time))


