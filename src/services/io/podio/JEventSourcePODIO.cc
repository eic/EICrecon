// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//
// This is a JANA event source that uses PODIO to read from a ROOT
// file created using the EDM4hep Data Model.
//
// This uses the EICRootReader and EICEventStore classes. It is thread safe.

#include "JEventSourcePODIO.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <filesystem>

#include <JANA/JFactoryGenerator.h>

// podio specific includes
#include "podio/EventStore.h"
#include "podio/IReader.h"
#include "podio/UserDataCollection.h"
#include "podio/podioVersion.h"

// This file is generated automatically by make_datamodel_glue.py
#include "datamodel_glue.h"


//------------------------------------------------------------------------------
// CopyPodioToJEventT
//------------------------------------------------------------------------------
template <typename T, typename Tobj, typename Tdata>
void CopyToJEventT(EICEventStore::DataVectorT<Tdata> *dvt, const podio::CollectionIDTable *collectionIDs, std::shared_ptr<JEvent> &event, std::vector<podio::ObjBase*> &podio_objs){

    // TODO: Recreate ObjectID from data in file (probably will need event_metadata)
    podio::ObjectID id{0,0};

    // Create high-level podio objects (e.g. edm4hep::EventHeader)
    // n.b. In podio, the data actually resides in a member of the
    // "Obj" clas (e.g. edm4hep::EventHeaderObj). Thus, we must instantiate
    // one of those and then instantiate the high-level object to wrap
    // it. The high-level objects will be passed to JANA to own and manage
    // while the "Obj" objects will be added to the podio_objs vector
    // so that the caller can manage deleting those. (The podio_objs
    // vector is likely a member of an EICEventStore object and all
    // of the objects in podio_objs will be deleted at the end of the
    // event.)
    std::vector<const T*> tptrs;
    for( auto &data : dvt->vec ){
        auto obj = new Tobj(id, data);
        podio_objs.push_back(obj); // pass ownship of "Obj" objects to caller
        tptrs.push_back( new T(obj) );
        id.index++; // TODO: See above to-do on ObjectIDs
    }
    event->Insert( tptrs, dvt->name ); // pass ownership of high-level objects to JANA
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
JEventSourcePODIO::JEventSourcePODIO(std::string resource_name, JApplication* app) : JEventSource(resource_name, app) {
    SetTypeName(NAME_OF_THIS); // Provide JANA with class name

    // Tell JANA that we want it to call the FinishEvent() method.
    EnableFinishEvent();
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
JEventSourcePODIO::~JEventSourcePODIO() {
    LOG << "Closing Event Source for " << GetResourceName() << LOG_END;
}

//------------------------------------------------------------------------------
// Open
//------------------------------------------------------------------------------
void JEventSourcePODIO::Open() {

    // Open is called exactly once when processing begins.
	
    // Allow user to specify to recycle events forever
    GetApplication()->SetDefaultParameter("PODIO:RUN_FOREVER", run_forever, "set to true to recycle through events continuously");

    bool print_type_table = false;
    GetApplication()->SetDefaultParameter("PODIO:PRINT_TYPE_TABLE", print_type_table, "Print list of collection names and their types");

    try {
        // Have PODIO reader open file and get the number of events from it.
        reader.OpenFile( GetResourceName() );

        auto version = reader.GetPodioVersion();
        bool version_mismatch = version.major > podio::version::build_version.major;
        version_mismatch |= (version.major == podio::version::build_version.major) && (version.minor>podio::version::build_version.minor);
        if( version_mismatch ){
            LOG_ERROR(default_cerr_logger) << "Mismatch in PODIO versions! " << version << " > " << podio::version::build_version << LOG_END;
            GetApplication()->Quit();
            return;
        }
        LOG << "PODIO version: file=" << version << " (executable=" << podio::version::build_version << ")" << LOG_END;

        Nevents_in_file = reader.GetNumEvents();
        LOG << "Opened PODIO file \"" << GetResourceName() << "\" with " << Nevents_in_file << " events" << LOG_END;

        if( print_type_table ) PrintCollectionTypeTable();

    }catch (std::exception &e ){
        _DBG__;
        LOG_ERROR(default_cerr_logger) << "Problem opening file \"" << GetResourceName() << "\"" << LOG_END;
        LOG_ERROR(default_cerr_logger) << e.what() << LOG_END;
        GetApplication()->Quit();
        return;
    }
}

//------------------------------------------------------------------------------
// GetEvent
//------------------------------------------------------------------------------
void JEventSourcePODIO::GetEvent(std::shared_ptr<JEvent> event) {

    /// Calls to GetEvent are synchronized with each other, which means they can
    /// read and write state on the JEventSource without causing race conditions.
    
    // Check if we have exhausted events from file
    if( Nevents_read >= Nevents_in_file ) {
        if( run_forever ){
            Nevents_read = 0;
        }else{
            reader.CloseFile();
            throw RETURN_STATUS::kNO_MORE_EVENTS;
        }
    }

    // Read the specified event into a new EICEventStore.
    auto es = reader.GetEvent( Nevents_read++ );  // take ownership ....
    event->Insert( es );                          // ... and hand over to JANA

    // TODO: The following could be deferred and done in parallel

    // At this point, the EICEventStore object has a bunch of std:vector objects
    // with the POD edm4hep::*Data types (e.g. edm4hep::EventHeaderData).
    // What we need to do now is copy them into high level data
    // types (e.g. edm4hep::EventHeader) and insert them into the JEvent.
    for( auto dv : es->m_datavectors ){
        CopyToJEvent( dv, reader.GetCollectionIDTable() , event, es->m_podio_objs);
    }

    // Get the EventHeader object which contains the run number and event number
    auto headers = event->Get<edm4hep::EventHeader>("EventHeader");
    for( auto h : headers ){ // should only be one, but this makes it easy
        event->SetEventNumber( h->getEventNumber() );
        event->SetRunNumber( h->getRunNumber() );
    }

}

//------------------------------------------------------------------------------
// FinishEvent
//------------------------------------------------------------------------------
void JEventSourcePODIO::FinishEvent(JEvent &event){

    // Get the EICEventStore object from the JEvent and delete underlying
    // "Obj" objects from it.
    auto es = event.GetSingle<EICEventStore>();
    if( es ) const_cast<EICEventStore*>(es)->FreeEventObjects(); // Delete all underlying podio "Obj" objects
}

//------------------------------------------------------------------------------
// GetDescription
//------------------------------------------------------------------------------
std::string JEventSourcePODIO::GetDescription() {

    /// GetDescription() helps JANA explain to the user what is going on
    return "PODIO root file (example)";
}

//------------------------------------------------------------------------------
// CheckOpenable
//------------------------------------------------------------------------------
template <>
double JEventSourceGeneratorT<JEventSourcePODIO>::CheckOpenable(std::string resource_name) {

    // If source is a root file, given minimal probability of success so we're chosen
    // only if no other ROOT file sources exist.
    return (resource_name.find(".root") != std::string::npos ) ? 0.01 : 0.0;
}

//------------------------------------------------------------------------------
// PrintCollectionTypeTable
//
/// Print the list of collection names from the currently open file along
/// with their types. This will be called automatically when the file is
/// open if the PODIO:PRINT_TYPE_TABLE variable is set to a non-zero value
//------------------------------------------------------------------------------
void JEventSourcePODIO::PrintCollectionTypeTable(void){

    // First, get maximum length of the collection name strings so
    // we can print nicely aligned columns.
    size_t max_name_len =0;
    size_t max_type_len = 0;
    for( auto dv : reader.GetDataVectors() ){
        max_name_len = std::max( max_name_len, dv->name.length() );
        max_type_len = std::max( max_type_len, dv->name.length() );
    }

    // Print table
    std::cout << std::endl;
    std::cout << "Available Collections" << std::endl;
    std::cout << std::endl;
    std::cout << "Collection Name" << std::string( max_name_len + 2 - std::string("Collection Name").length(), ' ' ) << "Data Type" << std::endl;
    std::cout << std::string(max_name_len, '-') << "  " << std::string(max_name_len, '-') << std::endl;
    for( auto dv : reader.GetDataVectors() ){
        std::cout << dv->name + std::string( max_name_len + 2 - dv->name.length(), ' ' );
        std::cout << dv->className << std::endl;
    }
    std::cout << std::endl;

}