
// Copyright 2022, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
#include <iostream>
#include <services/io/podio/RootWriter.h>
#include <services/io/podio/EventStore.h>
#include <services/io/podio/RootReader.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ClusterCollection.h>

void write_read_test() {
    std::cout << "Hello world!" << std::endl;
    auto logger = spdlog::default_logger();
    eic::ROOTWriter writer("test_out.root", logger);
    writer.registerForWrite("MyFunHits");
    writer.registerForWrite("MyFunClusters");

    // Set up writer
    eic::EventStore es;
    auto& hits = es.create<edm4eic::CalorimeterHitCollection>("MyFunHits");
    auto& clusters = es.create<edm4eic::ClusterCollection>("MyFunClusters");

    // Save several events

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
    cluster2.setEnergy(27.7);
    cluster2.setNhits(1);
    cluster2.addToHits(hit2);

    hits.push_back(hit1);
    hits.push_back(hit2);
    hits.push_back(hit3);

    clusters.push_back(cluster1);
    clusters.push_back(cluster2);

    writer.writeEvent(&es);
    es.clear();

    edm4eic::MutableCalorimeterHit hit4;
    hit4.setCellID(42);
    hit4.setEnergy(42.2);
    
    edm4eic::MutableCalorimeterHit hit5;
    hit5.setCellID(7);
    hit5.setEnergy(7.6);
    
    edm4eic::MutableCluster cluster3;
    cluster3.setEnergy(7.6);
    cluster3.setNhits(1);
    cluster3.addToHits(hit5);

    // auto& hits2 = es.get<edm4eic::CalorimeterHitCollection>("MyFunHits");
    // auto& clusters2 = es.get<edm4eic::ClusterCollection>("MyFunClusters");

    hits.push_back(hit4);
    hits.push_back(hit5);
    clusters.push_back(cluster3);

    writer.writeEvent(&es);
    writer.finish();

    eic::EventStore es_in;
    eic::ROOTReader reader;
    reader.openFile("test_out.root");

    auto nevents = reader.getEntries();
    for (int i=0; i<nevents; ++i) {
        reader.readEvent(&es_in, i);
        auto& hits_in = es_in.get<edm4eic::CalorimeterHitCollection>("MyFunHits");
        auto& clusters_in = es_in.get<edm4eic::ClusterCollection>("MyFunClusters");
        std::cout << "Event " << i << std::endl;
        std::cout << "Hits: " << hits_in.size() << std::endl;
        std::cout << "Clusters: " << clusters_in.size() << std::endl;
        std::cout << "Associations: " << clusters_in.size() << std::endl;
        for (auto cluster : clusters_in) {
            for (auto i=cluster.hits_begin(), end=cluster.hits_end(); i != end; ++i) {
                std::cout << i->getCellID() << " ";
            }
            std::cout << std::endl;
        }
        es_in.clear();
    }
}

void read_write_test() {

    eic::EventStore input_store;
    eic::EventStore output_store;
/*
    eic::ROOTReader reader;
    reader.openFile("test_out.root"); // comes from prev test
    input_store.setReader(&reader);

    auto logger = spdlog::default_logger();
    eic::ROOTWriter writer("test2_out.root", &output_store, logger);
    output_store.create<edm4eic::ClusterCollection>("MyFunClusters");
    output_store.create<edm4eic::ClusterCollection>("MyExhilaratingClusters");

    writer.registerForWrite("MyFunClusters");
    writer.registerForWrite("MyExhilaratingClusters"); // Collection needs to be added to store first

    auto nevents = reader.getEntries();
    for (int i=0; i<nevents; ++i) {
        std::cout << "Processing event " << i << std::endl;
        reader.goToEvent(i);
        auto& hits_in = input_store.get<edm4eic::CalorimeterHitCollection>("MyFunHits");
        auto& clusters_in = input_store.get<edm4eic::ClusterCollection>("MyFunClusters");

        for (auto cluster : clusters_in) {
            auto& clusters_out = output_store.get<edm4eic::ClusterCollection>("MyFunClusters");
            auto& clusters_out_filtered = output_store.get<edm4eic::ClusterCollection>("MyExhilaratingClusters");
            clusters_out.push_back(cluster.clone());
            if (cluster.getEnergy() < 50) {
                std::cout << "Adding cluster with energy " << cluster.getEnergy() << std::endl;
                clusters_out_filtered.push_back(cluster.clone()); 
                // will this do a deep or shallow copy in memory?
                // what about in file?
                // will references be included?
            }
        }
        writer.writeEvent();
        reader.endOfEvent();

        input_store.clear();
        output_store.clearCollections();
    }
    writer.finish();
    reader.closeFile();
    */
}

int main() {
    write_read_test();
    read_write_test();
}