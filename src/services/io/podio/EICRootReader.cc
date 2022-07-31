
#include <iostream>

#include <TInterpreter.h>

#include "EICRootReader.h"
#include "datamodel_glue.h"

#include <edm4hep/EventHeader.h>
#include <edm4hep/SimTrackerHit.h>


//----------------------------------------
// Destructor
//----------------------------------------
EICRootReader::~EICRootReader(){
    if( m_collectionid_table ) delete m_collectionid_table;
    if( m_fileVersion        ) delete m_fileVersion;
    if( m_run_metadata       ) delete m_run_metadata;
    if( m_col_metadata       ) delete m_col_metadata;
}

//----------------------------------------
// OpenFile
//----------------------------------------
void EICRootReader::OpenFile(const std::string &filename) {

    m_file = std::make_shared<TFile>( filename.c_str() );
    if( !m_file ) throw std::runtime_error("unable to open file: " + filename);

    // Get trees
    m_events_tree       = static_cast<TTree*>(m_file->Get("events"      ));
    m_metadata_tree     = static_cast<TTree*>(m_file->Get("metadata"    ));
    m_run_metadata_tree = static_cast<TTree*>(m_file->Get("run_metadata"));
    m_evt_metadata_tree = static_cast<TTree*>(m_file->Get("evt_metadata"));
    m_col_metadata_tree = static_cast<TTree*>(m_file->Get("col_metadata"));

    if( !m_events_tree       ) throw std::runtime_error("unable to find tree: events"      );
    if( !m_metadata_tree     ) throw std::runtime_error("unable to find tree: metadata"    );
    if( !m_run_metadata_tree ) throw std::runtime_error("unable to find tree: run_metadata");
    if( !m_evt_metadata_tree ) throw std::runtime_error("unable to find tree: evt_metadata");
    if( !m_col_metadata_tree ) throw std::runtime_error("unable to find tree: col_metadata");

    std::cout << "Opened file: " << filename << std::endl;

    // Read in metadata
    m_fileVersion = new podio::version::Version{0,0,0};
    m_collectionid_table = new podio::CollectionIDTable();
    m_metadata_tree->SetBranchAddress("PodioVersion", &m_fileVersion);
    m_metadata_tree->SetBranchAddress("CollectionIDs", &m_collectionid_table);
    m_metadata_tree->GetEntry(0);
    char verstr[256];
    sprintf( verstr, "v%02d-%02d-%02d", m_fileVersion->major, m_fileVersion->minor, m_fileVersion->patch );
    std::cout << "Podio version:  " << verstr << std::endl;
    std::cout << "Read CollectionIDTable with " << m_collectionid_table->ids().size() << " entries" << std::endl;

    // Read in run metadata
    m_run_metadata = new std::map<int,podio::GenericParameters>;
    m_run_metadata_tree->SetBranchAddress("runMD", &m_run_metadata);
    m_run_metadata_tree->GetEntry(0);

    // Read in col metadata
    m_col_metadata = new std::map<int,podio::GenericParameters>;
    m_col_metadata_tree->SetBranchAddress("colMD", &m_col_metadata);
    m_col_metadata_tree->GetEntry(0);

    // Get list of events tree branches by name. For each branch,
    // make a EICEventStore::DataVectorT object to hold the data
    // when an event is read.
    for( auto obj : *(m_events_tree->GetListOfBranches()) ){
        auto br = static_cast<TBranch*>( obj );
        auto dv = MakeDataVector( br->GetName(), br->GetClassName() );
        if( dv ) {
            auto addr_ptr = dv->GetVectorAddressPtr();
            br->SetAddress(addr_ptr);  // root wants address of pointer to data object, not address of data object!

            // Some branches are of type vector<podio::ObjectID> and we should treat them as
            // metadata as opposed to the POD objects that will be exposed as JANA objects
            if( dv->className == "vector<podio::ObjectID>" ){
                m_objidvectors.push_back(dv);
            }else{
                m_datavectors.push_back(dv);
            }
        }
    }
}

//----------------------------------------
// SetBranchStatus
//
/// Set the branch status for the named branch in the events tree.
/// By default, all branches are read for every event. By setting
/// a branch status to "0" in the tree, it will not be read. This
/// is useful if the user knows they want to ignore certain collections
/// in the file so processing can proceed faster. This simply calls
/// The TTree::SetBranchStatus method for the events tree.
//----------------------------------------
void EICRootReader::SetBranchStatus( const char *bname, Bool_t status, UInt_t *found ){
    m_events_tree->SetBranchStatus(bname, status, found);
}

//----------------------------------------
// GetNumEvents
//----------------------------------------
size_t EICRootReader::GetNumEvents() const {
    if( m_events_tree ) return m_events_tree->GetEntries();
    return 0;
}

//----------------------------------------
// GetDataVectors
//----------------------------------------
std::vector<const EICEventStore::DataVector*> EICRootReader::GetDataVectors( ) const{
    // Copy these into pointers to const so user knows not to modify them
    std::vector<const EICEventStore::DataVector*> datavectors;
    for(auto dv : m_datavectors) datavectors.push_back( dv );
    return std::move(datavectors);
}

//----------------------------------------
// GetObjIDVectors
//----------------------------------------
std::vector<const EICEventStore::DataVector*> EICRootReader::GetObjIDVectors( ) const{
    // Copy these into pointers to const so user knows not to modify them
    std::vector<const EICEventStore::DataVector*> objidvectors;
    for(auto dv : m_objidvectors) objidvectors.push_back( dv );
    return std::move(objidvectors);
}

//----------------------------------------
// GetEvent
//----------------------------------------
EICEventStore* EICRootReader::GetEvent( size_t entry_number ){

    // Read in event. Branch addresses should already be set up to point to vectors in m_datavectors
    m_events_tree->GetEntry( entry_number );

    // Create a new EICEventStore to hold this event. Create DataVectors to hold
    // the data and swap all vectors from the DataVector objects already set up
    // but ROOT.
    auto es = new EICEventStore();
    for( auto dv : m_datavectors ){
        auto es_dv = MakeDataVector(dv->name, dv->className );
        es_dv->Swap( dv );
        es->m_datavectors.push_back( es_dv );
    }

    // Do the same for the collectionID vectors
    for( auto dv : m_objidvectors ){
        auto es_dv = MakeDataVector(dv->name, dv->className );
        es_dv->Swap( dv );
        es->m_objidvectors.push_back( es_dv );
    }

    // TODO: There appears to be some additional metadata in the root file this code was developed
    // TODO: with. This is in the form of branches like "MCParticles#0" and "MCParticles#1". It
    // TODO: is not clear how to interpret this data and present it to the user in a meaningful way.

    // Return the new EICEventStore, passing ownership to the caller
    return es;
}
