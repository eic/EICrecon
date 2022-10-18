
// Copyright 2022, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
#include <iostream>
#include "edm4nwb/CalorimeterHitCollection.h"
#include "edm4nwb/ClusterCollection.h"
#include <services/io/podio/MTEventStore.h>
#include <services/io/podio/MTRootWriter.h>
#if 0
#include <services/io/podio/MTRootWriter.h>
#include <services/io/podio/MTRootReader.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ClusterCollection.h>

void write_read_test() {
    std::cout << "Hello world!" << std::endl;
    auto logger = spdlog::default_logger();
    eic::MTRootWriter writer("test_out.root", logger);
    writer.registerForWrite("MyFunHits");
    writer.registerForWrite("MyFunClusters");

    eic::MTEventStore es(logger);
    auto hits = new edm4eic::CalorimeterHitCollection;
    auto clusters = new edm4eic::ClusterCollection;
    es.put("MyFunHits", hits);
    es.put("MyFunClusters", clusters);

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

    hits->push_back(hit1);
    hits->push_back(hit2);
    hits->push_back(hit3);

    clusters->push_back(cluster1);
    clusters->push_back(cluster2);

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

    auto hits2 = new edm4eic::CalorimeterHitCollection;
    auto clusters2 = new edm4eic::ClusterCollection;
    es.put("MyFunHits", hits2);
    es.put("MyFunClusters", clusters2);

    hits2->push_back(hit4);
    hits2->push_back(hit5);
    clusters2->push_back(cluster3);

    writer.writeEvent(&es);
    es.clear();

    writer.finish();
    eic::MTRootReader reader;
    reader.setLogger(logger);
    reader.openFile("test_out.root");

    eic::MTEventStore es_in(logger);
    auto nevents = reader.getEntries();
    for (int i=0; i<nevents; ++i) {
        reader.readEvent(&es_in, i);
        auto* hits_in = es_in.get<edm4eic::CalorimeterHitCollection>("MyFunHits");
        auto* clusters_in = es_in.get<edm4eic::ClusterCollection>("MyFunClusters");
        std::cout << "Event " << i << std::endl;
        std::cout << "Hits: " << hits_in->size() << std::endl;
        std::cout << "Clusters: " << clusters_in->size() << std::endl;
        std::cout << "Associations: " << clusters_in->size() << std::endl;
        for (auto cluster : *clusters_in) {
            for (auto i=cluster.hits_begin(), end=cluster.hits_end(); i != end; ++i) {
                std::cout << i->getCellID() << " ";
            }
            std::cout << std::endl;
        }
        es_in.clear();
    }
}

void read_write_test() {

    auto logger = spdlog::default_logger();
    eic::MTRootReader reader;
    reader.setLogger(logger);
    reader.openFile("test_out.root"); // comes from prev test
    // Read everything = MyFunHits, MyFunClusters

    eic::MTRootWriter writer("test2_out.root", logger);
    writer.registerForWrite("MyFunClusters");
    writer.registerForWrite("MyExhilaratingClusters"); // Collection needs to be added to store first

    auto nevents = reader.getEntries();
    for (int i=0; i<nevents; ++i) {
        std::cout << "Processing event " << i << std::endl;

        auto store = new eic::MTEventStore(logger);
        reader.readEvent(store, i);
        // This should populate hits_in and clusters_in
        auto* hits_in = store->get<edm4eic::CalorimeterHitCollection>("MyFunHits");
        auto* clusters_in = store->get<edm4eic::ClusterCollection>("MyFunClusters");

        auto* clusters_out_filtered = new edm4eic::ClusterCollection;
        store->put("MyExhilaratingClusters", clusters_out_filtered);
        for (auto cluster : *clusters_in) {
            if (cluster.getEnergy() < 50) {
                std::cout << "Adding cluster with energy " << cluster.getEnergy() << std::endl;
                clusters_out_filtered->push_back(cluster.clone());
                // will this do a deep or shallow copy in memory?
                // what about in file?
                // will references be included?
            }
        }
        writer.writeEvent(store);
        store->clear();
    }
    writer.finish();
    reader.closeFile();
}

#endif

void collections_don_t_leak() {
    std::cout << "Running collections_don_t_leak" << std::endl;
    edm4nwb::CalorimeterHitCollection hits;
    edm4nwb::MutableCalorimeterHit hit;
    hit.setEnergy(22.2);
    hits.push_back(hit);
}

void pointer_to_podio_type_stays_valid_after_add() {
    std::cout << "Running pointer_to_podio_type_stays_valid_after_add" << std::endl;
    edm4nwb::CalorimeterHitCollection hits_collection;

    auto hit = new edm4nwb::MutableCalorimeterHit;
    hit->setEnergy(22.2);
    hits_collection.push_back(*hit);
    std::cout << hit->getEnergy() << std::endl;  // Apparently this is still a valid reference
    delete hit;
    // Still have to delete this guy, else memory leak.
    // It looks like this is a memory leak of the outer layer, not of the POD layer, luckily
    // I guess this means we don't want NOT_OBJECT_OWNER?
}

void collection_delete_check() {
    std::cout << "Running collection_delete_check" << std::endl;
    auto hits = new edm4nwb::CalorimeterHitCollection;
    edm4nwb::MutableCalorimeterHit hit;
    hits->push_back(hit);
    delete hits;

    /* This fails!!!
    Invalid read of size 4
    ==22320==    at 0x10D26C: podio::ObjBase::release() (ObjBase.h:27)
    ==22320==    by 0x1295A8: edm4nwb::MutableCalorimeterHit::~MutableCalorimeterHit() (MutableCalorimeterHit.cc:55)
    ==22320==    by 0x10C7F1: collection_leak_check() (podio_test.cpp:183)
    ==22320==    by 0x10C96D: main (podio_test.cpp:201)
    ==22320==  Address 0x4e74208 is 8 bytes inside a block of size 96 free'd
    ==22320==    at 0x484399B: operator delete(void*, unsigned long) (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
    ==22320==    by 0x114AB8: edm4nwb::CalorimeterHitObj::~CalorimeterHitObj() (CalorimeterHitObj.h:28)
    ==22320==    by 0x111263: edm4nwb::CalorimeterHitCollectionData::clear(bool) (CalorimeterHitCollectionData.cc:25)
    ==22320==    by 0x10D496: edm4nwb::CalorimeterHitCollection::~CalorimeterHitCollection() (CalorimeterHitCollection.cc:18)
    ==22320==    by 0x10D4DD: edm4nwb::CalorimeterHitCollection::~CalorimeterHitCollection() (CalorimeterHitCollection.cc:19)
    ==22320==    by 0x10C7E5: collection_leak_check() (podio_test.cpp:182)
    ==22320==    by 0x10C96D: main (podio_test.cpp:201)
    ==22320==  Block was alloc'd at
    ==22320==    at 0x4840F2F: operator new(unsigned long) (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
    ==22320==    by 0x1292AA: edm4nwb::MutableCalorimeterHit::MutableCalorimeterHit() (MutableCalorimeterHit.cc:16)
    ==22320==    by 0x10C766: collection_leak_check() (podio_test.cpp:178)
    ==22320==    by 0x10C96D: main (podio_test.cpp:201)
    */
}

void collection_delete_check_workaround() {
    std::cout << "Running collection_delete_check_workaround" << std::endl;
    auto hits = new edm4nwb::CalorimeterHitCollection;
    {
        edm4nwb::MutableCalorimeterHit hit;
        hits->push_back(hit);
    }
    delete hits;
    // Basically we need to make sure that the "free" PODIO objects are destroyed before the collection versions
    // Or we call hit.unlink()
}

void event_store_delete_check() {
    std::cout << "Running event_store_delete_check" << std::endl;
    auto hits = new edm4nwb::CalorimeterHitCollection;
    edm4nwb::MutableCalorimeterHit hit;
    hit.setEnergy(22.2);
    hits->push_back(hit);
    hit.unlink();
    auto logger = spdlog::default_logger();
    eic::MTEventStore store(logger);
    store.put("MyFunHits", hits);
}

void jana_factory_integration_check() {

}

void write_check() {
    std::cout << "Running write_check" << std::endl;
    auto hits = new edm4nwb::CalorimeterHitCollection;
    edm4nwb::MutableCalorimeterHit hit;
    hit.setEnergy(22.2);
    hits->push_back(hit);
    hit.unlink();
    auto logger = spdlog::default_logger();
    eic::MTEventStore store(logger);
    store.put("MyFunHits", hits);
    eic::MTRootWriter writer("write_check.root", logger);
    writer.registerForWrite("MyFunHits");
    writer.writeEvent(&store);
    writer.finish();
}

int main() {
    // collections_don_t_leak();
    // pointer_to_podio_type_stays_valid_after_add();
    // collection_delete_check();
    // collection_delete_check_workaround();
    // event_store_delete_check();
    write_check();

    // write_read_test();
    // read_write_test();
}