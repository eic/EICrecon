// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factoryT.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factoryT.h"
#include "factories/calorimetry/HEXPLIT_factoryT.h"
#include "factories/calorimetry/LogWeightReco_factoryT.h"


extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
         "ZDCRawHits", {"HcalFarForwardZDCHits"}, {"ZDCRawHits"},
          {
            .tRes = 0.0 * dd4hep::ns,
            .capADC = 32768,
            .dyRangeADC = 200 * dd4hep::MeV,
            .pedMeanADC = 400,
            .pedSigmaADC = 10,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .corrMeanScale = 1.0,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
          "ZDCRecHits", {"ZDCRawHits"}, {"ZDCRecHits"},
          {
            .capADC = 32678,
            .dyRangeADC = 200. * dd4hep::MeV,
            .pedMeanADC = 400,
            .pedSigmaADC = 10,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 0.0,
            .thresholdValue = -100.0,
            .sampFrac = 1.0,
            .readout = "HcalFarForwardZDCHits",
          },
          app   // TODO: Remove me once fixed
        ));
	
	app->Add(new JChainMultifactoryGeneratorT<HEXPLIT_factoryT>(
          "ZDCSubcellHits", {"ZDCRecHits"}, {"ZDCSubcellHits"},
          {
            .layer_spacing=25.1*dd4hep::mm,
            .side_length=31.3 *dd4hep::mm,
            .MIP = 470. * dd4hep::keV,
	    .Emin= 470./10 * dd4hep::keV,
            .tmax=320 * dd4hep::ns,
	    .rot_x=0,
	    .rot_y=-0.25,
	    .rot_z=0,
	    .trans_x=0,
	    .trans_y=0,
	    .trans_z=36601 * dd4hep::mm,
          },
          app   // TODO: Remove me once fixed
	));

	app->Add(new JChainMultifactoryGeneratorT<LogWeightReco_factoryT>(
	  "ZDCLogWeightClusters", {"ZDCSubcellHits"}, {"ZDCLogWeightClusters"},
          {
            .sampling_fraction=0.0203,
            .E0=50. * dd4hep::GeV,
            .w0_a=5.0,
	    .w0_b=0.65,
	    .w0_c=0.31,
          },
          app   // TODO: Remove me once fixed
	));
	
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterTruthClustering_factoryT>(
          "ZDCTruthProtoClusters", {"ZDCRecHits", "ZDCHits"}, {"ZDCTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterIslandCluster_factoryT>(
          "ZDCIslandProtoClusters", {"ZDCRecHits"}, {"ZDCIslandProtoClusters"},
          {
            .sectorDist = 5.0 * dd4hep::cm,
            .localDistXY = {50 * dd4hep::cm, 50 * dd4hep::cm},
            .dimScaledLocalDistXY = {50.0*dd4hep::mm, 50.0*dd4hep::mm},
            .splitCluster = true,
            .minClusterHitEdep = 0.1 * dd4hep::MeV,
            .minClusterCenterEdep = 3.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "globalDistEtaPhi",
            .transverseEnergyProfileScale = 1.,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
             "ZDCTruthClusters",
            {"ZDCTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "ZDCHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"ZDCTruthClusters",             // edm4eic::Cluster
             "ZDCTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
             "ZDCClusters",
            {"ZDCIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "ZDCHits"},                // edm4hep::SimCalorimeterHitCollection
            {"ZDCClusters",             // edm4eic::Cluster
             "ZDCClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );
    }
}
