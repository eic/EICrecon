
#include "JEventProcessorPODIO.h"
#include <services/log/Log_service.h>

#include <datamodel_glue.h>
#include <algorithm>


template <typename PodioT, typename PodioCollectionT>
struct InsertFacIntoStore {
    size_t operator() (JFactory* fac, eic::EventStore* store, bool create) {
        std::string collection_name = fac->GetTag();
        if (create) {
            store->create<PodioCollectionT>(collection_name);
        }
        auto& collection = store->get<PodioCollectionT>(collection_name);
        auto tobjs = fac->GetAs<PodioT>();
        for (auto t : tobjs) {
            collection.push_back( t->clone() );
        }
        return tobjs.size();
    }
};

JEventProcessorPODIO::JEventProcessorPODIO() {
    SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

    japp->SetDefaultParameter(
            "podio:output_file",
            m_output_file,
            "Name of EDM4hep/podio output file to write to. Setting this will cause the output file to be created and written to."
    );

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
    japp->SetDefaultParameter(
            "podio:output_file_copy_dir",
            m_output_file_copy_dir,
            "Directory name to make an additional copy of the output file to. Copy will be done at end of processing. Default is empty string which means do not make a copy. No check is made on path existing."
    );

    // Get the list of output collections to include/exclude
    std::vector<std::string> output_include_collections;  // need to get as vector, then convert to set
    std::vector<std::string> output_exclude_collections;  // need to get as vector, then convert to set
    japp->SetDefaultParameter(
            "podio:output_include_collections",
            output_include_collections,
            "Comma separated list of collection names to write out. If not set, all collections will be written (including ones from input file). Don't set this and use PODIO:OUTPUT_EXCLUDE_COLLECTIONS to write everything except a selection."
    );
    japp->SetDefaultParameter(
            "podio:output_exclude_collections",
            output_exclude_collections,
            "Comma separated list of collection names to not write out."
    );
    m_output_include_collections = std::set<std::string>(output_include_collections.begin(),
                                                         output_include_collections.end());
    m_output_exclude_collections = std::set<std::string>(output_exclude_collections.begin(),
                                                         output_exclude_collections.end());

}

void JEventProcessorPODIO::Init() {

    auto app = GetApplication();
    m_log = app->GetService<Log_service>()->logger("JEventProcessorPODIO");

    m_store = new eic::EventStore();
    m_writer = std::make_shared<eic::ROOTWriter>(m_output_file, m_store);
}


void JEventProcessorPODIO::Process(const std::shared_ptr<const JEvent> &event) {

    // Set up the set of collections_to_write
    if (m_is_first_event) {
        std::set<std::string> all_factory_collections;
        for (auto fac : event->GetAllFactories()) {
            all_factory_collections.insert(fac->GetTag());
        }

        if (m_output_include_collections.empty()) {
            // User has not specified an include list, so we include _all_ JFactories present in first event.
            // (Non-PODIO types will be ignored later)
            m_collections_to_write = all_factory_collections;
        }
        else {
            // We match up the include list with what is actually present in the JFactorySet
            std::set_intersection(m_output_include_collections.begin(),
                                  m_output_include_collections.end(),
                                  all_factory_collections.begin(),
                                  all_factory_collections.end(),
                                  std::inserter(m_collections_to_write, m_collections_to_write.begin()));
        }

        // We remove any collections on the exclude list
        // (whether it be from the include list or the list of all factories)
        for (const auto& col : m_output_exclude_collections) {
            m_collections_to_write.erase(col);
        }
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    m_log->trace("=================================="); 
    m_log->trace("Event #{}", event->GetEventNumber()); 


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

            auto result = CallWithPODIOType<InsertFacIntoStore, size_t, JFactory*, eic::EventStore*, bool>(fac->GetObjectName(), fac, m_store, m_is_first_event);

            if (result == std::nullopt) { 
                m_log->debug("Unrecognized PODIO type '{}:{}', ignoring.", fac->GetObjectName(), fac->GetTag()); 
            }
            else {
                m_log->info("Successfully added PODIO type '{}:{}' for writing.", fac->GetObjectName(), fac->GetTag());
                if (m_is_first_event) {
                    // We only want to register for write once, since internally PODIO uses a vector such that
                    // duplicates cause segfaults.
                    // We only register if we have also confirmed that this is actually a valid PODIO type.
                    m_writer->registerForWrite(fac->GetTag());
                }
            }
        }
        catch(std::exception &e){
            m_log->error("Exception adding PODIO type '{}:{}': {}.", fac->GetObjectName(), fac->GetTag(), e.what());
        }
    }
    m_writer->writeEvent();
    m_store->clearCollections();
    m_is_first_event = false;
}

void JEventProcessorPODIO::Finish() {
    m_writer->finish();
}

