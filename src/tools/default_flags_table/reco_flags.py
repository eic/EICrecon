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
# It is possible to pass additional jana parameters with -P flags:
#    python3 reco_flags.py -Pjana:timeout=0 input.edm4hep.root output_no_ext
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
    ('BEMC:EcalBarrelSciGlassRawHits:input_tag',                         'EcalBarrelSciGlassHits',         'Name of input collection to use'),
    ('BEMC:EcalBarrelSciGlassRawHits:capacityADC',                       'capacityBitsADC=14',             '*'),
    ('BEMC:EcalBarrelSciGlassRawHits:dynamicRangeADC',                   '20*GeV',                         '*'),
    ('BEMC:EcalBarrelSciGlassRawHits:energyResolutions',                 '0.0,0.02,0.0',                   '*'),
    ('BEMC:EcalBarrelSciGlassRawHits:fieldRefNumbers',                   '',                               ''),
    ('BEMC:EcalBarrelSciGlassRawHits:geoServiceName',                    'ActsGeometryProvider',           ''),
    ('BEMC:EcalBarrelSciGlassRawHits:pedestalMean',                      '100',                            '*'),
    ('BEMC:EcalBarrelSciGlassRawHits:pedestalSigma',                     '1',                              '*'),
    ('BEMC:EcalBarrelSciGlassRawHits:readoutClass',                      '',                               ''),
    ('BEMC:EcalBarrelSciGlassRawHits:resolutionTDC',                     '1e-11',                          ''),
    ('BEMC:EcalBarrelSciGlassRawHits:scaleResponse',                     '1',                              ''),
    ('BEMC:EcalBarrelSciGlassRawHits:signalSumFields',                   '',                               ''),
    ('BEMC:EcalBarrelSciGlassRawHits:timeResolution',                    '0',                              ''),

    # Hits reco
    ('BEMC:EcalBarrelSciGlassRecHits:input_tag',                         'EcalBarrelSciGlassRawHits',      'Name of input collection to use'),
    ('BEMC:EcalBarrelSciGlassRecHits:capacityADC',                       'capacityBitsADC=14',             '*'),
    ('BEMC:EcalBarrelSciGlassRecHits:dynamicRangeADC',                   '20*GeV',                         '*'),
    ('BEMC:EcalBarrelSciGlassRecHits:pedestalMean',                      '100',                            '*'),
    ('BEMC:EcalBarrelSciGlassRecHits:pedestalSigma',                     '1',                              '*'),
    ('BEMC:EcalBarrelSciGlassRecHits:resolutionTDC',                     '1e-11',                          ''),
    ('BEMC:EcalBarrelSciGlassRecHits:samplingFraction',                  '0.98',                           '*'),
    ('BEMC:EcalBarrelSciGlassRecHits:thresholdFactor',                   '3',                              '*'),
    ('BEMC:EcalBarrelSciGlassRecHits:thresholdValue',                    '3',                              '*'),

    # clustering
    ('BEMC:EcalBarrelSciGlassProtoClusters:input_tag',             'EcalBarrelSciGlassRecHits',      'Name of input collection to use'),
    ('BEMC:EcalBarrelSciGlassProtoClusters:dimScaledLocalDistXY',  '1.8,1.8',                        ''),
    ('BEMC:EcalBarrelSciGlassProtoClusters:globalDistEtaPhi',      '',                               ''),
    ('BEMC:EcalBarrelSciGlassProtoClusters:globalDistRPhi',        '',                               ''),
    ('BEMC:EcalBarrelSciGlassProtoClusters:localDistXY',           '',                               ''),
    ('BEMC:EcalBarrelSciGlassProtoClusters:localDistXZ',           '',                               ''),
    ('BEMC:EcalBarrelSciGlassProtoClusters:localDistYZ',           '',                               ''),
    ('BEMC:EcalBarrelSciGlassProtoClusters:minClusterCenterEdep',  '30*MeV',                         '*'),
    ('BEMC:EcalBarrelSciGlassProtoClusters:minClusterHitEdep',     '1.0*MeV',                        '*'),
    ('BEMC:EcalBarrelSciGlassProtoClusters:sectorDist',            '5.0*cm',                         '*'),
    ('BEMC:EcalBarrelSciGlassProtoClusters:splitCluster',          '0',                              '*'),

    ('BEMC:EcalBarrelSciGlassClusters:input_protoclust_tag',             'EcalBarrelSciGlassProtoClusters',  'Name of input collection to use'),
    ('BEMC:EcalBarrelSciGlassClusters:depthCorrection',                  '0',                              ''),
    ('BEMC:EcalBarrelSciGlassClusters:enableEtaBounds',                  '1',                              '*'),
    ('BEMC:EcalBarrelSciGlassClusters:energyWeight',                     'log',                            ''),
    ('BEMC:EcalBarrelSciGlassClusters:input_simhit_tag',                 'EcalBarrelSciGlassHits',         ''),
    ('BEMC:EcalBarrelSciGlassClusters:logWeightBase',                    '6.2',                            '*'),
    ('BEMC:EcalBarrelSciGlassClusters:moduleDimZName',                   '',                               ''),
    ('BEMC:EcalBarrelSciGlassClusters:samplingFraction',                 '1',                              '*'),

    ('BEMC:EcalBarrelSciGlassTruthClusters:input_protoclust_tag',        'EcalBarrelSciGlassTruthProtoClusters',  'Name of input collection to use'),
    ('BEMC:EcalBarrelSciGlassTruthClusters:depthCorrection',             '0',                              ''),
    ('BEMC:EcalBarrelSciGlassTruthClusters:enableEtaBounds',             '1',                              '*'),
    ('BEMC:EcalBarrelSciGlassTruthClusters:energyWeight',                'log',                            ''),
    ('BEMC:EcalBarrelSciGlassTruthClusters:input_simhit_tag',            'EcalBarrelSciGlassHits',         ''),
    ('BEMC:EcalBarrelSciGlassTruthClusters:logWeightBase',               '6.2',                            '*'),
    ('BEMC:EcalBarrelSciGlassTruthClusters:moduleDimZName',              '',                               ''),
    ('BEMC:EcalBarrelSciGlassTruthClusters:samplingFraction',            '1',                              '*'),

    # BEMC - Imaging Barrel
    #------------------
    ('BEMC:EcalBarrelImagingRawHits:input_tag',                 'EcalBarrelImagingHits',           'Name of input collection to use'),
    ('BEMC:EcalBarrelImagingRawHits:energyResolutions',         '0.0,0.02,0.0',                    ''),
    ('BEMC:EcalBarrelImagingRawHits:timeResolution',            '0.0*ns',                          ''),
    ('BEMC:EcalBarrelImagingRawHits:capacityADC',               'capacityBitsADC=13',              ''),
    ('BEMC:EcalBarrelImagingRawHits:dynamicRangeADC',           '3*MeV',                           ''),
    ('BEMC:EcalBarrelImagingRawHits:pedestalMean',              '100',                             ''),
    ('BEMC:EcalBarrelImagingRawHits:pedestalSigma',             '14',                              ''),
    ('BEMC:EcalBarrelImagingRawHits:resolutionTDC',             '10*ps',                           ''),
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
    ('BEMC:EcalBarrelScFiRawHits:resolutionTDC',                 '10*ps',                          ''),
    ('BEMC:EcalBarrelScFiRawHits:scaleResponse',                 '1.0',                            ''),
    ('BEMC:EcalBarrelScFiRawHits:signalSumFields',               '',                               ''),
    ('BEMC:EcalBarrelScFiRawHits:fieldRefNumbers',               '',                               ''),
    ('BEMC:EcalBarrelScFiRawHits:geoServiceName',                'ActsGeometryProvider',           ''),
    ('BEMC:EcalBarrelScFiRawHits:readoutClass',                  'light_guide',                    ''),

    ('BEMC:EcalBarrelImagingRecHits:input_tag',                 'EcalBarrelImagingRawHits',        'Name of input collection to use'),
    ('BEMC:EcalBarrelImagingRecHits:layerField',                'layer',                           ''),
    ('BEMC:EcalBarrelImagingRecHits:sectorField',               'module',                          ''),
    ('BEMC:EcalBarrelImagingRecHits:capacityADC',               'capacityBitsADC=13',              ''),
    ('BEMC:EcalBarrelImagingRecHits:pedestalMean',              '100',                             ''),
    ('BEMC:EcalBarrelImagingRecHits:dynamicRangeADC',           '3*MeV',                           ''),
    ('BEMC:EcalBarrelImagingRecHits:pedSigmaADC',               '14',                              ''),
    ('BEMC:EcalBarrelImagingRecHits:thresholdFactor',           '3.0',                             ''),
    ('BEMC:EcalBarrelImagingRecHits:samplingFraction',          '0.005',                           ''),

    ('BEMC:EcalBarrelScFiRecHits:input_tag',                    'EcalBarrelScFiRawHits',           'Name of input collection to use'),
    ('BEMC:EcalBarrelScFiRecHits:readout',                      'EcalBarrelScFiHits',              ''),
    ('BEMC:EcalBarrelScFiRecHits:layerField',                   'layer',                           ''),
    ('BEMC:EcalBarrelScFiRecHits:sectorField',                  'module',                          ''),
    ('BEMC:EcalBarrelScFiRecHits:capacityADC',                  'capacityBitsADC=14',              ''),
    ('BEMC:EcalBarrelScFiRecHits:dynamicRangeADC',              '750*MeV',                         ''),
    ('BEMC:EcalBarrelScFiRecHits:pedestalMean',                 '20',                              ''),
    ('BEMC:EcalBarrelScFiRecHits:pedestalSigma',                '0.3',                             ''),
    ('BEMC:EcalBarrelScFiRecHits:resolutionTDC',                '10*ps',                           ''),
    ('BEMC:EcalBarrelScFiRecHits:thresholdFactor',              '5.0',                             ''),
    ('BEMC:EcalBarrelScFiRecHits:thresholdValue',               '0.0',                             ''),
    ('BEMC:EcalBarrelScFiRecHits:samplingFraction',             '0.125',                           ''),

    ('BEMC:EcalBarrelscFiMergedHits:input_tag',                 'EcalBarrelScFiRecHits',           ''),
    ('BEMC:EcalBarrelscFiMergedHits:fields',                    'fiber,z',                         ''),
    ('BEMC:EcalBarrelscFiMergedHits:refs',                      '1,1',                             ''),

    ('BEMC:EcalBarrelImagingProtoClusters:input_tag',           'EcalBarrelImagingRecHits',        'Name of input collection to use'),
    ('BEMC:EcalBarrelImagingProtoClusters::localDistXY',        '2.0,2.0',                         '* [mm]'),
    ('BEMC:EcalBarrelImagingProtoClusters::layerDistEtaPhi',    '0.01,0.01',                       '* [radian]'),
    ('BEMC:EcalBarrelImagingProtoClusters::neighbourLayersRange', '2.0',                           ''),
    ('BEMC:EcalBarrelImagingProtoClusters::sectorDist',         '30.0*mm',                          ''),
    ('BEMC:EcalBarrelImagingProtoClusters::minClusterHitEdep',  '0.',                              ''),
    ('BEMC:EcalBarrelImagingProtoClusters::minClusterCenterEdep', '0.',                            ''),
    ('BEMC:EcalBarrelImagingProtoClusters::minClusterEdep',     '80*MeV',                          ''),
    ('BEMC:EcalBarrelImagingProtoClusters::minClusterNhits',    '5',                               ''),

    ('BEMC:EcalBarrelScFiProtoClusters:input_tag',              'EcalBarrelScFiMergedHits', 'Name of input collection to use'),
    ('BEMC:EcalBarrelScFiProtoClusters:splitCluster',           'false',                           ''),
    ('BEMC:EcalBarrelScFiProtoClusters:minClusterHitEdep',      '1.0*MeV',                         ''),
    ('BEMC:EcalBarrelScFiProtoClusters:minClusterCenterEdep',   '10.0*MeV',                        ''),
    ('BEMC:EcalBarrelScFiProtoClusters:sectorDist',             '50.0*mm',                          ''),
    ('BEMC:EcalBarrelScFiProtoClusters:localDistXY',            '',                                ''),
    ('BEMC:EcalBarrelScFiProtoClusters:localDistXZ',            '30.0,30.0',                       '* [mm]'),
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
    ('EEMC:EcalEndcapNIslandProtoClusters:minClusterCenterEdep', '30.0*MeV',                        '*'),
    ('EEMC:EcalEndcapNIslandProtoClusters:minClusterHitEdep',    '1.0*MeV',                         '*'),
    ('EEMC:EcalEndcapNIslandProtoClusters:sectorDist',           '5.0*cm',                         '*'),
    ('EEMC:EcalEndcapNIslandProtoClusters:splitCluster',         '0',                              '*'),

    ('EEMC:EcalEndcapNTruthClusters:depthCorrection',            '0',                              ''),
    ('EEMC:EcalEndcapNTruthClusters:enableEtaBounds',            '0',                              ''),
    ('EEMC:EcalEndcapNTruthClusters:energyWeight',               'log',                            ''),
    ('EEMC:EcalEndcapNTruthClusters:logWeightBase',              '4.6',                            '*'),
    ('EEMC:EcalEndcapNTruthClusters:moduleDimZName',             '',                               ''),
    ('EEMC:EcalEndcapNTruthClusters:samplingFraction',           '0.03',                           '*'),

    ('EEMC:EcalEndcapNClusters:input_protoclust_tag',            'EcalEndcapNIslandProtoClusters', ''),
    ('EEMC:EcalEndcapNClusters:samplingFraction',                '1',                              ''),
    ('EEMC:EcalEndcapNClusters:logWeightBase',                   '3.6',                            ''),
    ('EEMC:EcalEndcapNClusters:depthCorrection',                 '0',                              ''),
    ('EEMC:EcalEndcapNClusters:energyWeight',                    'log',                            ''),
    ('EEMC:EcalEndcapNClusters:moduleDimZName',                  '',                               ''),
    ('EEMC:EcalEndcapNClusters:enableEtaBounds',                 '0',                              ''),

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
    ('EEMC:EcalEndcapPRawHits:scaleResponse',                    '0.03',                           ''),
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
    ('EEMC:EcalEndcapPRecHits:sectorField',                      '',                               ''),
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

    ('EEMC:EcalEndcapPClusters:input_protoclust_tag',            'EcalEndcapPIslandProtoClusters', ''),
    ('EEMC:EcalEndcapPClusters:samplingFraction',                '1',                              ''),
    ('EEMC:EcalEndcapPClusters:logWeightBase',                   '3.6',                            ''),
    ('EEMC:EcalEndcapPClusters:depthCorrection',                 '0',                              ''),
    ('EEMC:EcalEndcapPClusters:energyWeight',                    'log',                            ''),
    ('EEMC:EcalEndcapPClusters:moduleDimZName',                  '',                               ''),
    ('EEMC:EcalEndcapPClusters:enableEtaBounds',                 '0',                              ''),

    # Positive Endcap insert
    ('EEMC:EcalEndcapPInsertRawHits:capacityADC',                      'capacityBitsADC=14',             '*'),
    ('EEMC:EcalEndcapPInsertRawHits:dynamicRangeADC',                  '3*GeV',                          '*'),
    ('EEMC:EcalEndcapPInsertRawHits:energyResolutions',                '0.00316,0.0015,0.0',             '*'),
    ('EEMC:EcalEndcapPInsertRawHits:fieldRefNumbers',                  '1,1',                            '*'),
    ('EEMC:EcalEndcapPInsertRawHits:geoServiceName',                   'ActsGeometryProvider',           ''),
    ('EEMC:EcalEndcapPInsertRawHits:pedestalMean',                     '100',                            '*'),
    ('EEMC:EcalEndcapPInsertRawHits:pedestalSigma',                    '0.7',                            '*'),
    ('EEMC:EcalEndcapPInsertRawHits:readoutClass',                     '',                               ''),
    ('EEMC:EcalEndcapPInsertRawHits:resolutionTDC',                    '1e-11',                          ''),
    ('EEMC:EcalEndcapPInsertRawHits:scaleResponse',                    '0.03',                           ''),
    ('EEMC:EcalEndcapPInsertRawHits:signalSumFields',                  '',                               ''),
    ('EEMC:EcalEndcapPInsertRawHits:timeResolution',                   '0',                              ''),

    ('EEMC:EcalEndcapPInsertRecHits:capacityADC',                      'capacityBitsADC=14',             '*'),
    ('EEMC:EcalEndcapPInsertRecHits:dynamicRangeADC',                  '3*GeV',                          '*'),
    ('EEMC:EcalEndcapPInsertRecHits:geoServiceName',                   'geoServiceName',                 ''),
    ('EEMC:EcalEndcapPInsertRecHits:layerField',                       '',                               ''),
    ('EEMC:EcalEndcapPInsertRecHits:localDetElement',                  '',                               ''),
    ('EEMC:EcalEndcapPInsertRecHits:localDetFields',                   '',                               ''),
    ('EEMC:EcalEndcapPInsertRecHits:pedestalMean',                     '100',                            '*'),
    ('EEMC:EcalEndcapPInsertRecHits:pedestalSigma',                    '0.7',                            '*'),
    ('EEMC:EcalEndcapPInsertRecHits:readout',                          'EcalEndcapPInsertHits',          ''),
    ('EEMC:EcalEndcapPInsertRecHits:resolutionTDC',                    '1e-11',                          ''),
    ('EEMC:EcalEndcapPInsertRecHits:samplingFraction',                 '0.03',                           '*'),
    ('EEMC:EcalEndcapPInsertRecHits:sectorField',                      '',                               ''),
    ('EEMC:EcalEndcapPInsertRecHits:thresholdFactor',                  '5.0',                            '*'),
    ('EEMC:EcalEndcapPInsertRecHits:thresholdValue',                   '2',                              '*'),

    ('EEMC:EcalEndcapPInsertIslandProtoClusters:dimScaledLocalDistXY', '1.5,1.5',                        ''),
    ('EEMC:EcalEndcapPInsertIslandProtoClusters:globalDistEtaPhi',     '',                               ''),
    ('EEMC:EcalEndcapPInsertIslandProtoClusters:globalDistRPhi',       '',                               ''),
    ('EEMC:EcalEndcapPInsertIslandProtoClusters:localDistXY',          '10,10',                          '* [mm]'),
    ('EEMC:EcalEndcapPInsertIslandProtoClusters:localDistXZ',          '',                               ''),
    ('EEMC:EcalEndcapPInsertIslandProtoClusters:localDistYZ',          '',                               ''),
    ('EEMC:EcalEndcapPInsertIslandProtoClusters:minClusterCenterEdep', '10.0*MeV',                       '*'),
    ('EEMC:EcalEndcapPInsertIslandProtoClusters:minClusterHitEdep',    '0',                              ''),
    ('EEMC:EcalEndcapPInsertIslandProtoClusters:sectorDist',           '5',                              ''),
    ('EEMC:EcalEndcapPInsertIslandProtoClusters:splitCluster',         '0',                              ''),

    ('EEMC:EcalEndcapPInsertTruthClusters:depthCorrection',            '0',                              ''),
    ('EEMC:EcalEndcapPInsertTruthClusters:enableEtaBounds',            '1',                              '*'),
    ('EEMC:EcalEndcapPInsertTruthClusters:energyWeight',               'log',                            ''),
    ('EEMC:EcalEndcapPInsertTruthClusters:logWeightBase',              '6.2',                            '*'),
    ('EEMC:EcalEndcapPInsertTruthClusters:moduleDimZName',             '',                               ''),
    ('EEMC:EcalEndcapPInsertTruthClusters:samplingFraction',           '1',                              ''),

    ('EEMC:EcalEndcapPInsertClusters:input_protoclust_tag',            'EcalEndcapPInsertIslandProtoClusters', ''),
    ('EEMC:EcalEndcapPInsertClusters:samplingFraction',                '1',                              ''),
    ('EEMC:EcalEndcapPInsertClusters:logWeightBase',                   '3.6',                            ''),
    ('EEMC:EcalEndcapPInsertClusters:depthCorrection',                 '0',                              ''),
    ('EEMC:EcalEndcapPInsertClusters:energyWeight',                    'log',                            ''),
    ('EEMC:EcalEndcapPInsertClusters:moduleDimZName',                  '',                               ''),
    ('EEMC:EcalEndcapPInsertClusters:enableEtaBounds',                 '0',                              ''),

    # B0ECAL - Far forward B0 Ecal
    # -----------------
    ('B0ECAL:B0ECalRawHits:capacityADC',                      'capacityBitsADC=14',                      '*'),
    ('B0ECAL:B0ECalRawHits:dynamicRangeADC',                  '20*GeV',                                   '*'),
    ('B0ECAL:B0ECalRawHits:energyResolutions',                '0.0,0.02,0.0',                            '*'),
    ('B0ECAL:B0ECalRawHits:fieldRefNumbers',                  '',                                        ''),
    ('B0ECAL:B0ECalRawHits:geoServiceName',                   '',                                        ''),
    ('B0ECAL:B0ECalRawHits:pedestalMean',                     '100',                                     '*'),
    ('B0ECAL:B0ECalRawHits:pedestalSigma',                    '1',                                       ''),
    ('B0ECAL:B0ECalRawHits:readoutClass',                     '',                                        ''),
    ('B0ECAL:B0ECalRawHits:resolutionTDC',                    '1e-11',                                   ''),
    ('B0ECAL:B0ECalRawHits:scaleResponse',                    '1',                                       ''),
    ('B0ECAL:B0ECalRawHits:signalSumFields',                  '',                                        ''),
    ('B0ECAL:B0ECalRawHits:timeResolution',                   '0',                                       ''),

    ('B0ECAL:B0ECalRecHits:capacityADC',                      'capacityBitsADC=14',                      '*'),
    ('B0ECAL:B0ECalRecHits:dynamicRangeADC',                  '20*GeV',                                  '*'),
    ('B0ECAL:B0ECalRecHits:geoServiceName',                   '',                                        ''),
    ('B0ECAL:B0ECalRecHits:layerField',                       '',                                        ''),
    ('B0ECAL:B0ECalRecHits:localDetElement',                  '',                                        ''),
    ('B0ECAL:B0ECalRecHits:localDetFields',                   '',                                        ''),
    ('B0ECAL:B0ECalRecHits:pedestalMean',                     '100',                                     '*'),
    ('B0ECAL:B0ECalRecHits:pedestalSigma',                    '1',                                       '*'),
    ('B0ECAL:B0ECalRecHits:readout',                          'B0ECalHits',                              ''),
    ('B0ECAL:B0ECalRecHits:resolutionTDC',                    '1e-11',                                   ''),
    ('B0ECAL:B0ECalRecHits:samplingFraction',                 '0.998',                                   '*'),
    ('B0ECAL:B0ECalRecHits:sectorField',                      'sector',                                  ''),
    ('B0ECAL:B0ECalRecHits:thresholdFactor',                  '4',                                       '*'),
    ('B0ECAL:B0ECalRecHits:thresholdValue',                   '3',                                       '*'),

    ('B0ECAL:B0ECalIslandProtoClusters:input_tag',            'B0ECalRecHits',                           ''),
    ('B0ECAL:B0ECalIslandProtoClusters:splitCluster',         '0',                              '*'),
    ('B0ECAL:B0ECalIslandProtoClusters:minClusterHitEdep',    '1.0*MeV',                        '*'),
    ('B0ECAL:B0ECalIslandProtoClusters:minClusterCenterEdep', '30.0*MeV',                       '*'),
    ('B0ECAL:B0ECalIslandProtoClusters:sectorDist',           '5.0*cm',                         '*'),
    ('B0ECAL:B0ECalIslandProtoClusters:localDistXY',          '',                               ''),
    ('B0ECAL:B0ECalIslandProtoClusters:localDistXZ',          '',                               ''),
    ('B0ECAL:B0ECalIslandProtoClusters:localDistYZ',          '',                               ''),
    ('B0ECAL:B0ECalIslandProtoClusters:globalDistRPhi',       '',                               ''),
    ('B0ECAL:B0ECalIslandProtoClusters:globalDistEtaPhi',     '',                               ''),
    ('B0ECAL:B0ECalIslandProtoClusters:dimScaledLocalDistXY', '1.8,1.8',                        '*'),

    ('B0ECAL:B0ECalClusters:input_protoclust_tag',            'B0ECalIslandProtoClusters',      ''),
    ('B0ECAL:B0ECalClusters:samplingFraction',                '1',                              ''),
    ('B0ECAL:B0ECalClusters:logWeightBase',                   '3.6',                            ''),
    ('B0ECAL:B0ECalClusters:depthCorrection',                 '0',                              ''),
    ('B0ECAL:B0ECalClusters:energyWeight',                    'log',                            ''),
    ('B0ECAL:B0ECalClusters:moduleDimZName',                  '',                               ''),
    ('B0ECAL:B0ECalClusters:enableEtaBounds',                 '0',                              ''),

    # HCAL Barrel
    # -----------
    ('HCAL:HcalBarrelRawHits:capacityADC',                       'capacityBitsADC=8',              '*'),
    ('HCAL:HcalBarrelRawHits:dynamicRangeADC',                   '50*MeV',                         '*'),
    ('HCAL:HcalBarrelRawHits:energyResolutions',                 '',                               ''),
    ('HCAL:HcalBarrelRawHits:fieldRefNumbers',                   '1,0',                            '*'),
    ('HCAL:HcalBarrelRawHits:geoServiceName',                    'ActsGeometryProvider',           ''),
    ('HCAL:HcalBarrelRawHits:pedestalMean',                      '10',                             '*'),
    ('HCAL:HcalBarrelRawHits:pedestalSigma',                     '2',                              '*'),
    ('HCAL:HcalBarrelRawHits:readoutClass',                      '',                               ''),
    ('HCAL:HcalBarrelRawHits:resolutionTDC',                     '1e-9',                           ''),
    ('HCAL:HcalBarrelRawHits:scaleResponse',                     '1',                              ''),
    ('HCAL:HcalBarrelRawHits:signalSumFields',                   '',                               ''),
    ('HCAL:HcalBarrelRawHits:timeResolution',                    '0',                              ''),

    ('HCAL:HcalBarrelRecHits:capacityADC',                       'capacityBitsADC=8',              '*'),
    ('HCAL:HcalBarrelRecHits:dynamicRangeADC',                   '20*MeV',                         '*'),
    ('HCAL:HcalBarrelRecHits:geoServiceName',                    'geoServiceName',                 ''),
    ('HCAL:HcalBarrelRecHits:layerField',                        'tower',                          ''),
    ('HCAL:HcalBarrelRecHits:localDetElement',                   '',                               ''),
    ('HCAL:HcalBarrelRecHits:localDetFields',                    '',                               ''),
    ('HCAL:HcalBarrelRecHits:pedestalMean',                      '20',                            '*'),
    ('HCAL:HcalBarrelRecHits:pedestalSigma',                     '0.3',                            '*'),
    ('HCAL:HcalBarrelRecHits:readout',                           'HcalBarrelHits',                 ''),
    ('HCAL:HcalBarrelRecHits:resolutionTDC',                     '1e-11',                          ''),
    ('HCAL:HcalBarrelRecHits:samplingFraction',                  '0.033',                          ''),
    ('HCAL:HcalBarrelRecHits:sectorField',                       'sector',                         ''),
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
    ('HCAL:HcalBarrelIslandProtoClusters:minClusterCenterEdep',  '30.0*MeV',                       ''),
    ('HCAL:HcalBarrelIslandProtoClusters:minClusterHitEdep',     '3.0*MeV',                       '*'),
    ('HCAL:HcalBarrelIslandProtoClusters:sectorDist',            '5',                              ''),
    ('HCAL:HcalBarrelIslandProtoClusters:splitCluster',          '0',                              '*'),

    ('HCAL:HcalBarrelMergedHits:fields',                         'tower,tile',                     ''),
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
    ('HCAL:HcalEndcapNRawHits:capacityADC',                      'capacityBitsADC=10',             '*'),
    ('HCAL:HcalEndcapNRawHits:dynamicRangeADC',                  '3.6*MeV',                            '*'),
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

    ('HCAL:HcalEndcapNRecHits:capacityADC',                      'capacityBitsADC=10',             '*'),
    ('HCAL:HcalEndcapNRecHits:dynamicRangeADC',                  '3.6*MeV',                            '*'),
    ('HCAL:HcalEndcapNRecHits:geoServiceName',                   'geoServiceName',                 ''),
    ('HCAL:HcalEndcapNRecHits:layerField',                       '',                               ''),
    ('HCAL:HcalEndcapNRecHits:localDetElement',                  '',                               ''),
    ('HCAL:HcalEndcapNRecHits:localDetFields',                   '',                               ''),
    ('HCAL:HcalEndcapNRecHits:pedestalMean',                     '20',                            '*'),
    ('HCAL:HcalEndcapNRecHits:pedestalSigma',                    '0.3',                            '*'),
    ('HCAL:HcalEndcapNRecHits:readout',                          'HcalEndcapNHits',                ''),
    ('HCAL:HcalEndcapNRecHits:resolutionTDC',                    '1e-11',                          ''),
    ('HCAL:HcalEndcapNRecHits:samplingFraction',                 '0.998',                          ''),
    ('HCAL:HcalEndcapNRecHits:sectorField',                      '',                               ''),
    ('HCAL:HcalEndcapNRecHits:thresholdFactor',                  '4',                              ''),
    ('HCAL:HcalEndcapNRecHits:thresholdValue',                   '1',                              '*'),

    ('HCAL:HcalEndcapNMergedHits:input_tag',                     'HcalEndcapNRecHits',             '*'),
    ('HCAL:HcalEndcapNMergedHits:readout',                       'HcalEndcapNHits',                '*'),
    ('HCAL:HcalEndcapNMergedHits:fields',                        'layer,slice',                    '*'),
    ('HCAL:HcalEndcapNMergedHits:refs',                          '1,0',                            '*'),

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
    ('HCAL:HcalEndcapPRecHits:readout',                          'HcalEndcapPHits',                ''),
    ('HCAL:HcalEndcapPRecHits:resolutionTDC',                    '1e-11',                          ''),
    ('HCAL:HcalEndcapPRecHits:samplingFraction',                 '0.025',                          ''),
    ('HCAL:HcalEndcapPRecHits:sectorField',                      '',                               ''),
    ('HCAL:HcalEndcapPRecHits:thresholdFactor',                  '5',                              '*'),
    ('HCAL:HcalEndcapPRecHits:thresholdValue',                   '3',                              '*'),

    ('HCAL:HcalEndcapPMergedHits:input_tag',                     'HcalEndcapPRecHits',             '*'),
    ('HCAL:HcalEndcapPMergedHits:readout',                       'HcalEndcapPHits',                '*'),
    ('HCAL:HcalEndcapPMergedHits:fields',                        'layer,slice',                    '*'),
    ('HCAL:HcalEndcapPMergedHits:refs',                          '1,0',                            '*'),

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
    ('HCAL:HcalEndcapPTruthClusters:input_protoclust_tag',       'HcalEndcapPTruthProtoClusters',  ''),
    ('HCAL:HcalEndcapPTruthClusters:input_simhit_tag',           'HcalEndcapPHits',                ''),
    ('HCAL:HcalEndcapPTruthClusters:logWeightBase',              '6.2',                            '*'),
    ('HCAL:HcalEndcapPTruthClusters:moduleDimZName',             '',                               ''),
    ('HCAL:HcalEndcapPTruthClusters:samplingFraction',           '0.025',                          '*'),

    ('HCAL:HcalEndcapPClusters:depthCorrection',                 '0',                              ''),
    ('HCAL:HcalEndcapPClusters:enableEtaBounds',                 '0',                              ''),
    ('HCAL:HcalEndcapPClusters:energyWeight',                    'log',                            ''),
    ('HCAL:HcalEndcapPClusters:input_protoclust_tag',            'HcalEndcapPIslandProtoClusters', ''),
    ('HCAL:HcalEndcapPClusters:input_simhit_tag',                'HcalEndcapPHits',                ''),
    ('HCAL:HcalEndcapPClusters:logWeightBase',                   '6.2',                            '*'),
    ('HCAL:HcalEndcapPClusters:moduleDimZName',                  '',                               ''),
    ('HCAL:HcalEndcapPClusters:samplingFraction',                '0.025',                          '*'),


    # HCAL - Positive Endcap insert
    # ------------------------------------------
    ('HCAL:HcalEndcapPInsertRawHits:capacityADC',                      'capacityBitsADC=15',                    '*'),
    ('HCAL:HcalEndcapPInsertRawHits:dynamicRangeADC',                  '200*MeV',                               '*'),
    ('HCAL:HcalEndcapPInsertRawHits:energyResolutions',                '',                                      ''),
    ('HCAL:HcalEndcapPInsertRawHits:fieldRefNumbers',                  '',                                      ''),
    ('HCAL:HcalEndcapPInsertRawHits:geoServiceName',                   'ActsGeometryProvider',                  ''),
    ('HCAL:HcalEndcapPInsertRawHits:pedestalMean',                     '400',                                   '*'),
    ('HCAL:HcalEndcapPInsertRawHits:pedestalSigma',                    '10',                                    '*'),
    ('HCAL:HcalEndcapPInsertRawHits:readoutClass',                     '',                                      ''),
    ('HCAL:HcalEndcapPInsertRawHits:resolutionTDC',                    '1e-11',                                 ''),
    ('HCAL:HcalEndcapPInsertRawHits:scaleResponse',                    '1',                                     ''),
    ('HCAL:HcalEndcapPInsertRawHits:signalSumFields',                  '',                                      ''),
    ('HCAL:HcalEndcapPInsertRawHits:timeResolution',                   '0',                                     ''),

    ('HCAL:HcalEndcapPInsertRecHits:capacityADC',                      'capacityBitsADC=15',                    '*'),
    ('HCAL:HcalEndcapPInsertRecHits:dynamicRangeADC',                  '200*MeV',                               '*'),
    ('HCAL:HcalEndcapPInsertRecHits:geoServiceName',                   'geoServiceName',                        ''),
    ('HCAL:HcalEndcapPInsertRecHits:layerField',                       '',                                      ''),
    ('HCAL:HcalEndcapPInsertRecHits:localDetElement',                  '',                                      ''),
    ('HCAL:HcalEndcapPInsertRecHits:localDetFields',                   '',                                      ''),
    ('HCAL:HcalEndcapPInsertRecHits:pedestalMean',                     '400',                                   '*'),
    ('HCAL:HcalEndcapPInsertRecHits:pedestalSigma',                    '10',                                    '*'),
    ('HCAL:HcalEndcapPInsertRecHits:readout',                          'HcalEndcapPInsertHits',                 ''),
    ('HCAL:HcalEndcapPInsertRecHits:resolutionTDC',                    '1e-11',                                 ''),
    ('HCAL:HcalEndcapPInsertRecHits:samplingFraction',                 '0.998',                                 ''),
    ('HCAL:HcalEndcapPInsertRecHits:sectorField',                      '',                                      ''),
    ('HCAL:HcalEndcapPInsertRecHits:thresholdFactor',                  '4',                                     ''),
    ('HCAL:HcalEndcapPInsertRecHits:thresholdValue',                   '0',                                     '*'),

    ('HCAL:HcalEndcapPInsertIslandProtoClusters:dimScaledLocalDistXY', '1.5,1.5',                               ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:globalDistEtaPhi',     '',                                      ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:globalDistRPhi',       '',                                      ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:localDistXY',          '',                                      ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:localDistXZ',          '',                                      ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:localDistYZ',          '',                                      ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:minClusterCenterEdep', '0.03*GeV',                              ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:minClusterHitEdep',    '0',                                     ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:sectorDist',           '5',                                     ''),
    ('HCAL:HcalEndcapPInsertIslandProtoClusters:splitCluster',         '1',                                     ''),

    ('HCAL:HcalEndcapPInsertTruthClusters:depthCorrection',            '0',                                     ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:enableEtaBounds',            '0',                                     ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:energyWeight',               'log',                                   ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:input_protoclust_tag',       'HcalEndcapPInsertTruthProtoClusters',   ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:input_simhit_tag',           'HcalEndcapPInsertHits',                 ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:logWeightBase',              '6.2',                                   '*'),
    ('HCAL:HcalEndcapPInsertTruthClusters:moduleDimZName',             '',                                      ''),
    ('HCAL:HcalEndcapPInsertTruthClusters:samplingFraction',           '1',                                     ''),

    ('HCAL:HcalEndcapPInsertClusters:depthCorrection',                 '0',                                     ''),
    ('HCAL:HcalEndcapPInsertClusters:enableEtaBounds',                 '0',                                     ''),
    ('HCAL:HcalEndcapPInsertClusters:energyWeight',                    'log',                                   ''),
    ('HCAL:HcalEndcapPInsertClusters:input_protoclust_tag',            'HcalEndcapPInsertIslandProtoClusters',  ''),
    ('HCAL:HcalEndcapPInsertClusters:input_simhit_tag',                'HcalEndcapPInsertHits',                 ''),
    ('HCAL:HcalEndcapPInsertClusters:logWeightBase',                   '6.2',                                   '*'),
    ('HCAL:HcalEndcapPInsertClusters:moduleDimZName',                  '',                                      ''),
    ('HCAL:HcalEndcapPInsertClusters:samplingFraction',                '1',                                     ''),


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
    ('ZDC:ZDCEcalRecHits:sectorField',                           '',                               ''),
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

    # ============================ P I D ===================================================

    # DRICH
    # -----------------------------
    ('DRICH:DRICHRawHits:seed',            '0',         'random number generator seed'                          ),
    ('DRICH:DRICHRawHits:hitTimeWindow',   '20.0*ns',   ''                                                      ),
    ('DRICH:DRICHRawHits:timeStep',        '0.0625*ns', ''                                                      ),
    ('DRICH:DRICHRawHits:speMean',         '80.0',      ''                                                      ),
    ('DRICH:DRICHRawHits:speError',        '16.0',      ''                                                      ),
    ('DRICH:DRICHRawHits:pedMean',         '200.0',     ''                                                      ),
    ('DRICH:DRICHRawHits:pedError',        '3.0',       ''                                                      ),
    ('DRICH:DRICHRawHits:enablePixelGaps', 'true',      'enable/disable removal of hits in gaps between pixels' ),
    ('DRICH:DRICHRawHits:pixelSize',       '3.0*mm',    'pixel (active) size'                                   ),
    ('DRICH:DRICHRawHits:safetyFactor',    '0.7',       'overall safety factor'                                 ),
    # ('DRICH:DRICHRawHits:quantumEfficiency', '...', 'quantum efficiency' ), # FIXME cannot define here; instead it is hard coded in src/algorithms/digi/PhotoMultiplierHitDigiConfig.h

    ('DRICH:DRICHAerogelTracks:numPlanes', '5',  'number of aerogel radiator track-projection planes' ),
    ('DRICH:DRICHGasTracks:numPlanes',     '10', 'number of gas radiator track-projection planes'     ),

    # ============================ F A R   F O R W A R D ===================================
    
    # RPOTS
    # -----------------------------
    ('RPOTS:ForwardRomanPotRawHits:threshold',                   '0.0',                            ''),
    ('RPOTS:ForwardRomanPotRawHits:timeResolution',              '8.0',                            'ns'),
    
    ('RPOTS:ForwardRomanPotRecHits:time_resolution',             '0.0',                            ''),

    ('RPOTS:ForwardRomanPotParticles:local_x_offset_station_1',  '-833.3878326',                   ''),
    ('RPOTS:ForwardRomanPotParticles:local_x_offset_station_2',  '-924.342804',                    ''),
    ('RPOTS:ForwardRomanPotParticles:local_x_slope_offset',      '-0.00622147',                    ''),
    ('RPOTS:ForwardRomanPotParticles:local_y_slope_offset',      '-0.0451035',                     ''),
    ('RPOTS:ForwardRomanPotParticles:crossingAngle',             '-0.025',                         ''),
    ('RPOTS:ForwardRomanPotParticles:nomMomentum',               '275.0',                          ''),
    ('RPOTS:ForwardRomanPotParticles:m_readout',                 '',                               ''),
    ('RPOTS:ForwardRomanPotParticles:m_layerField',              '',                               ''),
    ('RPOTS:ForwardRomanPotParticles:m_sectorField',             '',                               ''),
    ('RPOTS:ForwardRomanPotParticles:m_localDetElement',         '',                               ''),
    ('RPOTS:ForwardRomanPotParticles:u_localDetFields',          '',                               ''),


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
megaelectronvolt = 1.e-3
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
millimeter  = 0.1
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
# Time [T]
#
second = 1.
millisecond = 1.e-3*second
microsecond = 1.e-6*second
nanosecond = 1.e-9*second
picosecond = 1.e-12*second
# symbols
ms = millisecond
us = microsecond
ns = nanosecond
ps = picosecond
# Angle [A]
#
radian = 1.
mrad = radian/1000.0
degree = 0.017453292519943*radian


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
known_units_list = ['eV', 'MeV', 'GeV', 'mm', 'cm', 'mrad', 'ns', 'ps']

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
        else:
            flags_arguments.append(f'-P{record[0]}=""')
    return flags_arguments


if __name__ == "__main__":

    # Separate all parameters that starts with -P/-p from args
    parameter_args = []
    sys_argv = sys.argv.copy()
    for arg in sys_argv:
        if arg.startswith(("-P", "-p")):
            parameter_args.append(arg)
            sys.argv.remove(arg)

    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument('input_file', help="Input file name")
    parser.add_argument('output_base_name', help="Output files names (no file extensions here)")
    parser.add_argument('-n', '--nevents', default="0", help="Number of events to process")
    args = parser.parse_args()

    run_command = [
        f"eicrecon"
    ]

    default_parameters = [
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

    # Add parameters from args
    run_command.extend(parameter_args)

    # Add default parameters
    run_command.extend(default_parameters)

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
