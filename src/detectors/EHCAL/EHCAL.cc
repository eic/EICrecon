// Copyright 2023, Friederike Bock
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

#include "RawCalorimeterHit_factory_HcalEndcapNRawHits.h"
#include "CalorimeterHit_factory_HcalEndcapNRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapNMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapNTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapNIslandProtoClusters.h"
#include "Cluster_factory_HcalEndcapNClusters.h"
#include "Cluster_factory_HcalEndcapNTruthClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapNRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapNRecHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapNMergedHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapNTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapNIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapNClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapNTruthClusters>());

    }
}
