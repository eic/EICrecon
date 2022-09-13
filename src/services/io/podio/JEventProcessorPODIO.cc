
#include "JEventProcessorPODIO.h"
#include <JANA/JLogger.h>

#include <datamodel_glue.h>

enum class InsertResult { Success, AlreadyInStore, Failure };

template <typename PodioT, typename PodioCollectionT>
struct InsertFacIntoEventStore {
    InsertResult operator() (JFactory* fac, eic::EventStore* store) {

        std::string collection_name = fac->GetTag();

        const PodioCollectionT* collection = nullptr;
        if (store->get(collection_name, collection)) {
            return InsertResult::AlreadyInStore;
        }

        auto& mutable_collection = store->create<PodioCollectionT>(collection_name);
        auto tobjs = fac->GetAs<PodioT>();
        for (auto t : tobjs) {
            mutable_collection->push_back( t->clone() );
        }
        return InsertResult::Success;
    }
};

JEventProcessorPODIO::JEventProcessorPODIO() {
    SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
}

void JEventProcessorPODIO::Init() {

    japp->SetDefaultParameter("podio:output_file", m_output_file, "Name of EDM4hep/podio output file to write to. Setting this will cause the output file to be created and written to.");

    // Allow user to set PODIO:OUTPUT_FILE to "1" to specify using the default name.
    if( m_output_file == "1" ){
        auto param = japp->GetJParameterManager()->FindParameter("podio:output_file" );
        if(param) {
            param->SetValue( param->GetDefault() );
            m_output_file = param->GetDefault();
        }
    }

    // Get the output directory path for creating a second copy of the output file at the end of processing.
    // (this is duplicating similar functionality in Juggler/Gaudi so assume it is useful).
    japp->SetDefaultParameter("podio:output_file_copy_dir", m_output_file_copy_dir, "Directory name to make an additional copy of the output file to. Copy will be done at end of processing. Default is empty string which means do not make a copy. No check is made on path existing.");

    // Get the list of output collections to include/exclude
    std::vector<std::string> output_include_collections;  // need to get as vector, then convert to set
    std::vector<std::string> output_exclude_collections;  // need to get as vector, then convert to set
    japp->SetDefaultParameter("podio:output_include_collections", output_include_collections, "Comma separated list of collection names to write out. If not set, all collections will be written (including ones from input file). Don't set this and use PODIO:OUTPUT_EXCLUDE_COLLECTIONS to write everything except a selection.");
    japp->SetDefaultParameter("podio:output_exclude_collections", output_exclude_collections, "Comma separated list of collection names to not write out.");
    m_output_include_collections = std::set<std::string>(output_include_collections.begin(), output_include_collections.end());
    m_output_exclude_collections = std::set<std::string>(output_exclude_collections.begin(), output_exclude_collections.end());

    m_store = new eic::EventStore();
    m_writer = std::make_shared<eic::ROOTWriter>(m_output_file, m_store);
}


void JEventProcessorPODIO::Process(const std::shared_ptr<const JEvent> &event) {

    /*
    // Get list of collection names
    auto collectionIDtable = m_store->getCollectionIDTable();
    auto &collNames = collectionIDtable->names();
    std::set<std::string> collNames_set(collNames.begin(), collNames.end());

    // Determine what to include from include list (or include all if empty)
    if( m_output_include_collections.empty() ){
        m_collections_to_write = collNames_set; // user didn't specify. Assume all collections should be included

    }else{
        for( const auto &n : m_output_include_collections ){
            if( collNames_set.count( n ) ) m_collections_to_write.insert(n);
        }
    }

    // Determine which to exclude from exclude list
    for( const auto &n : m_output_exclude_collections ) m_collections_to_write.erase( n );

    // Apply include/exclude lists.
    for( const auto &collName : m_collections_to_write ) m_writer->registerForWrite( collName );
    */

    jout << "==================================" << jendl;
    jout << "Event #" << event->GetEventNumber() << jendl;

    // Look for objects created by JANA, but not part of a collection in the EventStore and add them
    // Loop over all factories.
    for( auto fac : event->GetAllFactories() ){

        // Attempt to put data from all factories into the store.
        // We need to do this for _all_ factories unless we've constrained it by using includes/excludes.
        // This is because podio::RootWriter constrains us to write one event at a time, so there
        // is no way to add a new branch after the first event.
        // This is called even for ones whose data classes don't inherit from
        // an edm4hep class. Those cases just silently do nothing here and return
        // an empty string. Note that this relies on the JFactory::EnableAs mechanism
        // so that needs to have been called in the factory constructor.
        try {

            auto result = CallWithPODIOType<InsertFacIntoEventStore, InsertResult, JFactory*, eic::EventStore*>(fac->GetObjectName(), fac, m_store);

            if (result == std::nullopt) {
                jout << "EICRootWriterSimple::Process: Not a recognized PODIO type: " << fac->GetObjectName() << ":" << fac->GetTag() << jendl;
            }
            else if (result == InsertResult::AlreadyInStore) {
                jout << "EICRootWriterSimple::Process: Already in store: " << fac->GetObjectName() << ":" << fac->GetTag() << jendl;
            }
            else {
                m_writer->registerForWrite(fac->GetTag());
                jout << "EICRootWriterSimple::Process: ADDING TO STORE: " << fac->GetObjectName() << ":" << fac->GetTag() << "; store collection count = " << m_store->getCollectionIDTable()->names().size() << jendl;
            }
        }
        catch(std::exception &e){
            LOG_ERROR(default_cerr_logger) << e.what() << " : " << fac->GetObjectName() << LOG_END;
        }
    }

    m_writer->writeEvent();
    // n.b. we don't call clearCollections() here so we can leave that to the event source which owns the EventStore.

}

void JEventProcessorPODIO::Finish() {

    m_writer->finish();
}

