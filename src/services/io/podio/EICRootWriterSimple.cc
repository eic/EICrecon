
#include "EICRootWriterSimple.h"
#include <JANA/JLogger.h>

#include <datamodel_glue.h>
#include "EICPodioBindings.h"

enum class InsertResult { Success, AlreadyInStore, Failure };

template <typename PodioT, typename PodioCollectionT>
struct InsertFacIntoEventStore {
    InsertResult operator() (JFactory* fac, podio::EventStore* store) {

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

EICRootWriterSimple::EICRootWriterSimple() {
    SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
}

void EICRootWriterSimple::Init() {

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
}

void EICRootWriterSimple::Process(const std::shared_ptr<const JEvent> &event) {

    // For now, we rely on the EventStore object having been created by the source and added to the event.
    // Copy it to a stack variable first so we can ensure it is the same EventStore object we use for
    // every event. This is because once the ROOTWriter is created below, we cannot change the EventStore
    // pointer it uses.
    auto store = event->GetSingle<podio::EventStore>();
    if( ! m_store ) m_store = const_cast<podio::EventStore*>(store);
    if( m_store != store ){
        LOG_ERROR(default_cerr_logger) << "podio::EventStore pointer has changed!" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "This podio writer requires that the same EventStore object be used" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "for every event. The one obtained from JANA for this event appears" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "to be different from the previous event (" << store << " != " << m_store << LOG_END;
        throw JException("podio::EventStore pointer has changed between JANA events");
    }

    // Create podio::ROOTWriter object if not already created
    // We must do this here because we don't have the EventStore in Init()
    if(m_writer == nullptr ){
        // TODO: Check for error
        m_writer = std::make_shared<podio::ROOTWriter>(m_output_file, m_store);

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
    }

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

            auto result = CallWithPODIOType<InsertFacIntoEventStore, InsertResult, JFactory*, podio::EventStore*>(fac->GetObjectName(), fac, m_store);

            if (result == std::nullopt) {
                // jout << "... match failure" << jendl;
            }
            else if (result == InsertResult::AlreadyInStore) {
                // jout << "... already in store!" << jendl;
            }
            else {
                m_writer->registerForWrite(fac->GetTag());
                jout << "EICRootWriterSimple::Process: Adding to store: " << fac->GetObjectName() << ":" << fac->GetTag() << jendl;
            }
        }
        catch(std::exception &e){
            LOG_ERROR(default_cerr_logger) << e.what() << " : " << fac->GetObjectName() << LOG_END;
        }
    }

    m_writer->writeEvent();
    // n.b. we don't call clearCollections() here so we can leave that to the event source which owns the EventStore.

}

void EICRootWriterSimple::Finish() {

    m_writer->finish();
}

