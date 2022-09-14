
// Copyright 2022, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
#include <iostream>
#include "services/io/podio/RootWriter.h"
#include "services/io/podio/EventStore.h"
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ClusterCollection.h>

int main() {
    std::cout << "Hello world!" << std::endl;
    eic::EventStore es;
    eic::ROOTWriter writer("test_out.root", &es);

    edm4eic::MutableCalorimeterHit hit1;
    hit1.setCellID(22);
    hit1.setEnergy(22.2);
    
    edm4eic::MutableCalorimeterHit hit2;
    hit2.setCellID(27);
    hit2.setEnergy(27.7);

    edm4eic::MutableCalorimeterHit hit3;
    hit3.setCellID(49);
    hit3.setEnergy(49.9);

    edm4eic::MutableCluster cluster1;
    cluster1.setEnergy(22.2+49.9);
    cluster1.setNhits(2);
    cluster1.addToHits(hit1);
    cluster1.addToHits(hit3);

    edm4eic::MutableCluster cluster2;
    cluster1.setEnergy(27.7);
    cluster1.setNhits(1);
    cluster1.addToHits(hit2);

    auto& hits = es.create<edm4eic::CalorimeterHitCollection>("MyFunHits");
    hits.push_back(hit1);
    hits.push_back(hit2);
    hits.push_back(hit3);

    auto& clusters = es.create<edm4eic::ClusterCollection>("MyFunClusters");
    clusters.push_back(cluster1);
    clusters.push_back(cluster2);

    writer.registerForWrite("MyFunHits");
    writer.registerForWrite("MyFunClusters");
    writer.writeEvent();

    es.clearCollections();

    edm4eic::MutableCalorimeterHit hit4;
    hit1.setCellID(42);
    hit1.setEnergy(42.2);
    
    edm4eic::MutableCalorimeterHit hit5;
    hit2.setCellID(7);
    hit2.setEnergy(7.6);
    
    edm4eic::MutableCluster cluster3;
    cluster3.setEnergy(7.6);
    cluster3.setNhits(1);
    cluster3.addToHits(hit5);

    auto& hits2 = es.get<edm4eic::CalorimeterHitCollection>("MyFunHits");
    auto& clusters2 = es.get<edm4eic::ClusterCollection>("MyFunClusters");
    hits2.push_back(hit4);
    hits2.push_back(hit5);
    clusters2.push_back(cluster3);

    writer.registerForWrite("MyFunHits");
    writer.registerForWrite("MyFunClusters");
    writer.writeEvent();
    writer.finish();
}


