#! /usr/bin/env python3
# Created 2022 by Dmitry Romanov
#
# One can run EICrecon with flags in this file:#
#    python3 reco_flags.py --nevents 1000 input.edm4hep.root output_no_ext
#
# Also when EICrecon builds it copies reco_flags.py as run_eicrecon_reco_flags.py.
# So if the output bin directory is in PATH one can do:#
#    run_eicrecon_reco_flags.py -n 1000 input.edm4hep.root output_no_ext
#
# The format of the table is:
#  [ (flag_name, default_val, description), ... ]
#
# '*' in the description means value is checked with reconstruction.py of Juggler

eicrecon_reco_flags = [

    # ========================= C A L O R I M E T R Y ================================

    # BEMC - Barrel EMC
    # ------------------

    # digitization
    ('BEMC:EcalBarrelRawHits:input_tag',                         'EcalBarrelHits',                  'Name of input collection to use'),
    ('BEMC:EcalBarrelRawHits:capacityADC',                       'capacityBitsADC=14',             '*'),
    ('BEMC:EcalBarrelRawHits:dynamicRangeADC',                   '20*GeV',                         '*'),
    ('BEMC:EcalBarrelRawHits:energyResolutions',                 '0.0,0.02,0.0',                   '*'),
    ('BEMC:EcalBarrelRawHits:fieldRefNumbers',                   '',                               ''),
    ('BEMC:EcalBarrelRawHits:geoServiceName',                    'ActsGeometryProvider',           ''),
    ('BEMC:EcalBarrelRawHits:pedestalMean',                      '100',                            '*'),
    ('BEMC:EcalBarrelRawHits:pedestalSigma',                     '1',                              '*'),
    ('BEMC:EcalBarrelRawHits:readoutClass',                      '',                               ''),
    ('BEMC:EcalBarrelRawHits:resolutionTDC',                     '1e-11',                          ''),
    ('BEMC:EcalBarrelRawHits:scaleResponse',                     '1',                              ''),
    ('BEMC:EcalBarrelRawHits:signalSumFields',                   '',                               ''),
    ('BEMC:EcalBarrelRawHits:timeResolution',                    '0',                              ''),

    # Hits reco
    ('BEMC:EcalBarrelRecHits:input_tag',                         'EcalBarrelRawHits',                  'Name of input collection to use'),
    ('BEMC:EcalBarrelRecHits:capacityADC',                       'capacityBitsADC=14',             '*'),
    ('BEMC:EcalBarrelRecHits:dynamicRangeADC',                   '20*GeV',                         '*'),
    ('BEMC:EcalBarrelRecHits:pedestalMean',                      '100',                            '*'),
    ('BEMC:EcalBarrelRecHits:pedestalSigma',                     '1',                              '*'),
    ('BEMC:EcalBarrelRecHits:resolutionTDC',                     '1e-11',                          ''),
    ('BEMC:EcalBarrelRecHits:samplingFraction',                  '0.98',                           '*'),
    ('BEMC:EcalBarrelRecHits:thresholdFactor',                   '3',                              '*'),
    ('BEMC:EcalBarrelRecHits:thresholdValue',                    '3',                              '*'),

    # clustering
    ('BEMC:EcalBarrelIslandProtoClusters:input_tag',             'EcalBarrelRecHits',                  'Name of input collection to use'),
    ('BEMC:EcalBarrelIslandProtoClusters:dimScaledLocalDistXY',  '1.8,1.8',                        ''),
    ('BEMC:EcalBarrelIslandProtoClusters:globalDistEtaPhi',      '',                               ''),
    ('BEMC:EcalBarrelIslandProtoClusters:globalDistRPhi',        '',                               ''),
    ('BEMC:EcalBarrelIslandProtoClusters:localDistXY',           '',                               ''),
    ('BEMC:EcalBarrelIslandProtoClusters:localDistXZ',           '',                               ''),
    ('BEMC:EcalBarrelIslandProtoClusters:localDistYZ',           '',                               ''),
    ('BEMC:EcalBarrelIslandProtoClusters:minClusterCenterEdep',  '30*MeV',                         '*'),
    ('BEMC:EcalBarrelIslandProtoClusters:minClusterHitEdep',     '1.0*MeV',                        '*'),
    ('BEMC:EcalBarrelIslandProtoClusters:sectorDist',            '5.0*cm',                         '*'),
    ('BEMC:EcalBarrelIslandProtoClusters:splitCluster',          '0',                              '*'),

    ('BEMC:EcalBarrelClusters:input_protoclust_tag',             'EcalBarrelIslandProtoClusters',  'Name of input collection to use'),
    ('BEMC:EcalBarrelClusters:depthCorrection',                  '0',                              ''),
    ('BEMC:EcalBarrelClusters:enableEtaBounds',                  '1',                              '*'),
    ('BEMC:EcalBarrelClusters:energyWeight',                     'log',                            ''),
    ('BEMC:EcalBarrelClusters:input_protoclust_tag',             'EcalBarrelIslandProtoClusters',  ''),
    ('BEMC:EcalBarrelClusters:input_simhit_tag',                 'EcalBarrelHits',                 ''),
    ('BEMC:EcalBarrelClusters:logWeightBase',                    '6.2',                            '*'),
    ('BEMC:EcalBarrelClusters:moduleDimZName',                   '',                               ''),
    ('BEMC:EcalBarrelClusters:samplingFraction',                 '1',                              '*'),

    ('BEMC:EcalBarrelTruthClusters:input_protoclust_tag',        'EcalBarrelTruthProtoClusters',  'Name of input collection to use'),
    ('BEMC:EcalBarrelTruthClusters:depthCorrection',             '0',                              ''),
    ('BEMC:EcalBarrelTruthClusters:enableEtaBounds',             '1',                              '*'),
    ('BEMC:EcalBarrelTruthClusters:energyWeight',                'log',                            ''),
    ('BEMC:EcalBarrelTruthClusters:input_protoclust_tag',        'EcalBarrelTruthProtoClusters',   ''),
    ('BEMC:EcalBarrelTruthClusters:input_simhit_tag',            'EcalBarrelHits',                 ''),
    ('BEMC:EcalBarrelTruthClusters:logWeightBase',               '6.2',                            '*'),
    ('BEMC:EcalBarrelTruthClusters:moduleDimZName',              '',                               ''),
    ('BEMC:EcalBarrelTruthClusters:samplingFraction',            '1',                              '*'),

    # BEMC - Imaging Barrel
    #------------------
    ('BEMC:EcalBarrelImagingRawHits:input_tag',                 'EcalBarrelHits',                  'Name of input collection to use'),
    ('BEMC:EcalBarrelImagingRawHits:energyResolutions',         '0.0,0.02,0.0',                    ''),
    ('BEMC:EcalBarrelImagingRawHits:timeResolution',            '0.0*ns',                          ''),
    ('BEMC:EcalBarrelImagingRawHits:capacityADC',               'capacityBitsADC=13',              ''),
    ('BEMC:EcalBarrelImagingRawHits:dynamicRangeADC',           '3*MeV',                           ''),
    ('BEMC:EcalBarrelImagingRawHits:pedestalMean',              '100',                             ''),
    ('BEMC:EcalBarrelImagingRawHits:pedestalSigma',             '14',                              ''),
    ('BEMC:EcalBarrelImagingRawHits:resolutionTDC',             '10*picosecond',                   ''),
    ('BEMC:EcalBarrelImagingRawHits:scaleResponse',             '1.0',                             ''),
    ('BEMC:EcalBarrelImagingRawHits:signalSumFields',           '',                                ''),
    ('BEMC:EcalBarrelImagingRawHits:fieldRefNumbers',           '',                                ''),
    ('BEMC:EcalBarrelImagingRawHits:geoServiceName',            'ActsGeometryProvider',            ''),
    ('BEMC:EcalBarrelImagingRawHits:readoutClass',              'pixel',                           ''),

    ('BEMC:EcalBarrelScFiRawHits:input_tag',                     'EcalBarrelScFiHits',             'Name of input collection to use'),
    ('BEMC:EcalBarrelScFiRawHits:energyResolutions',             '',                               ''),
    ('BEMC:EcalBarrelScFiRawHits:timeResolution',                '0*ns',                           ''),
    ('BEMC:EcalBarrelScFiRawHits:capacityADC',                   'capacityBitsADC=14',             ''),
    ('BEMC:EcalBarrelScFiRawHits:dynamicRangeADC',               '750*MeV',                        ''),
    ('BEMC:EcalBarrelScFiRawHits:pedestalMean',                  '20',                             ''),
    ('BEMC:EcalBarrelScFiRawHits:pedestalSigma',                 '0.3',                            ''),
    ('BEMC:EcalBarrelScFiRawHits:resolutionTDC',                 '10*picosecond',                  ''),
    ('BEMC:EcalBarrelScFiRawHits:scaleResponse',                 '1.0',                            ''),
    ('BEMC:EcalBarrelScFiRawHits:signalSumFields',               '',                               ''),
    ('BEMC:EcalBarrelScFiRawHits:fieldRefNumbers',               '',                               ''),
    ('BEMC:EcalBarrelScFiRawHits:geoServiceName',                'ActsGeometryProvider',           ''),
    ('BEMC:EcalBarrelScFiRawHits:readoutClass',                  'light_guide',                    ''),

    ('BEMC:EcalBarrelImagingRecHits:input_tag',                 'EcalBarrelRawHits',               'Name of input collection to use'),
    ('BEMC:EcalBarrelImagingRecHits:layerField',                'layer',                           ''),
    ('BEMC:EcalBarrelImagingRecHits:sectorField',               'module',                          ''),
    ('BEMC:EcalBarrelImagingRecHits:capacityADC',               'capacityBitsADC=13',              ''),
    ('BEMC:EcalBarrelImagingRecHits:pedestalMean',              '100',                             ''),
    ('BEMC:EcalBarrelImagingRecHits:dynamicRangeADC',           '3*MeV',                           ''),
    ('BEMC:EcalBarrelImagingRecHits:pedSigmaADC',               '14',                              ''),
    ('BEMC:EcalBarrelImagingRecHits:thresholdFactor',           '3.0',                             ''),
    ('BEMC:EcalBarrelImagingRecHits:samplingFraction',          '0.005',                           ''),

    ('BEMC:EcalBarrelScFiRecHits:input_tag',                    'EcalBarrelScFiRawHits',           'Name of input collection to use'),
    ('BEMC:EcalBarrelScFiRecHits:capacityADC',                  'capacityBitsADC=14',              ''),
    ('BEMC:EcalBarrelScFiRecHits:dynamicRangeADC',              '750*MeV',                         ''),
    ('BEMC:EcalBarrelScFiRecHits:pedestalMean',                 '20',                              ''),
    ('BEMC:EcalBarrelScFiRecHits:pedestalSigma',                '0.3',                             ''),
    ('BEMC:EcalBarrelScFiRecHits:resolutionTDC',                '10*picosecond',                   ''),
    ('BEMC:EcalBarrelScFiRecHits:thresholdFactor',              '5.0',                             ''),
    ('BEMC:EcalBarrelScFiRecHits:thresholdValue',               '0.0',                             ''),
    ('BEMC:EcalBarrelScFiRecHits:samplingFraction',             '0.125',                           ''),

    ('BEMC:EcalBarrelscFiMergedHits:input_tag',                 'EcalBarrelScFiRecHits',           ''),
    ('BEMC:EcalBarrelscFiMergedHits:fields',                    'fiber,z',                         ''),
    ('BEMC:EcalBarrelscFiMergedHits:refs',                      '1,1',                             ''),

    ('BEMC:EcalBarrelImagingProtoClusters:input_tag',           'EcalBarrelImagingRecHits',        'Name of input collection to use'),
    ('BEMC:EcalBarrelImagingProtoClusters::localDistXY',        '2.0*mm,2*mm',                     ''),
    ('BEMC:EcalBarrelImagingProtoClusters::layerDistEtaPhi',    '10*mrad,10*mrad',                 ''),
    ('BEMC:EcalBarrelImagingProtoClusters::neighbourLayersRange', '2.0',                           ''),
    ('BEMC:EcalBarrelImagingProtoClusters::sectorDist',         '3.0*cm',                          ''),
    ('BEMC:EcalBarrelImagingProtoClusters::minClusterHitEdep',  '0.',                              ''),
    ('BEMC:EcalBarrelImagingProtoClusters::minClusterCenterEdep', '0.',                            ''),
    ('BEMC:EcalBarrelImagingProtoClusters::minClusterEdep',     '0.5*MeV',                         ''),
    ('BEMC:EcalBarrelImagingProtoClusters::minClusterNhits',    '5',                               ''),

    ('BEMC:EcalBarrelScFiProtoClusters:input_tag',              'EcalBarrelScFiMergedHits', 'Name of input collection to use'),
    ('BEMC:EcalBarrelScFiProtoClusters:splitCluster',           'false',                           ''),
    ('BEMC:EcalBarrelScFiProtoClusters:minClusterHitEdep',      '1.0*MeV',                         ''),
    ('BEMC:EcalBarrelScFiProtoClusters:minClusterCenterEdep',   '10.0*MeV',                        ''),
    ('BEMC:EcalBarrelScFiProtoClusters:sectorDist',             '5.0*cm',                          ''),
    ('BEMC:EcalBarrelScFiProtoClusters:localDistXY',            '',                                ''),
    ('BEMC:EcalBarrelScFiProtoClusters:localDistXZ',            '30*mm, 30*mm',                    ''),
    ('BEMC:EcalBarrelScFiProtoClusters:localDistYZ',            '',                                ''),
    ('BEMC:EcalBarrelScFiProtoClusters:globalDistRPhi',         '',                                ''),
    ('BEMC:EcalBarrelScFiProtoClusters:globalDistEtaPhi',       '',                                ''),
    ('BEMC:EcalBarrelScFiProtoClusters:dimScaledLocalDistXY',   '',                                ''),

    ('BEMC:EcalBarrelImagingClusters:input_protoclust_tag',     'EcalBarrelImagingProtoClusters',  'Name of input collection to use'),
    ('BEMC:EcalBarrelImagingClusters:trackStopLayer',           '6',                               ''),

    ('BEMC:EcalBarrelScFiClusters:input_protoclust_tag',        'EcalBarrelScFiProtoClusters', 'Name of input collection to use'),
    ('BEMC:EcalBarrelScFiClusters:samplingFraction',            '1.0',                             ''),
    ('BEMC:EcalBarrelScFiClusters:logWeightBase',               '6.2',                             ''),
    ('BEMC:EcalBarrelScFiClusters:depthCorrection',             '0.0',                             ''),
    ('BEMC:EcalBarrelScFiClusters:input_simhit_tag',            'EcalBarrelScFiHits',              ''),
    ('BEMC:EcalBarrelScFiClusters:energyWeight',                'log',                             ''),
    ('BEMC:EcalBarrelScFiClusters:moduleDimZName',              '',                                ''),
    ('BEMC:EcalBarrelScFiClusters:enableEtaBounds',             'false',                           ''),

    ('BEMC:EcalBarrelImagingMergedClusters:inputMCParticles_tag',     'MCParticles',                          ''),
    ('BEMC:EcalBarrelImagingMergedClusters:energyClusters_tag',       'EcalBarrelScFiClusters',               ''),
    ('BEMC:EcalBarrelImagingMergedClusters:energyAssociation_tag',    'EcalBarrelScFiClusterAssociations',    ''),
    ('BEMC:EcalBarrelImagingMergedClusters:positionClusters_tag',     'EcalBarrelImagingClusters',            ''),
    ('BEMC:EcalBarrelImagingMergedClusters:positionAssociations_tag', 'EcalBarrelImagingClusterAssociations', ''),

# EEMC - Endcap EMC
    # -----------------
    # Negative Endcap
    ('EEMC:EcalEndcapNRawHits:capacityADC',                      'capacityBitsADC=14',             '*'),
    ('EEMC:EcalEndcapNRawHits:dynamicRangeADC',                  '20*GeV',                         '*'),
    ('EEMC:EcalEndcapNRawHits:energyResolutions',                '0.0,0.02,0.0',                   '*'),
    ('EEMC:EcalEndcapNRawHits:fieldRefNumbers',                  '',                               ''),
    ('EEMC:EcalEndcapNRawHits:geoServiceName',                   'ActsGeometryProvider',           ''),
    ('EEMC:EcalEndcapNRawHits:pedestalMean',                     '100',                            '*'),
    ('EEMC:EcalEndcapNRawHits:pedestalSigma',                    '1',                              ''),
    ('EEMC:EcalEndcapNRawHits:readoutClass',                     '',                               ''),
    ('EEMC:EcalEndcapNRawHits:resolutionTDC',                    '1e-11',                          ''),
    ('EEMC:EcalEndcapNRawHits:scaleResponse',                    '1',                              ''),
    ('EEMC:EcalEndcapNRawHits:signalSumFields',                  '',                               ''),
    ('EEMC:EcalEndcapNRawHits:timeResolution',                   '0',                              ''),

    ('EEMC:EcalEndcapNRecHits:capacityADC',                      'capacityBitsADC=14',             '*'),
    ('EEMC:EcalEndcapNRecHits:dynamicRangeADC',                  '20*GeV',                         '*'),
    ('EEMC:EcalEndcapNRecHits:geoServiceName',                   'geoServiceName',                 ''),
    ('EEMC:EcalEndcapNRecHits:layerField',                       '',                               ''),
    ('EEMC:EcalEndcapNRecHits:localDetElement',                  '',                               ''),
    ('EEMC:EcalEndcapNRecHits:localDetFields',                   '',                               ''),
    ('EEMC:EcalEndcapNRecHits:pedestalMean',                     '100',                            '*'),
    ('EEMC:EcalEndcapNRecHits:pedestalSigma',                    '1',                              '*'),
    ('EEMC:EcalEndcapNRecHits:readout',                          'EcalEndcapNHits',                ''),
    ('EEMC:EcalEndcapNRecHits:resolutionTDC',                    '1e-11',                          ''),
    ('EEMC:EcalEndcapNRecHits:samplingFraction',                 '0.998',                          '*'),
    ('EEMC:EcalEndcapNRecHits:sectorField',                      'sector',                         ''),
    ('EEMC:EcalEndcapNRecHits:thresholdFactor',                  '4',                              '*'),
    ('EEMC:EcalEndcapNRecHits:thresholdValue',                   '3',                              '*'),

    ('EEMC:EcalEndcapNIslandProtoClusters:dimScaledLocalDistXY', '1.8,1.8',                        '*'),
    ('EEMC:EcalEndcapNIslandProtoClusters:globalDistEtaPhi',     '',                               ''),
    ('EEMC:EcalEndcapNIslandProtoClusters:globalDistRPhi',       '',                               ''),
    ('EEMC:EcalEndcapNIslandProtoClusters:localDistXY',          '',                               ''),
    ('EEMC:EcalEndcapNIslandProtoClusters:localDistXZ',          '',                               ''),
    ('EEMC:EcalEndcapNIslandProtoClusters:localDistYZ',          '',                               ''),
    ('EEMC:EcalEndcapNIslandProtoClusters:minClusterCenterEdep', '1.0*MeV',                        '*'),
    ('EEMC:EcalEndcapNIslandProtoClusters:minClusterHitEdep',    '30*MeV',                         '*'),
    ('EEMC:EcalEndcapNIslandProtoClusters:sectorDist',           '5.0*cm',                         '*'),
    ('EEMC:EcalEndcapNIslandProtoClusters:splitCluster',         '0',                              '*'),

    ('EEMC:EcalEndcapNTruthClusters:depthCorrection',            '0',                              ''),
    ('EEMC:EcalEndcapNTruthClusters:enableEtaBounds',            '0',                              ''),
    ('EEMC:EcalEndcapNTruthClusters:energyWeight',               'log',                            ''),
    ('EEMC:EcalEndcapNTruthClusters:logWeightBase',              '4.6',                            '*'),
    ('EEMC:EcalEndcapNTruthClusters:moduleDimZName',             '',                               ''),
    ('EEMC:EcalEndcapNTruthClusters:samplingFraction',           '0.03',                           '*'),

    ('EEMC:EcalEndcapNClusters:depthCorrection',                 '0',                              ''),
    ('EEMC:EcalEndcapNClusters:dimScaledLocalDistXY',            '1.8,1.8',                        '*'),
    ('EEMC:EcalEndcapNClusters:enableEtaBounds',                 '0',                              ''),
    ('EEMC:EcalEndcapNClusters:energyWeight',                    'log',                            ''),
    ('EEMC:EcalEndcapNClusters:globalDistEtaPhi',                '',                               ''),
    ('EEMC:EcalEndcapNClusters:globalDistRPhi',                  '',                               ''),
    ('EEMC:EcalEndcapNClusters:localDistXY',                     '',                               ''),
    ('EEMC:EcalEndcapNClusters:localDistXZ',                     '',                               ''),
    ('EEMC:EcalEndcapNClusters:localDistYZ',                     '',                               ''),
    ('EEMC:EcalEndcapNClusters:logWeightBase',                   '3.6',                            ''),
    ('EEMC:EcalEndcapNClusters:minClusterCenterEdep',            '0.03',                           ''),
    ('EEMC:EcalEndcapNClusters:minClusterHitEdep',               '0.001',                          ''),
    ('EEMC:EcalEndcapNClusters:moduleDimZName',                  '',                               ''),
    ('EEMC:EcalEndcapNClusters:samplingFraction',                '1',                              ''),
    ('EEMC:EcalEndcapNClusters:sectorDist',                      '5',                              ''),
    ('EEMC:EcalEndcapNClusters:splitCluster',                    '0',                              ''),

    # Positive Endcap
    ('EEMC:EcalEndcapPRawHits:capacityADC',                      'capacityBitsADC=14',             '*'),
    ('EEMC:EcalEndcapPRawHits:dynamicRangeADC',                  '3*GeV',                          '*'),
    ('EEMC:EcalEndcapPRawHits:energyResolutions',                '0.00316,0.0015,0.0',             '*'),
    ('EEMC:EcalEndcapPRawHits:fieldRefNumbers',                  '1,1',                            '*'),
    ('EEMC:EcalEndcapPRawHits:geoServiceName',                   'ActsGeometryProvider',           ''),
    ('EEMC:EcalEndcapPRawHits:pedestalMean',                     '100',                            '*'),
    ('EEMC:EcalEndcapPRawHits:pedestalSigma',                    '0.7',                            '*'),
    ('EEMC:EcalEndcapPRawHits:readoutClass',                     '',                               ''),
    ('EEMC:EcalEndcapPRawHits:resolutionTDC',                    '1e-11',                          ''),
    ('EEMC:EcalEndcapPRawHits:scaleResponse',                    '1',                              ''),
    ('EEMC:EcalEndcapPRawHits:signalSumFields',                  '',                               ''),
    ('EEMC:EcalEndcapPRawHits:timeResolution',                   '0',                              ''),

    ('EEMC:EcalEndcapPRecHits:capacityADC',                      'capacityBitsADC=14',             '*'),
    ('EEMC:EcalEndcapPRecHits:dynamicRangeADC',                  '3*GeV',                          '*'),
    ('EEMC:EcalEndcapPRecHits:geoServiceName',                   'geoServiceName',                 ''),
    ('EEMC:EcalEndcapPRecHits:layerField',                       '',                               ''),
    ('EEMC:EcalEndcapPRecHits:localDetElement',                  '',                               ''),
    ('EEMC:EcalEndcapPRecHits:localDetFields',                   '',                               ''),
    ('EEMC:EcalEndcapPRecHits:pedestalMean',                     '100',                            '*'),
    ('EEMC:EcalEndcapPRecHits:pedestalSigma',                    '0.7',                            '*'),
    ('EEMC:EcalEndcapPRecHits:readout',                          'EcalEndcapPHits',                ''),
    ('EEMC:EcalEndcapPRecHits:resolutionTDC',                    '1e-11',                          ''),
    ('EEMC:EcalEndcapPRecHits:samplingFraction',                 '0.03',                           '*'),
    ('EEMC:EcalEndcapPRecHits:sectorField',                      'sector',                         ''),
    ('EEMC:EcalEndcapPRecHits:thresholdFactor',                  '5.0',                            '*'),
    ('EEMC:EcalEndcapPRecHits:thresholdValue',                   '2',                              '*'),

    ('EEMC:EcalEndcapPIslandProtoClusters:dimScaledLocalDistXY', '1.5,1.5',                        ''),
    ('EEMC:EcalEndcapPIslandProtoClusters:globalDistEtaPhi',     '',                               ''),
    ('EEMC:EcalEndcapPIslandProtoClusters:globalDistRPhi',       '',                               ''),
    ('EEMC:EcalEndcapPIslandProtoClusters:localDistXY',          '10,10',                          '* [mm]'),
    ('EEMC:EcalEndcapPIslandProtoClusters:localDistXZ',          '',                               ''),
    ('EEMC:EcalEndcapPIslandProtoClusters:localDistYZ',          '',                               ''),
    ('EEMC:EcalEndcapPIslandProtoClusters:minClusterCenterEdep', '10.0*MeV',                       '*'),
    ('EEMC:EcalEndcapPIslandProtoClusters:minClusterHitEdep',    '0',                              ''),
    ('EEMC:EcalEndcapPIslandProtoClusters:sectorDist',           '5',                              ''),
    ('EEMC:EcalEndcapPIslandProtoClusters:splitCluster',         '0',                              ''),

    ('EEMC:EcalEndcapPTruthClusters:depthCorrection',            '0',                              ''),
    ('EEMC:EcalEndcapPTruthClusters:enableEtaBounds',            '1',                              '*'),
    ('EEMC:EcalEndcapPTruthClusters:energyWeight',               'log',                            ''),
    ('EEMC:EcalEndcapPTruthClusters:logWeightBase',              '6.2',                            '*'),
    ('EEMC:EcalEndcapPTruthClusters:moduleDimZName',             '',                               ''),
    ('EEMC:EcalEndcapPTruthClusters:samplingFraction',           '1',                              ''),

    ('EEMC:EcalEndcapPClusters:depthCorrection',                 '0',                              ''),
    ('EEMC:EcalEndcapPClusters:dimScaledLocalDistXY',            '1.8,1.8',                        ''),
    ('EEMC:EcalEndcapPClusters:enableEtaBounds',                 '0',                              ''),
    ('EEMC:EcalEndcapPClusters:energyWeight',                    'log',                            ''),
    ('EEMC:EcalEndcapPClusters:globalDistEtaPhi',                '',                               ''),
    ('EEMC:EcalEndcapPClusters:globalDistRPhi',                  '',                               ''),
    ('EEMC:EcalEndcapPClusters:localDistXY',                     '',                               ''),
    ('EEMC:EcalEndcapPClusters:localDistXZ',                     '',                               ''),
    ('EEMC:EcalEndcapPClusters:localDistYZ',                     '',                               ''),
    ('EEMC:EcalEndcapPClusters:logWeightBase',                   '3.6',                            ''),
    ('EEMC:EcalEndcapPClusters:minClusterCenterEdep',            '0.03',                           ''),
    ('EEMC:EcalEndcapPClusters:minClusterHitEdep',               '0.001',                          ''),
    ('EEMC:EcalEndcapPClusters:moduleDimZName',                  '',                               ''),
    ('EEMC:EcalEndcapPClusters:samplingFraction',                '1',                              ''),
    ('EEMC:EcalEndcapPClusters:sectorDist',                      '5',                              ''),
    ('EEMC:EcalEndcapPClusters:splitCluster',                    '0',                              '*'),


    # HCAL Barrel
    # -----------
    ('HCAL:HcalBarrelRawHits:capacityADC',                       'capacityBitsADC=8',              '*'),
    ('HCAL:HcalBarrelRawHits:dynamicRangeADC',                   '20*MeV',                         '*'),
    ('HCAL:HcalBarrelRawHits:energyResolutions',                 '',                               ''),
    ('HCAL:HcalBarrelRawHits:fieldRefNumbers',                   '1,0',                            '*'),
    ('HCAL:HcalBarrelRawHits:geoServiceName',                    'ActsGeometryProvider',           ''),
    ('HCAL:HcalBarrelRawHits:pedestalMean',                      '20',                             '*'),
    ('HCAL:HcalBarrelRawHits:pedestalSigma',                     '0.3',                            '*'),
    ('HCAL:HcalBarrelRawHits:readoutClass',                      '',                               ''),
    ('HCAL:HcalBarrelRawHits:resolutionTDC',                     '1e-11',                          ''),
    ('HCAL:HcalBarrelRawHits:scaleResponse',                     '1',                              ''),
    ('HCAL:HcalBarrelRawHits:signalSumFields',                   '',                               ''),
    ('HCAL:HcalBarrelRawHits:timeResolution',                    '0',                              ''),

    ('HCAL:HcalBarrelRecHits:capacityADC',                       'capacityBitsADC=8',              '*'),
    ('HCAL:HcalBarrelRecHits:dynamicRangeADC',                   '20*MeV',                         '*'),
    ('HCAL:HcalBarrelRecHits:geoServiceName',                    'geoServiceName',                 ''),
    ('HCAL:HcalBarrelRecHits:layerField',                        'layer',                          ''),
    ('HCAL:HcalBarrelRecHits:localDetElement',                   '',                               ''),
    ('HCAL:HcalBarrelRecHits:localDetFields',                    '',                               ''),
    ('HCAL:HcalBarrelRecHits:pedestalMean',                      '20',                            '*'),
    ('HCAL:HcalBarrelRecHits:pedestalSigma',                     '0.3',                            '*'),
    ('HCAL:HcalBarrelRecHits:readout',                           'HcalBarrelHits',                 ''),
    ('HCAL:HcalBarrelRecHits:resolutionTDC',                     '1e-11',                          ''),
    ('HCAL:HcalBarrelRecHits:samplingFraction',                  '0.038',                          ''),
    ('HCAL:HcalBarrelRecHits:sectorField',                       'module',                         ''),
    ('HCAL:HcalBarrelRecHits:thresholdFactor',                   '5',                              '*'),
    ('HCAL:HcalBarrelRecHits:thresholdValue',                    '1',                              '*'),

    ('HCAL:HcalBarrelClusters:depthCorrection',                  '0',                              ''),
    ('HCAL:HcalBarrelClusters:enableEtaBounds',                  '0',                              ''),
    ('HCAL:HcalBarrelClusters:energyWeight',                     'log',                            ''),
    ('HCAL:HcalBarrelClusters:input_protoclust_tag',             'HcalBarrelIslandProtoClusters',  ''),
    ('HCAL:HcalBarrelClusters:input_simhit_tag',                 'HcalBarrelHits',                 ''),
    ('HCAL:HcalBarrelClusters:logWeightBase',                    '6.2',                            ''),
    ('HCAL:HcalBarrelClusters:moduleDimZName',                   '',                               ''),
    ('HCAL:HcalBarrelClusters:samplingFraction',                 '1',                              ''),

    ('HCAL:HcalBarrelIslandProtoClusters:dimScaledLocalDistXY',  '5,5',                            ''),
    ('HCAL:HcalBarrelIslandProtoClusters:globalDistEtaPhi',      '',                               ''),
    ('HCAL:HcalBarrelIslandProtoClusters:globalDistRPhi',        '',                               ''),
    ('HCAL:HcalBarrelIslandProtoClusters:localDistXY',           '150,150',                        '* [mm]'),
    ('HCAL:HcalBarrelIslandProtoClusters:localDistXZ',           '',                               ''),
    ('HCAL:HcalBarrelIslandProtoClusters:localDistYZ',           '',                               ''),
    ('HCAL:HcalBarrelIslandProtoClusters:minClusterCenterEdep',  '0.003',                          ''),
    ('HCAL:HcalBarrelIslandProtoClusters:minClusterHitEdep',     '30.0*MeV',                       '*'),
    ('HCAL:HcalBarrelIslandProtoClusters:sectorDist',            '5',                              ''),
    ('HCAL:HcalBarrelIslandProtoClusters:splitCluster',          '0',                              '*'),

    ('HCAL:HcalBarrelMergedHits:fields',                         'layer,slice',                    ''),
    ('HCAL:HcalBarrelMergedHits:input_tag',                      'HcalBarrelRecHits',              ''),
    ('HCAL:HcalBarrelMergedHits:refs',                           '1,0',                            ''),

    ('HCAL:HcalBarrelTruthClusters:depthCorrection',             '0',                              ''),
    ('HCAL:HcalBarrelTruthClusters:enableEtaBounds',             '0',                              ''),
    ('HCAL:HcalBarrelTruthClusters:energyWeight',                'log',                            ''),
    ('HCAL:HcalBarrelTruthClusters:input_protoclust_tag',        'HcalBarrelTruthProtoClusters',   ''),
    ('HCAL:HcalBarrelTruthClusters:input_simhit_tag',            'HcalBarrelHits',                 ''),
    ('HCAL:HcalBarrelTruthClusters:logWeightBase',               '6.2',                            ''),
    ('HCAL:HcalBarrelTruthClusters:moduleDimZName',              '',                               ''),
    ('HCAL:HcalBarrelTruthClusters:samplingFraction',            '1',                              ''),

    # HCAL Negative endcap
    # --------------------
    ('HCAL:HcalEndcapNRawHits:capacityADC',                      'capacityBitsADC=8',              '*'),
    ('HCAL:HcalEndcapNRawHits:dynamicRangeADC',                  '20*MeV',                            '*'),
    ('HCAL:HcalEndcapNRawHits:energyResolutions',                '',                               ''),
    ('HCAL:HcalEndcapNRawHits:fieldRefNumbers',                  '',                               ''),
    ('HCAL:HcalEndcapNRawHits:geoServiceName',                   'ActsGeometryProvider',           ''),
    ('HCAL:HcalEndcapNRawHits:pedestalMean',                     '20',                            '*'),
    ('HCAL:HcalEndcapNRawHits:pedestalSigma',                    '0.3',                            ''),
    ('HCAL:HcalEndcapNRawHits:readoutClass',                     '',                               ''),
    ('HCAL:HcalEndcapNRawHits:resolutionTDC',                    '1e-11',                          ''),
    ('HCAL:HcalEndcapNRawHits:scaleResponse',                    '1',                              ''),
    ('HCAL:HcalEndcapNRawHits:signalSumFields',                  '',                               ''),
    ('HCAL:HcalEndcapNRawHits:timeResolution',                   '0',                              ''),

    ('HCAL:HcalEndcapNRecHits:capacityADC',                      'capacityBitsADC=8',              '*'),
    ('HCAL:HcalEndcapNRecHits:dynamicRangeADC',                  '20*MeV',                            '*'),
    ('HCAL:HcalEndcapNRecHits:geoServiceName',                   'geoServiceName',                 ''),
    ('HCAL:HcalEndcapNRecHits:layerField',                       '',                               ''),
    ('HCAL:HcalEndcapNRecHits:localDetElement',                  '',                               ''),
    ('HCAL:HcalEndcapNRecHits:localDetFields',                   '',                               ''),
    ('HCAL:HcalEndcapNRecHits:pedestalMean',                     '20',                            '*'),
    ('HCAL:HcalEndcapNRecHits:pedestalSigma',                    '0.3',                            '*'),
    ('HCAL:HcalEndcapNRecHits:readout',                          'HcalEndcapNHits',                ''),
    ('HCAL:HcalEndcapNRecHits:resolutionTDC',                    '1e-11',                          ''),
    ('HCAL:HcalEndcapNRecHits:samplingFraction',                 '0.998',                          ''),
    ('HCAL:HcalEndcapNRecHits:sectorField',                      'sector',                         ''),
    ('HCAL:HcalEndcapNRecHits:thresholdFactor',                  '4',                              ''),
    ('HCAL:HcalEndcapNRecHits:thresholdValue',                   '1',                              '*'),

    ('HCAL:HcalEndcapNIslandProtoClusters:dimScaledLocalDistXY', '1.5,1.5',                        ''),
    ('HCAL:HcalEndcapNIslandProtoClusters:globalDistEtaPhi',     '',                               ''),
    ('HCAL:HcalEndcapNIslandProtoClusters:globalDistRPhi',       '',                               ''),
    ('HCAL:HcalEndcapNIslandProtoClusters:localDistXY',          '150,150',                        '* [mm]'),
    ('HCAL:HcalEndcapNIslandProtoClusters:localDistXZ',          '',                               ''),
    ('HCAL:HcalEndcapNIslandProtoClusters:localDistYZ',          '',                               ''),
    ('HCAL:HcalEndcapNIslandProtoClusters:minClusterCenterEdep', '30*MeV',                         '*'),
    ('HCAL:HcalEndcapNIslandProtoClusters:minClusterHitEdep',    '0',                              ''),
    ('HCAL:HcalEndcapNIslandProtoClusters:sectorDist',           '5',                              ''),
    ('HCAL:HcalEndcapNIslandProtoClusters:splitCluster',         '1',                              ''),

    ('HCAL:HcalEndcapNTruthClusters:depthCorrection',            '0',                              ''),
    ('HCAL:HcalEndcapNTruthClusters:enableEtaBounds',            '0',                              ''),
    ('HCAL:HcalEndcapNTruthClusters:energyWeight',               'log',                            ''),
    ('HCAL:HcalEndcapNTruthClusters:input_protoclust_tag',       'HcalEndcapNTruthProtoClusters',  ''),
    ('HCAL:HcalEndcapNTruthClusters:input_simhit_tag',           'HcalEndcapNHits',                ''),
    ('HCAL:HcalEndcapNTruthClusters:logWeightBase',              '6.2',                            '*'),
    ('HCAL:HcalEndcapNTruthClusters:moduleDimZName',             '',                               ''),
    ('HCAL:HcalEndcapNTruthClusters:samplingFraction',           '1',                              ''),

    ('HCAL:HcalEndcapNClusters:depthCorrection',                 '0',                              ''),
    ('HCAL:HcalEndcapNClusters:enableEtaBounds',                 '0',                              ''),
    ('HCAL:HcalEndcapNClusters:energyWeight',                    'log',                            ''),
    ('HCAL:HcalEndcapNClusters:input_protoclust_tag',            'HcalEndcapNIslandProtoClusters', ''),
    ('HCAL:HcalEndcapNClusters:input_simhit_tag',                'HcalEndcapNHits',                ''),
    ('HCAL:HcalEndcapNClusters:logWeightBase',                   '6.2',                            '*'),
    ('HCAL:HcalEndcapNClusters:moduleDimZName',                  '',                               ''),
    ('HCAL:HcalEndcapNClusters:samplingFraction',                '1',                              ''),

    # HCAL Positive endcap
    # --------------------
    ('HCAL:HcalEndcapPRawHits:capacityADC',                      'capacityBitsADC=10',             '*'),
    ('HCAL:HcalEndcapPRawHits:dynamicRangeADC',                  '3.6*GeV',                            '*'),
    ('HCAL:HcalEndcapPRawHits:energyResolutions',                '',                               ''),
    ('HCAL:HcalEndcapPRawHits:fieldRefNumbers',                  '',                               ''),
    ('HCAL:HcalEndcapPRawHits:geoServiceName',                   'ActsGeometryProvider',           ''),
    ('HCAL:HcalEndcapPRawHits:pedestalMean',                     '20',                            '*'),
    ('HCAL:HcalEndcapPRawHits:pedestalSigma',                    '0.8',                            '*'),
    ('HCAL:HcalEndcapPRawHits:readoutClass',                     '',                               ''),
    ('HCAL:HcalEndcapPRawHits:resolutionTDC',                    '1e-11',                          ''),
    ('HCAL:HcalEndcapPRawHits:scaleResponse',                    '1',                              ''),
    ('HCAL:HcalEndcapPRawHits:signalSumFields',                  '',                               ''),
    ('HCAL:HcalEndcapPRawHits:timeResolution',                   '0',                              ''),

    ('HCAL:HcalEndcapPRecHits:capacityADC',                      'capacityBitsADC=10',             '*'),
    ('HCAL:HcalEndcapPRecHits:dynamicRangeADC',                  '3.6*GeV',                            '*'),
    ('HCAL:HcalEndcapPRecHits:geoServiceName',                   'geoServiceName',                 ''),
    ('HCAL:HcalEndcapPRecHits:layerField',                       '',                               ''),
    ('HCAL:HcalEndcapPRecHits:localDetElement',                  '',                               ''),
    ('HCAL:HcalEndcapPRecHits:localDetFields',                   '',                               ''),
    ('HCAL:HcalEndcapPRecHits:pedestalMean',                     '20',                            '*'),
    ('HCAL:HcalEndcapPRecHits:pedestalSigma',                    '0.8',                            '*'),
    ('HCAL:HcalEndcapPRecHits:readout',                          'HcalEndcapNHits',                ''),
    ('HCAL:HcalEndcapPRecHits:resolutionTDC',                    '1e-11',                          ''),
    ('HCAL:HcalEndcapPRecHits:samplingFraction',                 '0.025',                          ''),
    ('HCAL:HcalEndcapPRecHits:sectorField',                      'sector',                         ''),
    ('HCAL:HcalEndcapPRecHits:thresholdFactor',                  '5',                              '*'),
    ('HCAL:HcalEndcapPRecHits:thresholdValue',                   '3',                              '*'),

    ('HCAL:HcalEndcapPIslandProtoClusters:dimScaledLocalDistXY', '1.5,1.5',                        ''),
    ('HCAL:HcalEndcapPIslandProtoClusters:globalDistEtaPhi',     '',                               ''),
    ('HCAL:HcalEndcapPIslandProtoClusters:globalDistRPhi',       '',                               ''),
    ('HCAL:HcalEndcapPIslandProtoClusters:localDistXY',          '150,150',                        '* [mm]'),
    ('HCAL:HcalEndcapPIslandProtoClusters:localDistXZ',          '',                               ''),
    ('HCAL:HcalEndcapPIslandProtoClusters:localDistYZ',          '',                               ''),
    ('HCAL:HcalEndcapPIslandProtoClusters:minClusterCenterEdep', '30.0*MeV',                       '*'),
    ('HCAL:HcalEndcapPIslandProtoClusters:minClusterHitEdep',    '0',                              ''),
    ('HCAL:HcalEndcapPIslandProtoClusters:sectorDist',           '5',                              ''),
    ('HCAL:HcalEndcapPIslandProtoClusters:splitCluster',         '1',                              ''),

    ('HCAL:HcalEndcapPTruthClusters:depthCorrection',            '0',                              ''),
    ('HCAL:HcalEndcapPTruthClusters:enableEtaBounds',            '0',                              ''),
    ('HCAL:HcalEndcapPTruthClusters:energyWeight',               'log',                            ''),
    ('HCAL:HcalEndcapPTruthClusters:input_protoclust_tag',       'HcalEndcapNTruthProtoClusters',  ''),
    ('HCAL:HcalEndcapPTruthClusters:input_simhit_tag',           'HcalEndcapNHits',                ''),
    ('HCAL:HcalEndcapPTruthClusters:logWeightBase',              '6.2',                            '*'),
    ('HCAL:HcalEndcapPTruthClusters:moduleDimZName',             '',                               ''),
    ('HCAL:HcalEndcapPTruthClusters:samplingFraction',           '0.025',                          '*'),

    ('HCAL:HcalEndcapPClusters:depthCorrection',                 '0',                              ''),
    ('HCAL:HcalEndcapPClusters:enableEtaBounds',                 '0',                              ''),
    ('HCAL:HcalEndcapPClusters:energyWeight',                    'log',                            ''),
    ('HCAL:HcalEndcapPClusters:input_protoclust_tag',            'HcalEndcapNIslandProtoClusters', ''),
    ('HCAL:HcalEndcapPClusters:input_simhit_tag',                'HcalEndcapNHits',                ''),
    ('HCAL:HcalEndcapPClusters:logWeightBase',                   '6.2',                            '*'),
    ('HCAL:HcalEndcapPClusters:moduleDimZName',                  '',                               ''),
    ('HCAL:HcalEndcapPClusters:samplingFraction',                '0.025',                          '*'),


    # HCAL - Positive Endcap insert
    # ------------------------------------------
    ('HCAL:HcalEndcapPInsertRawHits:capacityADC',                      'capacityBitsADC=15',             '*'),
    ('HCAL:HcalEndcapPInsertRawHits:dynamicRangeADC',                  '200*MeV',                        '*'),
    ('HCAL:HcalEndcapPInsertRawHits:energyResolutions',                '',                               ''),
    ('HCAL:HcalEndcapPInsertRawHits:fieldRefNumbers',                  '',                               ''),
    ('HCAL:HcalEndcapPInsertRawHits:geoServiceName',                   'ActsGeometryProvider',           ''),
    ('HCAL:HcalEndcapPInsertRawHits:pedestalMean',                     '400',                            '*'),
    ('HCAL:HcalEndcapPInsertRawHits:pedestalSigma',                    '10',                             '*'),
    ('HCAL:HcalEndcapPInsertRawHits:readoutClass',                     '',                               ''),
    ('HCAL:HcalEndcapPInsertRawHits:resolutionTDC',                    '1e-11',                          ''),
    ('HCAL:HcalEndcapPInsertRawHits:scaleResponse',                    '1',                              ''),
    ('HCAL:HcalEndcapPInsertRawHits:signalSumFields',                  '',                               ''),
    ('HCAL:HcalEndcapPInsertRawHits:timeResolution',                   '0',                              ''),

    ('HCAL:HcalEndcapPInsertRecHits:capacityADC',                      'capacityBitsADC=15',             '*'),
    ('HCAL:HcalEndcapPInsertRecHits:dynamicRangeADC',                  '200*MeV',                        '*'),
    ('HCAL:HcalEndcapPInsertRecHits:geoServiceName',                   'geoServiceName',                 ''),
    ('HCAL:HcalEndcapPInsertRecHits:layerField',                       '',                               ''),
    ('HCAL:HcalEndcapPInsertRecHits:localDetElement',                  '',                               ''),
    ('HCAL:HcalEndcapPInsertRecHits:localDetFields',                   '',                               ''),
    ('HCAL:HcalEndcapPInsertRecHits:pedestalMean',                     '400',                            '*'),
    ('HCAL:HcalEndcapPInsertRecHits:pedestalSigma',                    '10',                             '*'),
    ('HCAL:HcalEndcapPInsertRecHits:readout',                          'HcalEndcapNHits',                ''),
    ('HCAL:HcalEndcapPInsertRecHits:resolutionTDC',                    '1e-11',                          ''),
    ('HCAL:HcalEndcapPInsertRecHits:samplingFraction',                 '0.998',                          ''),
    ('HCAL:HcalEndcapPInsertRecHits:sectorField',                      'sector',                         ''),
    ('HCAL:HcalEndcapPInsertRecHits:thresholdFactor',                  '4',                              ''),
    ('HCAL:HcalEndcapPInsertRecHits:thresholdValue',                   '0',                              '*'),

    ('HCAL:HcalEndcapPInsertIslandProtoClusters:dimScaledLocalDistXY', '1.5,1.5',                        ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:globalDistEtaPhi',     '',                               ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:globalDistRPhi',       '',                               ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:localDistXY',          '',                               ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:localDistXZ',          '',                               ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:localDistYZ',          '',                               ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:minClusterCenterEdep', '0.03',                           ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:minClusterHitEdep',    '0',                              ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:sectorDist',           '5',                              ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:splitCluster',         '1',                              ''),

    ('HCAL:HcalEndcapPInsertTruthClusters:depthCorrection',            '0',                              ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:enableEtaBounds',            '0',                              ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:energyWeight',               'log',                            ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:input_protoclust_tag',       'HcalEndcapNTruthProtoClusters',  ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:input_simhit_tag',           'HcalEndcapNHits',                ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:logWeightBase',              '6.2',                            '*'),
    ('HCAL:HcalEndcapPInsertTruthClusters:moduleDimZName',             '',                               ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:samplingFraction',           '1',                              ''),

    ('HCAL:HcalEndcapPInsertClusters:depthCorrection',                 '0',                              ''),
    ('HCAL:HcalEndcapPInsertClusters:enableEtaBounds',                 '0',                              ''),
    ('HCAL:HcalEndcapPInsertClusters:energyWeight',                    'log',                            ''),
    ('HCAL:HcalEndcapPInsertClusters:input_protoclust_tag',            'HcalEndcapNIslandProtoClusters', ''),
    ('HCAL:HcalEndcapPInsertClusters:input_simhit_tag',                'HcalEndcapNHits',                ''),
    ('HCAL:HcalEndcapPInsertClusters:logWeightBase',                   '6.2',                            '*'),
    ('HCAL:HcalEndcapPInsertClusters:moduleDimZName',                  '',                               ''),
    ('HCAL:HcalEndcapPInsertClusters:samplingFraction',                '1',                              ''),


    # ZDC - Zero degree calorimeter
    # -----------------------------
    ('ZDC:ZDCEcalRawHits:capacityADC',                           '8096',                           ''),
    ('ZDC:ZDCEcalRawHits:dynamicRangeADC',                       '0.1',                            ''),
    ('ZDC:ZDCEcalRawHits:energyResolutions',                     '',                               ''),
    ('ZDC:ZDCEcalRawHits:fieldRefNumbers',                       '',                               ''),
    ('ZDC:ZDCEcalRawHits:geoServiceName',                        'ActsGeometryProvider',           ''),
    ('ZDC:ZDCEcalRawHits:pedestalMean',                          '400',                            ''),
    ('ZDC:ZDCEcalRawHits:pedestalSigma',                         '3.2',                            ''),
    ('ZDC:ZDCEcalRawHits:readoutClass',                          '',                               ''),
    ('ZDC:ZDCEcalRawHits:resolutionTDC',                         '1e-11',                          ''),
    ('ZDC:ZDCEcalRawHits:scaleResponse',                         '1',                              ''),
    ('ZDC:ZDCEcalRawHits:signalSumFields',                       '',                               ''),
    ('ZDC:ZDCEcalRawHits:timeResolution',                        '0',                              ''),

    ('ZDC:ZDCEcalRecHits:capacityADC',                           '8096',                           ''),
    ('ZDC:ZDCEcalRecHits:dynamicRangeADC',                       '0.1',                            ''),
    ('ZDC:ZDCEcalRecHits:geoServiceName',                        'geoServiceName',                 ''),
    ('ZDC:ZDCEcalRecHits:layerField',                            '',                               ''),
    ('ZDC:ZDCEcalRecHits:localDetElement',                       '',                               ''),
    ('ZDC:ZDCEcalRecHits:localDetFields',                        '',                               ''),
    ('ZDC:ZDCEcalRecHits:pedestalMean',                          '400',                            ''),
    ('ZDC:ZDCEcalRecHits:pedestalSigma',                         '3.2',                            ''),
    ('ZDC:ZDCEcalRecHits:readout',                               'ZDCEcalHits',                    ''),
    ('ZDC:ZDCEcalRecHits:resolutionTDC',                         '1e-11',                          ''),
    ('ZDC:ZDCEcalRecHits:samplingFraction',                      '1',                              '*'),
    ('ZDC:ZDCEcalRecHits:sectorField',                           'sector',                         ''),
    ('ZDC:ZDCEcalRecHits:thresholdFactor',                       '4',                              ''),
    ('ZDC:ZDCEcalRecHits:thresholdValue',                        '0',                              ''),

    ('ZDC:ZDCEcalClusters:depthCorrection',                      '0',                              ''),
    ('ZDC:ZDCEcalClusters:enableEtaBounds',                      '0',                              ''),
    ('ZDC:ZDCEcalClusters:energyWeight',                         'log',                            ''),
    ('ZDC:ZDCEcalClusters:input_protoclust_tag',                 'ZDCEcalIslandProtoClusters',     ''),
    ('ZDC:ZDCEcalClusters:input_simhit_tag',                     'ZDCEcalHits',                    ''),
    ('ZDC:ZDCEcalClusters:logWeightBase',                        '6.2',                            '*'),
    ('ZDC:ZDCEcalClusters:moduleDimZName',                       '',                               ''),
    ('ZDC:ZDCEcalClusters:samplingFraction',                     '1',                              '*'),

    ('ZDC:ZDCEcalIslandProtoClusters:dimScaledLocalDistXY',      '5,5',                            ''),
    ('ZDC:ZDCEcalIslandProtoClusters:globalDistEtaPhi',          '',                               ''),
    ('ZDC:ZDCEcalIslandProtoClusters:globalDistRPhi',            '',                               ''),
    ('ZDC:ZDCEcalIslandProtoClusters:localDistXY',               '50,50',                          '* [mm]'),
    ('ZDC:ZDCEcalIslandProtoClusters:localDistXZ',               '',                               ''),
    ('ZDC:ZDCEcalIslandProtoClusters:localDistYZ',               '',                               ''),
    ('ZDC:ZDCEcalIslandProtoClusters:minClusterCenterEdep',      '3.*MeV',                         '*'),
    ('ZDC:ZDCEcalIslandProtoClusters:minClusterHitEdep',         '0.1*MeV',                        '*'),
    ('ZDC:ZDCEcalIslandProtoClusters:sectorDist',                '5',                              ''),
    ('ZDC:ZDCEcalIslandProtoClusters:splitCluster',              '1',                              ''),

    ('ZDC:ZDCEcalTruthClusters:depthCorrection',                 '0',                              ''),
    ('ZDC:ZDCEcalTruthClusters:enableEtaBounds',                 '0',                              ''),
    ('ZDC:ZDCEcalTruthClusters:energyWeight',                    'log',                            ''),
    ('ZDC:ZDCEcalTruthClusters:input_protoclust_tag',            'ZDCEcalTruthProtoClusters',      ''),
    ('ZDC:ZDCEcalTruthClusters:input_simhit_tag',                'ZDCEcalHits',                    ''),
    ('ZDC:ZDCEcalTruthClusters:logWeightBase',                   '3.6',                            '*'),
    ('ZDC:ZDCEcalTruthClusters:moduleDimZName',                  '',                               ''),
    ('ZDC:ZDCEcalTruthClusters:samplingFraction',                '1',                              '*'),

    # ========================= T R A C K I N G ================================

    ('BTRK:BarrelTrackerRawHit:Threshold',                       '0',                              '* [GeV] EDep threshold for hits to pass through, '),
    ('BTRK:BarrelTrackerRawHit:TimeResolution',                  '8',                              '* [ns] Time resolution gauss smearing'),
    ('BTRK:BarrelTrackerHit:TimeResolution',                     '8',                              '* [ns] Time resolution set to covariance matrix for CKF input'),

    ('BVTX:BarrelVertexRawHit:Threshold',                        '0',                              '* [GeV] EDep threshold for hits to pass through'),
    ('BVTX:BarrelVertexRawHit:TimeResolution',                   '8',                              '* [ns] Time resolution gauss smearing [ns]'),
    ('BVTX:BarrelVertexHit:TimeResolution',                      '8',                              '* [ns] Time resolution set to covariance matrix for CKF input'),

    ('ECTRK:EndcapTrackerRawHit:Threshold',                      '0',                              '* [GeV] EDep threshold for hits to pass through'),
    ('ECTRK:EndcapTrackerRawHit:TimeResolution',                 '8',                              '* [ns] Time resolution gauss smearing'),
    ('ECTRK:EndcapTrackerHit:TimeResolution',                    '8',                              '* [ns] Time resolution set to covariance matrix for CKF input'),

    ('MPGD:MPGDTrackerRawHit:Threshold',                         '0',                              '* [GeV] EDep threshold for hits to pass through'),
    ('MPGD:MPGDTrackerRawHit:TimeResolution',                    '8',                              '* [ns] Time resolution gauss smearing'),
    ('MPGD:MPGDTrackerHit:TimeResolution',                       '8',                              '* [ns] Time resolution set to covariance matrix for CKF input'),

    ('ECTOF:TOFEndcapRawHit:Threshold',                          '0',                              '* [GeV] EDep threshold for hits to pass through'),
    ('ECTOF:TOFEndcapRawHit:TimeResolution',                     '0.025',                          '* [ns] Time resolution gauss smearing'),
    ('ECTOF:TOFEndcapTrackerHit:TimeResolution',                 '0.025',                          '* [ns] Time resolution set to covariance matrix for CKF input'),

    ('BTOF:TOFBarrelRawHit:Threshold',                           '0',                              '* [GeV] EDep threshold for hits to pass through'),
    ('BTOF:TOFBarrelRawHit:TimeResolution',                      '0.025',                          '* [ns] Time resolution gauss smearing'),
    ('BTOF:TOFBarrelTrackerHit:TimeResolution',                  '0.025',                          '* [ns] Time resolution set to covariance matrix for CKF input'),

    ('tracking:CentralCKFTrajectories:Chi2CutOff',               '15',                             'Chi2 Cut Off for ACTS CKF tracking'),
    ('tracking:CentralCKFTrajectories:EtaBins',                  '',                               'Eta Bins for ACTS CKF tracking reco'),
    ('tracking:CentralCKFTrajectories:NumMeasurementsCutOff',    '10',                             'Number of measurements Cut Off for ACTS CKF tracking'),

    # ========================= R E C O N S T R U C T I O N ================================
    ('Reco:GeneratedParticles:MomentumSmearing',                 '0',                              'Gaussian momentum smearing value'),

]

#
# ========================================================================================================================================================
# System of units used here:
# ========================================================================================================================================================
# (Copied from system_of_units.py to remove dependency to copy)

#
# Energy [E]
#
megaelectronvolt = 1.
electronvolt     = 1.e-6*megaelectronvolt
kiloelectronvolt = 1.e-3*megaelectronvolt
gigaelectronvolt = 1.e+3*megaelectronvolt
teraelectronvolt = 1.e+6*megaelectronvolt
petaelectronvolt = 1.e+9*megaelectronvolt
# symbols
MeV = megaelectronvolt
eV  = electronvolt
keV = kiloelectronvolt
GeV = gigaelectronvolt
TeV = teraelectronvolt
PeV = petaelectronvolt
# Length [L]
#
millimeter  = 1.
millimeter2 = millimeter*millimeter
millimeter3 = millimeter*millimeter*millimeter
centimeter  = 10.*millimeter
centimeter2 = centimeter*centimeter
centimeter3 = centimeter*centimeter*centimeter
meter  = 1000.*millimeter
meter2 = meter*meter
meter3 = meter*meter*meter
kilometer = 1000.*meter
kilometer2 = kilometer*kilometer
kilometer3 = kilometer*kilometer*kilometer
parsec = 3.0856775807e+16*meter
micrometer = 1.e-6 *meter
nanometer  = 1.e-9 *meter
angstrom   = 1.e-10*meter
fermi      = 1.e-15*meter
barn       = 1.e-28*meter2
millibarn  = 1.e-3 *barn
microbarn  = 1.e-6 *barn
nanobarn   = 1.e-9 *barn
picobarn   = 1.e-12*barn
# symbols
mm  = millimeter
mm2 = millimeter2
mm3 = millimeter3
cm  = centimeter
cm2 = centimeter2
cm3 = centimeter3
m  = meter
m2 = meter2
m3 = meter3
km  = kilometer
km2 = kilometer2
km3 = kilometer3
pc = parsec


# ========================================================================================================================================================
# The next code allows to execute this file
# ========================================================================================================================================================

#! /usr/bin/env python3
desc = """
Run eicrecon with reco_flags.py

    python3 reco_flags.py input_file.edm4hep.root output_name_no_ext
    
Script should successfully run and create files:

    output_name_no_ext.edm4eic.root    # with output flat tree
    output_name_no_ext.ana.root        # with histograms and other things filled by monitoring plugins
    
One can add -n/--nevents file with the number of events to process    
"""

import io
from pprint import pprint
import os
import sys
import subprocess
from datetime import datetime
import argparse

# For some values we need to eval the result
known_units_list = ['eV', 'MeV', 'GeV', 'mm', 'cm', 'mrad']

def has_unit_conversion(value):
    """Checks if string value use units like X*MeV or X/GeV"""
    for unit_name in known_units_list:
        if f'*{unit_name}' in value or \
                f'* {unit_name}' in value or \
                f'/{unit_name}' in value or \
                f'/ {unit_name}' in value:
            return True
    return False


def value_eval(value):
    """Converts string value with units or capacityBitsADC to number"""

    if has_unit_conversion(value):
        # need evaluation of unit conversion
        return str(eval(value))

    if 'capacityBitsADC' in value:
        # Value given in a form of 'capacityBitsADC=8'
        capacity_bits = 2 ** int(value.split("=")[1])
        return str(capacity_bits)

    return value


def make_flags_from_records():
    flags_arguments = []
    for record in eicrecon_reco_flags:
        if record[1]:
            value = value_eval(record[1])
            flags_arguments.append(f'-P{record[0]}={value}')
    return flags_arguments


if __name__ == "__main__":

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

    flags_arguments = make_flags_from_records()

    # Add reco_flags
    run_command.extend(flags_arguments)

    # RUN EICrecon
    start_time = datetime.now()
    subprocess.run(" ".join(run_command), shell=True, check=True)
    end_time = datetime.now()

    # Print execution time
    print("Start date and time : {}".format(start_time.strftime('%Y-%m-%d %H:%M:%S')))
    print("End date and time   : {}".format(end_time.strftime('%Y-%m-%d %H:%M:%S')))
    print("Execution real time : {}".format(end_time - start_time))
