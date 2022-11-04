// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//
// This is a JANA event source that uses PODIO to read from a ROOT
// file created using the EDM4hep Data Model.
//
// This uses the podio supplied RootReader and EventStore classes. Thus,
// it is limited to processing only a single event at a time.

#include "JEventSourcePODIOsimple.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <filesystem>
#include <fmt/color.h>

#include <JANA/JFactoryGenerator.h>

// podio specific includes
#include <podio/EventStore.h>
#include <podio/IReader.h>
#include <podio/UserDataCollection.h>
#include <podio/podioVersion.h>

#include <fmt/format.h>

// This file is generated automatically by make_datamodel_glue.py
#include "datamodel_glue.h"


//------------------------------------------------------------------------------
// CopyPodioToJEventSimpleT
//
/// This is called from the GetEvent method below by way of the generated code
/// in datamodel_glue.h. This will make copies of the high-level podio objects
/// that wrap the same "Obj" mid-level objects managed by the collection.
/// The copies of the high-level objects are managed by JANA and can be accessed
/// via the standard JANA event.Get<>() mechanism.
///
/// \tparam T           podio high-level data type (e.g. edm4hep::EventHeader)
/// \tparam Tcollection podio collection type (e.g. edm4hep::EventHeaderCollection)
/// \param collection   pointer to the podio collection (e.g. edm4hep::EventHeaderCollection*)
/// \param name         name of the collection which will be used as the factory tag for these objects
/// \param event        JANA JEvent to copy the data objects into
//------------------------------------------------------------------------------
template <typename T, typename Tcollection>
void CopyToJEventSimpleT(const Tcollection *collection, const std::string &name, std::shared_ptr<JEvent> &event){

    std::vector<const T*> tptrs;
    for( int i=0; i<collection->size(); i++){
        const auto &obj = (*collection)[i];  // Create new object of type "T" on stack that uses existing "Obj" object.
        tptrs.push_back( new T(obj) ); // Create new object of type "T" on heap that uses existing "Obj" object.
    }
    event->Insert( tptrs, name );
}

//------------------------------------------------------------------------------
// Constructor
//
///
/// \param resource_name  Name of root file to open (n.b. file is not opened until Open() is called)
/// \param app            JApplication
//------------------------------------------------------------------------------
JEventSourcePODIOsimple::JEventSourcePODIOsimple(std::string resource_name, JApplication* app) : JEventSource(resource_name, app) {
    SetTypeName(NAME_OF_THIS); // Provide JANA with class name

    // Tell JANA that we want it to call the FinishEvent() method.
    EnableFinishEvent();

    // Allow user to specify to recycle events forever
    GetApplication()->SetDefaultParameter(
            "podio:run_forever",
            m_run_forever,
            "set to true to recycle through events continuously"
            );

    bool print_type_table = false;
    GetApplication()->SetDefaultParameter(
            "podio:print_type_table",
            print_type_table,
            "Print list of collection names and their types"
            );

    std::string background_filename;
    GetApplication()->SetDefaultParameter(
            "podio:background_filename",
            background_filename,
            "Name of file containing background events to merge in (default is not to merge any background)"
            );

    int num_background_events=1;
    GetApplication()->SetDefaultParameter(
            "podio:num_background_events",
            num_background_events,
            "Number of background events to add to every primary event."
    );

}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
JEventSourcePODIOsimple::~JEventSourcePODIOsimple() {
    LOG << "Closing Event Source for " << GetResourceName() << LOG_END;
    for( auto &[bg_reader, bg_store, ievent] : readers_background ){
        delete bg_store;
        delete bg_reader;
    }
}

//------------------------------------------------------------------------------
// Open
//
/// Open the root file and read in metadata.
//------------------------------------------------------------------------------
void JEventSourcePODIOsimple::Open() {

    bool print_type_table = GetApplication()->GetParameterValue<bool>("podio:print_type_table");
    std::string background_filename = GetApplication()->GetParameterValue<std::string>("podio:background_filename");;
    int num_background_events = GetApplication()->GetParameterValue<int>("podio:num_background_events");;

    // Open primary events file
    try {

        // Verify file exists
        if( ! std::filesystem::exists(GetResourceName()) ){
            // Here we go against the standard practice of throwing an error and print
            // the message and exit immediately. This is because we want the last message
            // on the screen to be that the file doesn't exist.
            auto mess = fmt::format(fmt::emphasis::bold | fg(fmt::color::red),"ERROR: ");
            mess += fmt::format(fmt::emphasis::bold, "file: {} does not exist!",  GetResourceName());
            std::cerr << std::endl << std::endl << mess << std::endl << std::endl;
            _exit(-1);
        }

        // Have PODIO reader open file and get the number of events from it.
        reader.openFile( GetResourceName() );
        if( ! reader.isValid() ) throw std::runtime_error( fmt::format("podio ROOTReader says {} is invalid", GetResourceName()) );

        auto version = reader.currentFileVersion();
        bool version_mismatch = version.major > podio::version::build_version.major;
        version_mismatch |= (version.major == podio::version::build_version.major) && (version.minor>podio::version::build_version.minor);
        if( version_mismatch ) {
            std::stringstream ss;
            ss << "Mismatch in PODIO versions! " << version << " > " << podio::version::build_version;
            // FIXME: The podio ROOTReader is somehow failing to read in the correct version numbers from the file
//            throw JException(ss.str());
        }

        LOG << "PODIO version: file=" << version << " (executable=" << podio::version::build_version << ")" << LOG_END;

        Nevents_in_file = reader.getEntries();
        LOG << "Opened PODIO file \"" << GetResourceName() << "\" with " << Nevents_in_file << " events" << LOG_END;

        store.setReader(&reader);
        reader.readEvent();

        if( print_type_table ) PrintCollectionTypeTable();

    }catch (std::exception &e ){
        LOG_ERROR(default_cerr_logger) << e.what() << LOG_END;
        throw JException( fmt::format( "Problem opening file \"{}\"", GetResourceName() ) );
    }

    // If the user specified a background events file, then create dedicated readers
    // and EventStores for the number of background events to be added to each primary event.
    // This seems a bit over-the-top, but podio likes the data objects to be owned by a
    // collection and a collection to be owned by an EventStore. Thus, we do it this way.
    if( ! background_filename.empty() ) {
        for (int i = 0; i < num_background_events; i++) {
            auto bg_reader = new podio::ROOTReader();
            auto bg_store = new podio::EventStore();
            bg_reader->openFile( background_filename );
            if (!bg_reader->isValid())
                throw std::runtime_error(fmt::format("podio ROOTReader says background events file {} is invalid", background_filename));

            auto version = bg_reader->currentFileVersion();
            bool version_mismatch = version.major > podio::version::build_version.major;
            version_mismatch |= (version.major == podio::version::build_version.major) &&
                                (version.minor > podio::version::build_version.minor);
            if (version_mismatch) {
                std::stringstream ss;
                ss << "Mismatch in PODIO versions for background file! " << version << " > " << podio::version::build_version;
                // FIXME: The podio ROOTReader is somehow failing to read in the correct version numbers from the file
//            throw JException(ss.str());
            }

            // Need to offset where events are read from each file so they are not duplicating background events
            // (at least any more than necessary)
            auto Nentries_background = bg_reader->getEntries();
            auto offset = (readers_background.size()*Nentries_background)/num_background_events;

            bg_store->setReader(bg_reader);
            bg_reader->goToEvent(offset);
            bg_reader->readEvent();

            readers_background.emplace_back(bg_reader, bg_store, offset);
        }

        LOG << "Merging " << readers_background.size() << " background events from " << background_filename << LOG_END;
    }
}

//------------------------------------------------------------------------------
// GetEvent
//
/// Read next event from file and copy its objects into the given JEvent.
///
/// \param event
//------------------------------------------------------------------------------
void JEventSourcePODIOsimple::GetEvent(std::shared_ptr<JEvent> event) {

    /// Calls to GetEvent are synchronized with each other, which means they can
    /// read and write state on the JEventSource without causing race conditions.

    // Check if we have exhausted events from file
    if( Nevents_read >= Nevents_in_file ) {
        if( m_run_forever ){
            Nevents_read = 0;
        }else{
            reader.closeFile();
            throw RETURN_STATUS::kNO_MORE_EVENTS;
        }
    }

    // The podio supplied RootReader and EventStore are not multi-thread capable so limit to a single event in flight
    // Since the JANA skip events mechanism does not seem to call FinishEvent when skipping, any flag for
    // in-flight events will not be cleared. Thus, use the JEvent pointer so that
    std::thread::id no_thread; // default value is no thread
    if( processing_thread_id == no_thread ){
        // No thread is processing event
        processing_thread_id = std::this_thread::get_id();
    }else if( processing_thread_id != std::this_thread::get_id() ){
        // Another thread is already processing the event
        throw RETURN_STATUS ::kBUSY;
    }

    // Read the specified event into the EventStore and make the EventStore pointer available via JANA
    store.clear();
    reader.endOfEvent();
    reader.goToEvent( Nevents_read++ );
    auto fac = event->Insert( &store );
    fac->SetFactoryFlag(JFactory::NOT_OBJECT_OWNER); // jana should not delete this

    // Loop over collections in EventStore and copy pointers to their contents into jevent
    auto collectionIDtable = store.getCollectionIDTable();
    for( auto id : collectionIDtable->ids() ){
         podio::CollectionBase *coll={nullptr};
        if( store.get(id, coll) ){
            auto name = collectionIDtable->name(id);
            auto className = coll->getTypeName();
            CopyToJEventSimple( className, name, coll, event);

            if( name == "EventHeader"){
                auto ehc = reinterpret_cast<const edm4hep::EventHeaderCollection *>(coll);
                if( ehc && ehc->size() ){
                    event->SetEventNumber( (*ehc)[0].getEventNumber());
                    event->SetRunNumber( (*ehc)[0].getRunNumber());
                }
            }
        }
    }

    // If user specified to add background hits, do that here
    for( auto &[bg_reader, bg_store, ievent] : readers_background ){
        bg_store->clear();
        bg_reader->endOfEvent();
        if( ++ievent >= bg_reader->getEntries() ) ievent = 0; // rewind if we have used all events
        bg_reader->goToEvent( ievent );
        event->Insert( bg_store ); // factory already flagged as NOT_OBJECT_OWNER above
        auto bg_collectionIDtable = bg_store->getCollectionIDTable();
        for (auto id: bg_collectionIDtable->ids()) {
            podio::CollectionBase *coll = {nullptr};
            if (bg_store->get(id, coll)) {
                auto name = bg_collectionIDtable->name(id);
                auto className = coll->getTypeName();
                CopyToJEventSimple(className, name, coll, event); // n.b. this will add to existing objects
            }
        }
    }
}

//------------------------------------------------------------------------------
// FinishEvent
//
/// Clear the flag used to limit us to a single event in flight.
///
/// \param event
//------------------------------------------------------------------------------
void JEventSourcePODIOsimple::FinishEvent(JEvent &event){

    processing_thread_id = std::thread::id(); // reset to no thread value
}

//------------------------------------------------------------------------------
// GetDescription
//------------------------------------------------------------------------------
std::string JEventSourcePODIOsimple::GetDescription() {

    /// GetDescription() helps JANA explain to the user what is going on
    return "PODIO root file (simple)";
}

//------------------------------------------------------------------------------
// CheckOpenable
//
/// Return a value from 0-1 indicating probability that this source will be
/// able to read this root file. Currently, it simply checks that the file
/// name contains the string ".root" and if does, returns a small number (0.02).
/// This will need to be made more sophisticated if the alternative root file
/// formats need to be supported by other event sources.
///
/// \param resource_name name of root file to evaluate.
/// \return              value from 0-1 indicating confidence that this source can open the given file
//------------------------------------------------------------------------------
template <>
double JEventSourceGeneratorT<JEventSourcePODIOsimple>::CheckOpenable(std::string resource_name) {

    // If source is a root file, given minimal probability of success so we're chosen
    // only if no other ROOT file sources exist.
    return (resource_name.find(".root") != std::string::npos ) ? 0.02 : 0.0;
}

//------------------------------------------------------------------------------
// PrintCollectionTypeTable
//
/// Print the list of collection names from the currently open file along
/// with their types. This will be called automatically when the file is
/// open if the PODIO:PRINT_TYPE_TABLE variable is set to a non-zero value
//------------------------------------------------------------------------------
void JEventSourcePODIOsimple::PrintCollectionTypeTable(void) {

    // First, get maximum length of the collection name strings so
    // we can print nicely aligned columns.
    size_t max_name_len = 0;
    size_t max_type_len = 0;
    std::map<std::string, std::string> collectionNames;
    auto collectionIDtable = store.getCollectionIDTable();
    for (auto id : collectionIDtable->ids()) {
        auto name = collectionIDtable->name(id);
        podio::CollectionBase *coll = {nullptr};
        if (store.get(id, coll)) {
            auto type = coll->getTypeName();
            max_name_len = std::max(max_name_len, name.length());
            max_type_len = std::max(max_type_len, type.length());
            collectionNames[name] = type;
        }
    }

    // Print table
    std::cout << std::endl;
    std::cout << "Available Collections" << std::endl;
    std::cout << std::endl;
    std::cout << "Collection Name" << std::string(max_name_len + 2 - std::string("Collection Name").length(), ' ')
              << "Data Type" << std::endl;
    std::cout << std::string(max_name_len, '-') << "  " << std::string(max_name_len, '-') << std::endl;
    for (auto &[name, type] : collectionNames) {
        std::cout << name + std::string(max_name_len + 2 - name.length(), ' ');
        std::cout << type << std::endl;
    }
    std::cout << std::endl;

}

