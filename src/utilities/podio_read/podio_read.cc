
//
// example 122MB file : https://eicaidata.s3.amazonaws.com/2022_07-18_necal_disk_gun_electron_0-15GeV_1000ev.edm4hep.root
// podio_copy 2022_07-18_necal_disk_gun_electron_0-15GeV_1000ev.edm4hep.root copy.root
//

#include <iostream>
#include <iomanip>
#include <ctime>
#include <set>

#include <podio/ROOTReader.h>
#include <podio/EventStore.h>
#include <podio/CollectionBase.h>

#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/CalorimeterHitCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/ClusterCollection.h>
#include <edm4hep/TrackerHitCollection.h>
#include <edm4hep/TrackerHitPlaneCollection.h>
#include <edm4hep/TPCHitCollection.h>
#include <edm4hep/TrackCollection.h>
#include <edm4hep/VertexCollection.h>
#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/MCRecoParticleAssociationCollection.h>
#include <edm4hep/MCRecoCaloAssociationCollection.h>
#include <edm4hep/MCRecoTrackerAssociationCollection.h>
#include <edm4hep/MCRecoTrackerHitPlaneAssociationCollection.h>
#include <edm4hep/MCRecoCaloParticleAssociationCollection.h>
#include <edm4hep/MCRecoClusterParticleAssociationCollection.h>
#include <edm4hep/MCRecoTrackParticleAssociationCollection.h>
#include <edm4hep/RecoParticleVertexAssociationCollection.h>

#include <TFile.h>
#include <TTree.h>

void createBranch(const std::string& collName, podio::CollectionBase* collBase, TTree *m_datatree);
void resetBranches(const std::map<std::string, podio::CollectionBase*>& collections, TTree *m_datatree);
podio::CollectionBase* CopyCollection(podio::EventStore& store_in, podio::EventStore& store_out, const std::string& name, const std::string &colltype);

// Used to hold collection info for output store
std::vector<std::tuple<int, std::string, bool>> m_collectionInfo;


//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main(int narg, char *argv[]){

    if( narg<3 ) {std::cerr << "Usage: podio_copy file_in file_out" << std::endl; return -1;}
    auto file_in  = argv[1];
    auto file_out = argv[2];

    // Open input file for reading
    podio::ROOTReader reader;
    podio::EventStore store_in;  // EventStore used to read events in
    reader.openFile(file_in);
    auto Nevents_in_file = reader.getEntries();
    store_in.setReader(&reader);

    // Open output file for writing
//    podio::EventStore store_out; // EventStore used to write events out
//    auto m_file = new TFile(file_out, "RECREATE", "data file");
//    auto m_datatree     = new TTree("events",       "Events tree");
//    auto m_metadatatree = new TTree("metadata",     "Metadata tree");
//    auto m_runMDtree    = new TTree("run_metadata", "Run metadata tree");
//    auto m_evtMDtree    = new TTree("evt_metadata", "Event metadata tree");
//    auto m_colMDtree    = new TTree("col_metadata", "Collection metadata tree");
//    m_evtMDtree->Branch("evtMD", "GenericParameters", store_out.eventMetaDataPtr() ) ;

    // Loop over all events in input file
    auto start_time = clock();
    for( size_t ievent=0; ievent<Nevents_in_file; ievent++) {
        auto event_start_time = clock();

        // Read in all collections for event
        store_in.clear();
        reader.goToEvent(ievent);
        std::map<std::string, std::string> collection_names_in;
        for (auto id : store_in.getCollectionIDTable()->ids()) {
            podio::CollectionBase *coll = nullptr;
            if (store_in.get(id, coll)) {
                collection_names_in[store_in.getCollectionIDTable()->name(id)] = coll->getValueTypeName();
            }
        }

//        // Copy all collections to store_out.
//        store_out.clear();
//        for (auto &[name, colltype] : collection_names_in) {
//            auto collection_out = CopyCollection(store_in, store_out, name, colltype);
//
//            // Create branch in events tree for collection if needed.
//            static std::set<std::string> branches_created;
//            if( ! branches_created.count(name) ){
//                createBranch(name, collection_out, m_datatree);
//                branches_created.insert( name );
//
//                // Record collection info to write to tree at end
//                const auto collID = store_out.getCollectionIDTable()->collectionID(name);
//                const auto collType = collection_out->getValueTypeName() + "Collection";
//                const auto collInfo = std::tuple<int, std::string, bool>(collID, collType, collection_out->isSubsetCollection());
//                m_collectionInfo.push_back(collInfo);
//            }
//        }
//
//        // Reset branch addresses for all collections in store_out and write event to file
//        std::map<std::string, podio::CollectionBase*> collection_map_out;
//        for (auto id : store_out.getCollectionIDTable()->ids()) {
//            podio::CollectionBase *coll = nullptr;
//            if (store_out.get(id, coll)) {
//                collection_map_out[store_out.getCollectionIDTable()->name(id)] = coll;
//            }
//        }
//        resetBranches( collection_map_out, m_datatree );
//        m_datatree->Fill();
//        m_evtMDtree->Fill();

        // Print status
        auto rate = CLOCKS_PER_SEC/(double)(clock()-event_start_time);
        auto avg_rate = CLOCKS_PER_SEC/(double)(clock()-start_time)*(double)(ievent+1);
        std::cout << " " << ievent+1 << " events copied  ( " << std::setprecision(2) << rate << " Hz  avg. " << avg_rate << " Hz)      \r";  std::cout.flush();
    }

//    // Finalize root file
//    m_file->cd();
//    podio::version::Version podioVersion = podio::version::build_version;
//    m_metadatatree->Branch("PodioVersion", &podioVersion);
//    m_metadatatree->Branch("CollectionTypeInfo", &m_collectionInfo);
//    m_metadatatree->Branch("CollectionIDs", store_out.getCollectionIDTable());
//    m_metadatatree->Fill();
//    m_colMDtree->Branch("colMD", "std::map<int,podio::GenericParameters>", store_out.getColMetaDataMap() ) ;
//    m_colMDtree->Fill();
//    m_runMDtree->Branch("runMD", "std::map<int,podio::GenericParameters>", store_out.getRunMetaDataMap() ) ;
//    m_runMDtree->Fill();
//    m_datatree->Write();
//    m_file->Write();
//    m_file->Close();


    return 0;
}


//------------------------------------------------------------------------------
// createBranch
//------------------------------------------------------------------------------
void createBranch(const std::string& collName, podio::CollectionBase* collBase, TTree *m_datatree) {

    auto buffers = collBase->getBuffers();
    auto* data = buffers.data;
    auto* references = buffers.references;
    auto* vecmembers = buffers.vectorMembers;

    const std::string className     = collBase->getValueTypeName();
    const std::string collClassName = "vector<" + className + "Data>";

    auto branch = m_datatree->Branch(collName.c_str(), collClassName.c_str(), data);
    // Create branches for collections holding relations
    if (auto* refColls = references) {
        int j = 0;
        for (auto& c : (*refColls)) {
            const auto brName = collName + "#" + std::to_string(j);
            m_datatree->Branch(brName.c_str(), c.get());
            ++j;
        }
    }
    // vector members
    if (auto* vminfo = vecmembers) {
        int j = 0;
        for (auto& [dataType, add] : (*vminfo)) {
            const std::string typeName = "vector<" + dataType + ">";
            const auto brName          = collName + "_" + std::to_string(j);
            m_datatree->Branch(brName.c_str(), typeName.c_str(), add);
            ++j;
        }
    }

    // Backfill for events we've missed.
    // This branch may not be created until some events have already been processed. In order to align
    // future events, we need to insert empty events for this branch. To do this, we need a data pointer
    // that points to an empty vector (data currently points to a vector, but one that is not empty).
    // Ideally we would make a std:vector of the correct type here, but that is a little difficult and
    // adds extra overhead. Instead, we make a temporary std::vector<uint64_t> with the assumption that
    // root will see it has zero elements and do the correct thing.
    std::vector<uint64_t> tmpv;
    branch->SetAddress((void*)&tmpv);
    auto Nentries = m_datatree->GetEntries();
    for( size_t i=0; i< Nentries; i++ ) branch->Fill();
}

//------------------------------------------------------------------------------
// resetBranches
//------------------------------------------------------------------------------
void resetBranches(const std::map<std::string, podio::CollectionBase*>& collections, TTree *m_datatree) {
    for (const auto& [collName, collBuffers] : collections) {

        // This copies all of the POD structures into the internally managed std::vector
        // objects that will be returned by the getBuffers() call below.
        collBuffers->prepareForWrite();

        auto buffers = collBuffers->getBuffers();
        auto* data = buffers.data;
        auto* references = buffers.references;
        auto* vecmembers = buffers.vectorMembers;

        // Reconnect branches and collections
        m_datatree->SetBranchAddress(collName.c_str(), data);
        auto* colls = references;
        if (colls != nullptr) {
            for (size_t j = 0; j < colls->size(); ++j) {
                const auto brName = collName + "#" + std::to_string(j);
                auto* l_branch = m_datatree->GetBranch(brName.c_str());
                l_branch->SetAddress(&(*colls)[j]);
            }
        }
        auto* colls_v = vecmembers;
        if (colls_v != nullptr) {
            int j = 0;
            for (auto& [dataType, add] : (*colls_v)) {
                const auto brName = collName + "#" + std::to_string(j);
                m_datatree->SetBranchAddress(brName.c_str(), add);
                ++j;
            }
        }
    }
}

//------------------------------------------------------------------------------
// CopyCollectionT
//
// Copy the named collection from the input store to the output store
// via an intermediate set of objects not associated with any store.
//------------------------------------------------------------------------------
template<typename T, typename C>
podio::CollectionBase* CopyCollectionT(podio::EventStore& store_in, podio::EventStore& store_out, const std::string& name) {

    // Get pointer to collection in store_in.
    const C* collection_in=nullptr;
    store_in.get<C>(name, collection_in);

    // Get pointer to collection in store_out. Create if neccessary.
    // (this seems like I'm doing something wrong.)
    C* collection_out=nullptr;
    const C* tmpPtr = nullptr;
    if( store_out.get<C>(name, tmpPtr) ) {
        collection_out = const_cast<C*>(tmpPtr);
    }else{
        auto &c = store_out.create<C>( name );
        collection_out = &c;
    }

    // Loop over objects in input collection and create independent copies
    std::vector<T*> objects;
    for( const auto &t : *collection_in ) {
        if (t.isAvailable()) {
            objects.push_back(new T(t));
        }
    }

    // At this point, "objects" contains copies of the objects not associated
    // with any EventStore (I think).

    // Loop over independent copies and insert them into the output collection
    for( auto obj : objects ){
        collection_out->push_back( obj->clone() );
    }

    return collection_out;
}

//------------------------------------------------------------------------------
// CopyCollection
//------------------------------------------------------------------------------
podio::CollectionBase* CopyCollection(podio::EventStore& store_in, podio::EventStore& store_out, const std::string& name, const std::string &colltype){
    if( colltype=="edm4hep::EventHeader" ) return CopyCollectionT<edm4hep::EventHeader, edm4hep::EventHeaderCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::MCParticle" ) return CopyCollectionT<edm4hep::MCParticle, edm4hep::MCParticleCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::SimTrackerHit" ) return CopyCollectionT<edm4hep::SimTrackerHit, edm4hep::SimTrackerHitCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::CaloHitContribution" ) return CopyCollectionT<edm4hep::CaloHitContribution, edm4hep::CaloHitContributionCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::SimCalorimeterHit" ) return CopyCollectionT<edm4hep::SimCalorimeterHit, edm4hep::SimCalorimeterHitCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::RawCalorimeterHit" ) return CopyCollectionT<edm4hep::RawCalorimeterHit, edm4hep::RawCalorimeterHitCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::CalorimeterHit" ) return CopyCollectionT<edm4hep::CalorimeterHit, edm4hep::CalorimeterHitCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::ParticleID" ) return CopyCollectionT<edm4hep::ParticleID, edm4hep::ParticleIDCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::Cluster" ) return CopyCollectionT<edm4hep::Cluster, edm4hep::ClusterCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::TrackerHit" ) return CopyCollectionT<edm4hep::TrackerHit, edm4hep::TrackerHitCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::TrackerHitPlane" ) return CopyCollectionT<edm4hep::TrackerHitPlane, edm4hep::TrackerHitPlaneCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::TPCHit" ) return CopyCollectionT<edm4hep::TPCHit, edm4hep::TPCHitCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::Track" ) return CopyCollectionT<edm4hep::Track, edm4hep::TrackCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::Vertex" ) return CopyCollectionT<edm4hep::Vertex, edm4hep::VertexCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::ReconstructedParticle" ) return CopyCollectionT<edm4hep::ReconstructedParticle, edm4hep::ReconstructedParticleCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::MCRecoParticleAssociation" ) return CopyCollectionT<edm4hep::MCRecoParticleAssociation, edm4hep::MCRecoParticleAssociationCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::MCRecoCaloAssociation" ) return CopyCollectionT<edm4hep::MCRecoCaloAssociation, edm4hep::MCRecoCaloAssociationCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::MCRecoTrackerAssociation" ) return CopyCollectionT<edm4hep::MCRecoTrackerAssociation, edm4hep::MCRecoTrackerAssociationCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::MCRecoTrackerHitPlaneAssociation" ) return CopyCollectionT<edm4hep::MCRecoTrackerHitPlaneAssociation, edm4hep::MCRecoTrackerHitPlaneAssociationCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::MCRecoCaloParticleAssociation" ) return CopyCollectionT<edm4hep::MCRecoCaloParticleAssociation, edm4hep::MCRecoCaloParticleAssociationCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::MCRecoClusterParticleAssociation" ) return CopyCollectionT<edm4hep::MCRecoClusterParticleAssociation, edm4hep::MCRecoClusterParticleAssociationCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::MCRecoTrackParticleAssociation" ) return CopyCollectionT<edm4hep::MCRecoTrackParticleAssociation, edm4hep::MCRecoTrackParticleAssociationCollection>( store_in, store_out, name);
    if( colltype=="edm4hep::RecoParticleVertexAssociation" ) return CopyCollectionT<edm4hep::RecoParticleVertexAssociation, edm4hep::RecoParticleVertexAssociationCollection>( store_in, store_out, name);
    return nullptr;
}
