// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "RawCalorimeterHit_factory_EcalBarrelRawHits.h"
#include "CalorimeterHit_factory_EcalBarrelRecHits.h"
#include "ProtoCluster_factory_EcalBarrelIslandProtoClusters.h"
#include "Cluster_factory_EcalBarrelClusters.h"
#include "Cluster_factory_EcalBarrelMergedClusters.h"

#include "RawCalorimeterHit_factory_EcalBarrelScFiRawHits.h"
#include "CalorimeterHit_factory_EcalBarrelScFiRecHits.h"
#include "CalorimeterHit_factory_EcalBarrelScFiMergedHits.h"
#include "ProtoCluster_factory_EcalBarrelScFiProtoClusters.h"
#include "Cluster_factory_EcalBarrelScFiClusters.h"

#include "RawCalorimeterHit_factory_EcalBarrelImagingRawHits.h"
#include "CalorimeterHit_factory_EcalBarrelImagingRecHits.h"
#include "ProtoCluster_factory_EcalBarrelImagingProtoClusters.h"
#include "Cluster_factory_EcalBarrelImagingClusters.h"
#include "Cluster_factory_EcalBarrelImagingMergedClusters.h"

#include "ProtoCluster_factory_EcalBarrelTruthProtoClusters.h"
#include "Cluster_factory_EcalBarrelTruthClusters.h"
#include "Cluster_factory_EcalBarrelMergedTruthClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelMergedClusters>());

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelScFiRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelScFiRecHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelScFiMergedHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelScFiProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelScFiClusters>());

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelImagingRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelImagingRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelImagingProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelImagingClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelImagingMergedClusters>());

        // Inserted types (so they can be written to output podio file)
        app->Add(new JFactoryGeneratorT<JFactoryT<edm4eic::Cluster>>("EcalBarrelImagingLayers"));
        app->Add(new JFactoryGeneratorT<JFactoryT<edm4eic::MCRecoClusterParticleAssociation>>("EcalBarrelImagingClusterAssociations"));

        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelTruthClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelMergedTruthClusters>());
    }
}
    
