desc = """
This script is a simple demonstration of how to run eicrecon with available digitization/reconstruction flag
Currently this script is created manually (TODO automatic generation) so if flags here is outdated, please file an issue

    python3 full_flags_run.py input_file.edm4hep.root output_name_no_ext

Script should successfully run and create files:

    output_name_no_ext.edm4eic.root    # with output flat tree
    output_name_no_ext.ana.root        # with histograms and other things filled by monitoring plugins

"""

import argparse

parser = argparse.ArgumentParser(description=desc)
parser.add_argument('input_file', help="Input file name")
parser.add_argument('output_base_name', help="Output files names (no file extensions here)")
args = parser.parse_args()


flags = [
    # "BEMC:capacityADC="              # m_capADC          8096
    # "BEMC:dynamicRangeADC="          # m_dyRangeADC      100. * MeV
    # "BEMC:pedestalMean="             # m_pedMeanADC      400
    # "BEMC:pedestalSigma="            # m_pedSigmaADC     3.2
    # "BEMC:resolutionTDC="            # m_resolutionTDC   10 * dd4hep::picosecond
    # "BEMC:thresholdFactor="          # m_thresholdFactor 4
    # "BEMC:thresholdValue="           # m_thresholdValue  0
    # "BEMC:samplingFraction="         # m_sampFrac        0.998

    # "BEMC:EcalBarrelClusters:samplingFraction=",       # m_sampFrac            1
    # "BEMC:EcalBarrelClusters:logWeightBase=",          # m_logWeightBase       3.6
    # "BEMC:EcalBarrelClusters:depthCorrection=",        # m_depthCorrection     0
    # "BEMC:EcalBarrelClusters:energyWeight=",           # m_energyWeight        log
    # "BEMC:EcalBarrelClusters:moduleDimZName=",         # m_moduleDimZName      ""
    # "BEMC:EcalBarrelClusters:enableEtaBounds=",        # m_enableEtaBounds     FALSE

    # "BEMC:EcalBarrelTruthClusters:samplingFraction=",  # m_sampFrac            1
    # "BEMC:EcalBarrelTruthClusters:logWeightBase=",     # m_logWeightBase       3.6
    # "BEMC:EcalBarrelTruthClusters:depthCorrection=",   # m_depthCorrection     0
    # "BEMC:EcalBarrelTruthClusters:energyWeight=",      # m_energyWeight        log
    # "BEMC:EcalBarrelTruthClusters:moduleDimZName=",    # m_moduleDimZName      ""
    # "BEMC:EcalBarrelTruthClusters:enableEtaBounds=",   # m_enableEtaBounds     FALSE

    # "BEMC:splitCluster=",               # m_splitCluster           FALSE
    # "BEMC:minClusterHitEdep=",          # m_minClusterHitEdep      1.0 * MeV
    # "BEMC:minClusterCenterEdep=",       # m_minClusterCenterEdep   30.0 * MeV
    # "BEMC:sectorDist=",                 # m_sectorDist             5.0 * cm
    # "BEMC:localDistXY=",                # u_localDistXY            {}
    # "BEMC:localDistXZ=",                # u_localDistXZ            {}
    # "BEMC:localDistYZ=",                # u_localDistYZ            {}
    # "BEMC:globalDistRPhi=",             # u_globalDistRPhi         {}
    # "BEMC:globalDistEtaPhi=",           # u_globalDistEtaPhi       {}
    # "BEMC:dimScaledLocalDistXY=",       # u_dimScaledLocalDistXY   {1.8,1.8}


    # "BEMC:energyResolutions=",          # u_eRes                   0.0 * MeV
    # "BEMC:timeResolution=",             # m_tRes                   0.0 * ns
    # "BEMC:capacityADC=",                # m_capADC                 8096
    # "BEMC:dynamicRangeADC=",            # m_dyRangeADC             100 * MeV
    # "BEMC:pedestalMean=",               # m_pedMeanADC             400
    # "BEMC:pedestalSigma=",              # m_pedSigmaADC            3.2
    # "BEMC:resolutionTDC=",              # m_resolutionTDC          10 * picosecond
    # "BEMC:scaleResponse=",              # m_corrMeanScale          1
    # "BEMC:signalSumFields=",            # u_fields                 {}
    # "BEMC:fieldRefNumbers=",            # u_refs                   {}
    # "BEMC:geoServiceName=",             # m_geoSvcName             ActsGeometryProvider
    # "BEMC:readoutClass=",               # m_readout                ""

    # "EEMC:EcalEndcapNRecHits:capacityADC=",         #  m_capADC              8096
    # "EEMC:EcalEndcapNRecHits:dynamicRangeADC=",     #  m_dyRangeADC          100*MeV
    # "EEMC:EcalEndcapNRecHits:pedestalMean=",        #  m_pedMeanADC          400
    # "EEMC:EcalEndcapNRecHits:pedestalSigma=",       #  m_pedSigmaADC         3.2
    # "EEMC:EcalEndcapNRecHits:resolutionTDC=",       #  m_resolutionTDC       10
    # "EEMC:EcalEndcapNRecHits:thresholdFactor=",     #  m_thresholdFactor     4
    # "EEMC:EcalEndcapNRecHits:thresholdValue=",      #  m_thresholdValue      0
    # "EEMC:EcalEndcapNRecHits:samplingFraction=",    #  m_sampFrac            0.998

    # "EEMC:EcalEndcapPRecHits:capacityADC=",         # m_capADC              8096
    # "EEMC:EcalEndcapPRecHits:dynamicRangeADC=",     # m_dyRangeADC          100*MeV
    # "EEMC:EcalEndcapPRecHits:pedestalMean=",        # m_pedMeanADC          400
    # "EEMC:EcalEndcapPRecHits:pedestalSigma=",       # m_pedSigmaADC         3.2
    # "EEMC:EcalEndcapPRecHits:resolutionTDC=",       # m_resolutionTDC       10
    # "EEMC:EcalEndcapPRecHits:thresholdFactor=",     # m_thresholdFactor     4
    # "EEMC:EcalEndcapPRecHits:thresholdValue=",      # m_thresholdValue      0
    # "EEMC:EcalEndcapPRecHits:samplingFraction=",    # m_sampFrac            0.998

    # "EEMC:EcalEndcapNClusters:samplingFraction="    # m_sampFrac            1
    # "EEMC:EcalEndcapNClusters:logWeightBase="       # m_logWeightBase       3.6
    # "EEMC:EcalEndcapNClusters:depthCorrection="     # m_depthCorrection     0
    # "EEMC:EcalEndcapNClusters:energyWeight="        # m_energyWeight        log
    # "EEMC:EcalEndcapNClusters:moduleDimZName="      # m_moduleDimZName      ""
    # "EEMC:EcalEndcapNClusters:enableEtaBounds="     # m_enableEtaBounds     FALSE

    # "EEMC:EcalEndcapPClusters:samplingFraction"     # m_sampFrac            1
    # "EEMC:EcalEndcapPClusters:logWeightBase"        # m_logWeightBase       3.6
    # "EEMC:EcalEndcapPClusters:depthCorrection"      # m_depthCorrection     0
    # "EEMC:EcalEndcapPClusters:energyWeight"         # m_energyWeight        log
    # "EEMC:EcalEndcapPClusters:moduleDimZName"       # m_moduleDimZName      ""
    # "EEMC:EcalEndcapPClusters:enableEtaBounds"      # m_enableEtaBounds     FALSE


    # "EEMC:EcalEndcapNClusters:splitCluster" m_splitCluster FALSE
    # "EEMC:EcalEndcapNClusters:minClusterHitEdep" m_minClusterHitEdep 1.0 * MeV
    # "EEMC:EcalEndcapNClusters:minClusterCenterEdep" m_minClusterCenterEdep 30.0 * MeV
    # "EEMC:EcalEndcapNClusters:sectorDist" m_sectorDist 5.0 * cm
    # "EEMC:EcalEndcapNClusters:localDistXY" u_localDistXY {}
    # "EEMC:EcalEndcapNClusters:localDistXZ" u_localDistXZ {}
    # "EEMC:EcalEndcapNClusters:localDistYZ" u_localDistYZ {}
    # "EEMC:EcalEndcapNClusters:globalDistRPhi" u_globalDistRPhi {}
    # "EEMC:EcalEndcapNClusters:globalDistEtaPhi" u_globalDistEtaPhi {}
    # "EEMC:EcalEndcapNClusters:dimScaledLocalDistXY" u_dimScaledLocalDistXY {1.8,1.8}

    # "EEMC:EcalEndcapPClusters:splitCluster" m_splitCluster FALSE
    # "EEMC:EcalEndcapPClusters:minClusterHitEdep" m_minClusterHitEdep 1.0 * MeV
    # "EEMC:EcalEndcapPClusters:minClusterCenterEdep" m_minClusterCenterEdep 30.0 * MeV
    # "EEMC:EcalEndcapPClusters:sectorDist" m_sectorDist 5.0 * cm
    # "EEMC:EcalEndcapPClusters:localDistXY" u_localDistXY {}
    # "EEMC:EcalEndcapPClusters:localDistXZ" u_localDistXZ {}
    # "EEMC:EcalEndcapPClusters:localDistYZ" u_localDistYZ {}
    # "EEMC:EcalEndcapPClusters:globalDistRPhi" u_globalDistRPhi {}
    # "EEMC:EcalEndcapPClusters:globalDistEtaPhi" u_globalDistEtaPhi {}
    # "EEMC:EcalEndcapPClusters:dimScaledLocalDistXY" u_dimScaledLocalDistXY {1.8,1.8}

    # "EEMC:EcalEndcapNRawHits:energyResolutions",     # u_eRes            0.0 * MeV
    # "EEMC:EcalEndcapNRawHits:timeResolution",        # m_tRes            0.0 * ns
    # "EEMC:EcalEndcapNRawHits:capacityADC",           # m_capADC          8096
    # "EEMC:EcalEndcapNRawHits:dynamicRangeADC",       # m_dyRangeADC      100 * MeV
    # "EEMC:EcalEndcapNRawHits:pedestalMean",          # m_pedMeanADC      400
    # "EEMC:EcalEndcapNRawHits:pedestalSigma",         # m_pedSigmaADC     3.2
    # "EEMC:EcalEndcapNRawHits:resolutionTDC",         # m_resolutionTDC   10 * picosecond
    # "EEMC:EcalEndcapNRawHits:scaleResponse",         # m_corrMeanScale   1
    # "EEMC:EcalEndcapNRawHits:signalSumFields",       # u_fields          {}
    # "EEMC:EcalEndcapNRawHits:fieldRefNumbers",       # u_refs            {}
    # "EEMC:EcalEndcapNRawHits:geoServiceName",        # m_geoSvcName      ActsGeometryProvider
    # "EEMC:EcalEndcapNRawHits:readoutClass",          # m_readout         ""


    # "EEMC:EcalEndcapPRawHits:energyResolutions=",    # u_eRes            0.0 * MeV
    # "EEMC:EcalEndcapPRawHits:timeResolution=",       # m_tRes            0.0 * ns
    # "EEMC:EcalEndcapPRawHits:capacityADC=",          # m_capADC          8096
    # "EEMC:EcalEndcapPRawHits:dynamicRangeADC=",      # m_dyRangeADC      100 * MeV
    # "EEMC:EcalEndcapPRawHits:pedestalMean=",         # m_pedMeanADC      400
    # "EEMC:EcalEndcapPRawHits:pedestalSigma=",        # m_pedSigmaADC     3.2
    # "EEMC:EcalEndcapPRawHits:resolutionTDC=",        # m_resolutionTDC   10 * picosecond
    # "EEMC:EcalEndcapPRawHits:scaleResponse=",        # m_corrMeanScale   1
    # "EEMC:EcalEndcapPRawHits:signalSumFields=",      # u_fields          {"layer","slice"}
    # "EEMC:EcalEndcapPRawHits:fieldRefNumbers=",      # u_refs            {1,0}
    # "EEMC:EcalEndcapPRawHits:geoServiceName=",       # m_geoSvcName      ActsGeometryProvider
    # "EEMC:EcalEndcapPRawHits:readoutClass=",         # m_readout         ""



    # "HCAL:HcalBarrelMergedHits:input_tag",   # m_input_tag   HcalBarrelRecHits
    # "HCAL:HcalBarrelMergedHits:fields",      # u_fields      {"layer", "slice"}
    # "HCAL:HcalBarrelMergedHits:refs",        # u_refs        {1, 0}


    # "HCAL:HcalBarrelRecHits:capacityADC=",         # m_capADC            8096
    # "HCAL:HcalBarrelRecHits:dynamicRangeADC=",     # m_dyRangeADC        100 * MeV
    # "HCAL:HcalBarrelRecHits:pedestalMean=",        # m_pedMeanADC        400
    # "HCAL:HcalBarrelRecHits:pedestalSigma=",       # m_pedSigmaADC       3.2
    # "HCAL:HcalBarrelRecHits:resolutionTDC=",       # m_resolutionTDC     10 dd4hep::picosecond
    # "HCAL:HcalBarrelRecHits:thresholdFactor=",     # m_thresholdFactor   5
    # "HCAL:HcalBarrelRecHits:thresholdValue=",      # m_thresholdValue    0
    # "HCAL:HcalBarrelRecHits:samplingFraction=",    # m_sampFrac          0.038


    # "HCAL:HcalBarrelMergedHits:input_tag=",      m_input_tag  HcalEndcapNRecHits
    # "HCAL:HcalBarrelMergedHits:fields=",         u_fields     {"layer", "slice"}
    # "HCAL:HcalBarrelMergedHits:refs=",           u_refs       {1, 0}


    # "HCAL:capacityADC="              m_capADC 8096
    # "HCAL:dynamicRangeADC="          m_dyRangeADC 100. * MeV
    # "HCAL:pedestalMean="             m_pedMeanADC 400
    # "HCAL:pedestalSigma="            m_pedSigmaADC 3.2
    # "HCAL:resolutionTDC="            m_resolutionTDC 10 * dd4hep::picosecond
    # "HCAL:thresholdFactor="          m_thresholdFactor 4
    # "HCAL:thresholdValue="           m_thresholdValue 0
    # "HCAL:samplingFraction="         m_sampFrac 0.998


    #HCAL:HcalBarrelMergedHits:input_tag" m_input_tag HcalEndcapPInsertHits
    #HCAL:HcalBarrelMergedHits:fields" u_fields {"layer", "slice"}
    #HCAL:HcalBarrelMergedHits:refs" u_refs {1, 0}


    #HCAL:capacityADC" m_capADC 8096
    #HCAL:dynamicRangeADC" m_dyRangeADC 100 * MeV
    #HCAL:pedestalMean" m_pedMeanADC 400
    #HCAL:pedestalSigma" m_pedSigmaADC 3.2
    #HCAL:resolutionTDC" m_resolutionTDC 10 * dd4hep::picosecond
    #HCAL:thresholdFactor" m_thresholdFactor 4
    #HCAL:thresholdValue" m_thresholdValue 0
    #HCAL:samplingFraction" m_sampFrac 0.998

    #HCAL:HcalBarrelMergedHits:input_tag" m_input_tag HcalEndcapPHits
    #HCAL:HcalBarrelMergedHits:fields" u_fields {"layer", "slice"}
    #HCAL:HcalBarrelMergedHits:refs" u_refs {1, 0}

    #HCAL:capacityADC" m_capADC 8096
    #HCAL:dynamicRangeADC" m_dyRangeADC 100 * MeV
    #HCAL:pedestalMean" m_pedMeanADC 400
    #HCAL:pedestalSigma" m_pedSigmaADC 3.2
    #HCAL:resolutionTDC" m_resolutionTDC 10 * dd4hep::picosecond
    #HCAL:thresholdFactor" m_thresholdFactor 4
    #HCAL:thresholdValue" m_thresholdValue 0
    #HCAL:samplingFraction" m_sampFrac 0.998

    # "HCAL:HcalBarrelClusters:samplingFraction=",      # m_sampFrac           1
    # "HCAL:HcalBarrelClusters:logWeightBase=",         # m_logWeightBase      3.6
    # "HCAL:HcalBarrelClusters:depthCorrection=",       # m_depthCorrection    0
    # "HCAL:HcalBarrelClusters:energyWeight=",          # m_energyWeight       log
    # "HCAL:HcalBarrelClusters:moduleDimZName=",        # m_moduleDimZName     ""
    # "HCAL:HcalBarrelClusters:enableEtaBounds=",       # m_enableEtaBounds    FALSE

    # "HCAL:HcalBarrelTruthClusters:samplingFraction=",  # m_sampFrac          1
    # "HCAL:HcalBarrelTruthClusters:logWeightBase=",     # m_logWeightBase     log
    # "HCAL:HcalBarrelTruthClusters:depthCorrection=",   # m_depthCorrection   0
    # "HCAL:HcalBarrelTruthClusters:energyWeight=",      # m_energyWeight      log
    # "HCAL:HcalBarrelTruthClusters:moduleDimZName=",    # m_moduleDimZName    ""
    # "HCAL:HcalBarrelTruthClusters:enableEtaBounds=",   # m_enableEtaBounds   FALSE

    # "HCAL:HcalEndcapNClusters:samplingFraction=",      # m_sampFrac          1
    # "HCAL:HcalEndcapNClusters:logWeightBase=",         # m_logWeightBase     6.2
    # "HCAL:HcalEndcapNClusters:depthCorrection=",       # m_depthCorrection   0
    # "HCAL:HcalEndcapNClusters:energyWeight=",          # m_energyWeight      log
    # "HCAL:HcalEndcapNClusters:moduleDimZName=",        # m_moduleDimZName    ""
    # "HCAL:HcalEndcapNClusters:enableEtaBounds=",       # m_enableEtaBounds   FALSE

    # "HCAL:HcalEndcapNTruthClusters:samplingFraction=",  # m_sampFrac          1
    # "HCAL:HcalEndcapNTruthClusters:logWeightBase=",     # m_logWeightBase     3.6
    # "HCAL:HcalEndcapNTruthClusters:depthCorrection=",   # m_depthCorrection   0
    # "HCAL:HcalEndcapNTruthClusters:energyWeight=",      # m_energyWeight      log
    # "HCAL:HcalEndcapNTruthClusters:moduleDimZName=",    # m_moduleDimZName    ""
    # "HCAL:HcalEndcapNTruthClusters:enableEtaBounds=",   # m_enableEtaBounds   FALSE


    # "HCAL:HcalEndcapPClusters:samplingFraction=",       # m_sampFrac            1
    # "HCAL:HcalEndcapPClusters:logWeightBase=",          # m_logWeightBase       6.2
    # "HCAL:HcalEndcapPClusters:depthCorrection=",        # m_depthCorrection     0
    # "HCAL:HcalEndcapPClusters:energyWeight=",           # m_energyWeight        log
    # "HCAL:HcalEndcapPClusters:moduleDimZName=",         # m_moduleDimZName      ""
    # "HCAL:HcalEndcapPClusters:enableEtaBounds=",        # m_enableEtaBounds     FALSE

    # "HCAL:HcalEndcapPInsertClusters:samplingFraction=",                 # m_sampFrac              1
    # "HCAL:HcalEndcapPInsertClusters:logWeightBase=",                    # m_logWeightBase         6.2
    # "HCAL:HcalEndcapPInsertClusters:depthCorrection=",                  # m_depthCorrection       0
    # "HCAL:HcalEndcapPInsertClusters:energyWeight=",                     # m_energyWeight          log
    # "HCAL:HcalEndcapPInsertClusters:moduleDimZName=",                   # m_moduleDimZName        ""
    # "HCAL:HcalEndcapPInsertClusters:enableEtaBounds=",                  # m_enableEtaBounds       FALSE
    # "HCAL:HcalEndcapPInsertTruthClusters:samplingFraction=",            # m_sampFrac              1
    # "HCAL:HcalEndcapPInsertTruthClusters:logWeightBase=",               # m_logWeightBase         3.6
    # "HCAL:HcalEndcapPInsertTruthClusters:depthCorrection=",             # m_depthCorrection       0
    # "HCAL:HcalEndcapPInsertTruthClusters:energyWeight=",                # m_energyWeight          log
    # "HCAL:HcalEndcapPInsertTruthClusters:moduleDimZName=",              # m_moduleDimZName        ""
    # "HCAL:HcalEndcapPInsertTruthClusters:enableEtaBounds=",             # m_enableEtaBounds       FALSE
    # "HCAL:HcalEndcapPTruthClusters:samplingFraction=",                  # m_sampFrac              1
    # "HCAL:HcalEndcapPTruthClusters:logWeightBase=",                     # m_logWeightBase         3.6
    # "HCAL:HcalEndcapPTruthClusters:depthCorrection=",                   # m_depthCorrection       0
    # "HCAL:HcalEndcapPTruthClusters:energyWeight=",                      # m_energyWeight          log
    # "HCAL:HcalEndcapPTruthClusters:moduleDimZName=",                    # m_moduleDimZName        ""
    # "HCAL:HcalEndcapPTruthClusters:enableEtaBounds=",                   # m_enableEtaBounds       FALSE

    # "HCAL:HcalBarrelIslandProtoClusters:splitCluster=",                 # m_splitCluster          TRUE
    # "HCAL:HcalBarrelIslandProtoClusters:minClusterHitEdep=",            # m_minClusterHitEdep     0.1 * MeV
    # "HCAL:HcalBarrelIslandProtoClusters:minClusterCenterEdep=",         # m_minClusterCenterEdep  3.0 * MeV
    # "HCAL:HcalBarrelIslandProtoClusters:sectorDist=",                   # m_sectorDist            5.0 * cm
    # "HCAL:HcalBarrelIslandProtoClusters:localDistXY=",                  # u_localDistXY           {}
    # "HCAL:HcalBarrelIslandProtoClusters:localDistXZ=",                  # u_localDistXZ           {}
    # "HCAL:HcalBarrelIslandProtoClusters:localDistYZ=",                  # u_localDistYZ           {}
    # "HCAL:HcalBarrelIslandProtoClusters:globalDistRPhi=",               # u_globalDistRPhi        {}
    # "HCAL:HcalBarrelIslandProtoClusters:globalDistEtaPhi=",             # u_globalDistEtaPhi      {}
    # "HCAL:HcalBarrelIslandProtoClusters:dimScaledLocalDistXY=",         # u_dimScaledLocalDistXY  {50.0*dd4hep::mm, 50.0*dd4hep::mm}

    # "HCAL:HcalEndcapNIslandProtoClusters:splitCluster=",                # m_splitCluster          TRUE
    # "HCAL:HcalEndcapNIslandProtoClusters:minClusterHitEdep=",           # m_minClusterHitEdep     0.0 * MeV
    # "HCAL:HcalEndcapNIslandProtoClusters:minClusterCenterEdep=",        # m_minClusterCenterEdep  30.0 * MeV
    # "HCAL:HcalEndcapNIslandProtoClusters:sectorDist=",                  # m_sectorDist            5.0 * cm
    # "HCAL:HcalEndcapNIslandProtoClusters:localDistXY=",                 # u_localDistXY           {}
    # "HCAL:HcalEndcapNIslandProtoClusters:localDistXZ=",                 # u_localDistXZ           {}
    # "HCAL:HcalEndcapNIslandProtoClusters:localDistYZ=",                 # u_localDistYZ           {}
    # "HCAL:HcalEndcapNIslandProtoClusters:globalDistRPhi=",              # u_globalDistRPhi        {}
    # "HCAL:HcalEndcapNIslandProtoClusters:globalDistEtaPhi=",            # u_globalDistEtaPhi      {}

    # "HCAL:HcalEndcapNIslandProtoClusters:dimScaledLocalDistXY=",        # u_dimScaledLocalDistXY  {15.0*dd4hep::mm, 15.0*dd4hep::mm}
    # "HCAL:HcalEndcapPInsertIslandProtoClusters:splitCluster=",          # m_splitCluster          TRUE
    # "HCAL:HcalEndcapPInsertIslandProtoClusters:minClusterHitEdep=",     # m_minClusterHitEdep     0.0 * MeV
    # "HCAL:HcalEndcapPInsertIslandProtoClusters:minClusterCenterEdep=",  # m_minClusterCenterEdep  30.0 * MeV
    # "HCAL:HcalEndcapPInsertIslandProtoClusters:sectorDist=",            # m_sectorDist            5.0 * cm
    # "HCAL:HcalEndcapPInsertIslandProtoClusters:localDistXY=",           # u_localDistXY           {}
    # "HCAL:HcalEndcapPInsertIslandProtoClusters:localDistXZ=",           # u_localDistXZ           {}
    # "HCAL:HcalEndcapPInsertIslandProtoClusters:localDistYZ=",           # u_localDistYZ           {}
    # "HCAL:HcalEndcapPInsertIslandProtoClusters:globalDistRPhi=",        # u_globalDistRPhi        {}
    # "HCAL:HcalEndcapPInsertIslandProtoClusters:globalDistEtaPhi=",      # u_globalDistEtaPhi      {}

    # "HCAL:HcalEndcapPInsertIslandProtoClusters:dimScaledLocalDistXY=",  # u_dimScaledLocalDistXY  {15.0*dd4hep::mm, 15.0*dd4hep::mm}
    # "HCAL:HcalEndcapPIslandProtoClusters:splitCluster=",                # m_splitCluster          TRUE
    # "HCAL:HcalEndcapPIslandProtoClusters:minClusterHitEdep=",           # m_minClusterHitEdep     0.0 * MeV
    # "HCAL:HcalEndcapPIslandProtoClusters:minClusterCenterEdep=",        # m_minClusterCenterEdep  30.0 * MeV
    # "HCAL:HcalEndcapPIslandProtoClusters:sectorDist=",                  # m_sectorDist            5.0 * cm
    # "HCAL:HcalEndcapPIslandProtoClusters:localDistXY=",                 # u_localDistXY           {}
    # "HCAL:HcalEndcapPIslandProtoClusters:localDistXZ=",                 # u_localDistXZ           {}
    # "HCAL:HcalEndcapPIslandProtoClusters:localDistYZ=",                 # u_localDistYZ           {}
    # "HCAL:HcalEndcapPIslandProtoClusters:globalDistRPhi=",              # u_globalDistRPhi        {}
    # "HCAL:HcalEndcapPIslandProtoClusters:globalDistEtaPhi=",            # u_globalDistEtaPhi      {}
    # "HCAL:HcalEndcapPIslandProtoClusters:dimScaledLocalDistXY=",        # u_dimScaledLocalDistXY  {15.0*dd4hep::mm, 15.0*dd4hep::mm}

    # "HCAL:energyResolutions=",      # u_eRes ??
    # "HCAL:timeResolution=",         # m_tRes 0.0 * ns
    # "HCAL:capacityADC=",            # m_capADC 8096
    # "HCAL:dynamicRangeADC=",        # m_dyRangeADC 100 * MeV
    # "HCAL:pedestalMean=",           # m_pedMeanADC 400
    # "HCAL:pedestalSigma=",          # m_pedSigmaADC 3.2
    # "HCAL:resolutionTDC=",          # m_resolutionTDC 10 * picosecond
    # "HCAL:scaleResponse=",          # m_corrMeanScale 1
    # "HCAL:signalSumFields=",        # u_fields ??
    # "HCAL:fieldRefNumbers=",        # u_refs ??
    # "HCAL:geoServiceName=",         # m_geoSvcName ActsGeometryProvider
    # "HCAL:readoutClass=",           # m_readout ""


    # "RPOTS:ForwardRomanPotRawHits:threshold=",                  # config.threshold             # 0
    # "RPOTS:ForwardRomanPotRawHits:timeResolution=",             # config.timeResolution        # 8
    # "RPOTS:ForwardRomanPotRawHits:local_x_offset_station_1=",   # local_x_offset_station_1     # -833.3878326
    # "RPOTS:ForwardRomanPotRawHits:local_x_offset_station_2=",   # local_x_offset_station_2     # -924.342804
    # "RPOTS:ForwardRomanPotRawHits:local_x_slope_offset=",       # local_x_slope_offset         # -0.00622147
    # "RPOTS:ForwardRomanPotRawHits:local_y_slope_offset=",       # local_y_slope_offset         # -0.0451035
    # "RPOTS:ForwardRomanPotRawHits:crossingAngle=",              # crossingAngle                # -0.025
    # "RPOTS:ForwardRomanPotRawHits:nomMomentum=",                # nomMomentum                  # 275
    # "RPOTS:ForwardRomanPotRawHits:m_readout=",                  # m_readout                    # ""
    # "RPOTS:ForwardRomanPotRawHits:m_layerField=",               # m_layerField                 # ""
    # "RPOTS:ForwardRomanPotRawHits:m_sectorField=",              # m_sectorField                # ""
    # "RPOTS:ForwardRomanPotRawHits:m_localDetElement=",          # m_localDetElement            # ""
    # "RPOTS:ForwardRomanPotRawHits:u_localDetFields=",           # u_localDetFields             # {}
    # "RPOTS:ForwardRomanPotRawHits:time_resolution=",            # config.time_resolution       # 0


    # "ZDC:capacityADC=",                            # m_capADC            8096
    # "ZDC:dynamicRangeADC=",                        # m_dyRangeADC        100 * MeV
    # "ZDC:pedestalMean=",                           # m_pedMeanADC        400
    # "ZDC:pedestalSigma=",                          # m_pedSigmaADC       3.2
    # "ZDC:resolutionTDC=",                          # m_resolutionTDC     10 * dd4hep::picosecond
    # "ZDC:thresholdFactor=",                        # m_thresholdFactor   4
    # "ZDC:thresholdValue=",                         # m_thresholdValue    0
    # "ZDC:samplingFraction=",                       # m_sampFrac          0.998

    # "ZDC:ZDCEcalClusters:samplingFraction=",       # m_sampFrac          1
    # "ZDC:ZDCEcalClusters:logWeightBase=",          # m_logWeightBase     3.6
    # "ZDC:ZDCEcalClusters:depthCorrection=",        # m_depthCorrection   0
    # "ZDC:ZDCEcalClusters:energyWeight=",           # m_energyWeight      log
    # "ZDC:ZDCEcalClusters:moduleDimZName=",         # m_moduleDimZName    ""
    # "ZDC:ZDCEcalClusters:enableEtaBounds=",        # m_enableEtaBounds   FALSE

    # "ZDC:ZDCEcalTruthClusters:samplingFraction=",  # m_sampFrac          1
    # "ZDC:ZDCEcalTruthClusters:logWeightBase=",     # m_logWeightBase     3.6
    # "ZDC:ZDCEcalTruthClusters:depthCorrection=",   # m_depthCorrection   0
    # "ZDC:ZDCEcalTruthClusters:energyWeight=",      # m_energyWeight      log
    # "ZDC:ZDCEcalTruthClusters:moduleDimZName=",    # m_moduleDimZName    ""
    # "ZDC:ZDCEcalTruthClusters:enableEtaBounds=",   # m_enableEtaBounds   FALSE

    # "ZDC:splitCluster=",           # m_splitCluster            TRUE
    # "ZDC:minClusterHitEdep=",      # m_minClusterHitEdep       0.1 * MeV
    # "ZDC:minClusterCenterEdep=",   # m_minClusterCenterEdep    3.0 * MeV
    # "ZDC:sectorDist=",             # m_sectorDist              5.0 * cm
    # "ZDC:localDistXY=",            # u_localDistXY             {}
    # "ZDC:localDistXZ=",            # u_localDistXZ             {}
    # "ZDC:localDistYZ=",            # u_localDistYZ             {}
    # "ZDC:globalDistRPhi=",         # u_globalDistRPhi          {}
    # "ZDC:globalDistEtaPhi=",       # u_globalDistEtaPhi        {}
    # "ZDC:dimScaledLocalDistXY=",   # u_dimScaledLocalDistXY    {50.0*dd4hep::mm, 50.0*dd4hep::mm}

    # "ZDC:energyResolutions=",   # u_eRes            ??
    # "ZDC:timeResolution=",      # m_tRes            0.0 * ns
    # "ZDC:capacityADC=",         # m_capADC          8096
    # "ZDC:dynamicRangeADC=",     # m_dyRangeADC      100 * MeV
    # "ZDC:pedestalMean=",        # m_pedMeanADC      400
    # "ZDC:pedestalSigma=",       # m_pedSigmaADC     3.2
    # "ZDC:resolutionTDC=",       # m_resolutionTDC   10 * picosecond
    # "ZDC:scaleResponse=",       # m_corrMeanScale   1
    # "ZDC:signalSumFields=",     # u_fields          ??
    # "ZDC:fieldRefNumbers=",     # u_refs            ??
    # "ZDC:geoServiceName=",      # m_geoSvcName      ActsGeometryProvider
    # "ZDC:readoutClass=",        # m_readout         ""
]