// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "RawCalorimeterHit_factory_EcalBarrelSciGlassRawHits.h"
#include "CalorimeterHit_factory_EcalBarrelSciGlassRecHits.h"
#include "ProtoCluster_factory_EcalBarrelSciGlassProtoClusters.h"
#include "Cluster_factory_EcalBarrelSciGlassClusters.h"

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

#include "ProtoCluster_factory_EcalBarrelSciGlassTruthProtoClusters.h"
#include "Cluster_factory_EcalBarrelSciGlassTruthClusters.h"
#include "Cluster_factory_EcalBarrelSciGlassMergedTruthClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelSciGlassRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelSciGlassRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelSciGlassProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassClusters>());

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelScFiRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelScFiRecHits>());
        // app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelScFiMergedHits>());
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

        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelSciGlassTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassTruthClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassMergedTruthClusters>());
    }
}
