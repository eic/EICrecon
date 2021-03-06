// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//
// This is a JANA event source that uses PODIO to read from a ROOT
// file created using the EDM4hep Data Model.
//
// n.b. This is currently NOT thread safe. This is due to using the
// example ROOTReader that comes with PODIO which uses a.
// TTree underneath. Thus, reading a second event in will overwrite
// the current one.

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
// GetPODIODataT
//
/// This templated global routine is used to easily add new data types to the
/// JEvent which are read from the PODIO store. Ownership of the deep, POD
/// object is maintained by PODIO.
///
/// The "T" class is the object type, while the "C" class is the collection type
/// that holds it. (e.g. T=SimTrackerHit, C=SimTrackerHitCollection )
///
/// This gets called from the GetPODIOData routine defined in the datamodel_glue.h
/// file which is generated by the make_datamodel_glue.py script.
//------------------------------------------------------------------------------
template <class T, class C>
void GetPODIODataT( const char *collection_name, std::shared_ptr <JEvent> &event, podio::EventStore &store){
    std::vector<const T *> T_pointers;
    auto& Ts  = store.get<C>(collection_name);
    if (Ts.isValid()) {
        for( const auto &t : Ts ) {
            if(t.isAvailable()) { // only copy if the underlying data object is actually available
                // The "t" variable references the data member of type T of the stack iterator
                // created by the for loop. Thus, it is temporary. It is just a wrapper though
                // around the actual POD type in the store. Thus, we need to create a new object
                // of type T on the heap that wraps that same data. PODIO manages the reference
                // counting so when JANA deletes the T objects later, the underlying POD data
                // reference is properly handled.
                T_pointers.push_back( new T(t) );
            }
        }
    }

    // Insert the pointers into the JEvent.
    // n.b. it is important to insert here even if T_pointers it empty since it creates
    // the factory object. Downstream code asking for the objects will cause an exception
    // to be thrown if the factory doesn't exist at all so we need it to be there.
    event->Insert(T_pointers, collection_name);
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
        std::string filename = GetResourceName();

        // TODO: Replace the example podio::ROOTReader with something that is thread safe
        // TODO: and does better error checking.
        if( ! std::filesystem::exists(filename) ){
            LOG_ERROR(default_cerr_logger) << LOG_END
            LOG_ERROR(default_cerr_logger) << "File \"" << filename << "\" does not exist!" << LOG_END;
            LOG_ERROR(default_cerr_logger) << LOG_END
            GetApplication()->Quit();
            return;
        }
        reader.openFile(filename);

        if( ! reader.isValid() ){
            LOG_ERROR(default_cerr_logger) << "podio::ROOTReader is invalid after attempting to open file." << LOG_END;
            GetApplication()->Quit();
            return;
        }
        if (reader.currentFileVersion() != podio::version::build_version) {
            LOG_ERROR(default_cerr_logger) << "Mismatch in PODIO versions! " << reader.currentFileVersion() << " != "
                  << podio::version::build_version << LOG_END;
            GetApplication()->Quit();
            return;
        }
        LOG << "PODIO version: file=" << reader.currentFileVersion() << " (executable=" << podio::version::build_version << ")" << LOG_END;

        Nevents_in_file = reader.getEntries();
        LOG << "Opened PODIO file \"" << filename << "\" with " << Nevents_in_file << " events" << LOG_END;

        // Tell PODIO event store where to get its data from
        store.setReader(&reader);

        // Here we need to extract the names and types of all of the data collections defined in the file.
        reader.readEvent();  // calls to store.get() below fail unless we do this
        for( auto id : store.getCollectionIDTable()->ids() ){
            podio::CollectionBase *coll = nullptr;
            if( store.get( id, coll ) ){
                collection_names[store.getCollectionIDTable()->name( id )] = coll->getValueTypeName();
            }else{
                LOG_WARN(default_cout_logger) << "Found podio data collection named \"" << store.getCollectionIDTable()->name( id ) << "\" but unable to get date type." << LOG_END;
            }
        }
        LOG << "Found " << collection_names.size() << " collection types" << LOG_END;
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
void JEventSourcePODIO::GetEvent(std::shared_ptr <JEvent> event) {

    /// Calls to GetEvent are synchronized with each other, which means they can
    /// read and write state on the JEventSource without causing race conditions.
    
    // Check if we have exhausted events from file
    if( Nevents_read >= Nevents_in_file ) {
        if( run_forever ){
            Nevents_read = 0;
        }else{
            reader.closeFile();
            throw RETURN_STATUS::kNO_MORE_EVENTS;
        }
    }

    // Check/Set flag to indicate an event is being processed
    if( m_event_in_flight ) throw RETURN_STATUS::kBUSY;
    m_event_in_flight = true;

    // This tells PODIO to free up the memory/caches used for the
    // collections and MetaData left from the last event.
    store.clear();
	
    // Tell PODIO which event to read into the store on the next calls to
    // getEventMetaData() and get<>() etc... below
    reader.goToEvent( Nevents_read++ );

    // The following reads the data for each of the collections into memory and
    // then copies the pointers into the JEvent so they can easily be used
    // by JANA algorithms. Ownership of the objects still resides with the
    // PODIO store. The GetPODIOData routine is defined in the datamodel_glue.h
    // file which is automatically generated by the make_datamodel_glue.py
    // script baed on source files in the EDM4hep data modle. See also notes
    // near GetPODIOT at the top of this file.
    for( auto p : collection_names ) GetPODIOData( p.first, p.second, event, store);

    // Get the EventHeader object which contains the run number and event number
    auto headers = event->Get<edm4hep::EventHeader>("EventHeader");
    for( auto h : headers ){ // should only be one, but this makes it easy
        event->SetEventNumber( h->getEventNumber() );
        event->SetRunNumber( h->getRunNumber() );
        //event->SetEventNumber(Nevents_read);
    }

}

//------------------------------------------------------------------------------
// FinishEvent
//------------------------------------------------------------------------------
void JEventSourcePODIO::FinishEvent(JEvent&){
    // Event finished. Clear flag indicating it is safe to read another in.
    m_event_in_flight = false;
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
    for( auto p : collection_names ){
        max_name_len = std::max( max_name_len, p.first.length() );
        max_type_len = std::max( max_type_len, p.first.length() );
    }

    // Print table
    std::cout << std::endl;
    std::cout << "Available Collections" << std::endl;
    std::cout << std::endl;
    std::cout << "Collection Name" << std::string( max_name_len + 2 - std::string("Collection Name").length(), ' ' ) << "Data Type" << std::endl;
    std::cout << std::string(max_name_len, '-') << "  " << std::string(max_name_len, '-') << std::endl;
    for( auto p : collection_names ){
        std::cout << p.first + std::string( max_name_len + 2 - p.first.length(), ' ' );
        std::cout << p.second << std::endl;
    }
    std::cout << std::endl;

}