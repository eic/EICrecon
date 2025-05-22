
#include "addDummyEntryProcesser.h"

#include <JANA/JApplication.h>
#include <JANA/JLogger.h>
#include <JANA/Services/JParameterManager.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <fmt/core.h>
#include <podio/CollectionBase.h>
#include <podio/Frame.h>
#include <podio/ROOTWriter.h>
#include <exception>
#include <ostream>
#include <stdexcept>

#include "services/log/Log_service.h"

addDummyEntryProcesser::addDummyEntryProcesser() {
  SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

  japp->SetDefaultParameter("podio:output_file", m_output_file_addDummy,
                            "Name of EDM4hep/podio output file to write to. Setting this will "
                            "cause the output file to be created and written to.");

  // Allow user to set PODIO:OUTPUT_FILE to "1" to specify using the default name.
  if (m_output_file_addDummy == "1") {
    auto param = japp->GetJParameterManager()->FindParameter("podio:output_file");
    if (param) {
      param->SetValue(param->GetDefault());
      m_output_file_addDummy = param->GetDefault();
    }
  }

  // Get the output directory path for creating a second copy of the output file at the end of processing.
  // (this is duplicating similar functionality in Juggler/Gaudi so assume it is useful).
  japp->SetDefaultParameter("podio:output_file_copy_dir", m_output_file_copy_dir_addDummy,
                            "Directory name to make an additional copy of the output file to. Copy "
                            "will be done at end of processing. Default is empty string which "
                            "means do not make a copy. No check is made on path existing.");

  // Get the list of output collections to include/exclude
  std::vector<std::string> output_collections = {
    // Header and other metadata
    "EventHeader",

    // Truth record
    // "MCParticles",
    // "MCBeamElectrons",
    // "MCBeamProtons",
    // "MCScatteredElectrons",
    // "MCScatteredProtons",
    // "MCParticlesHeadOnFrameNoBeamFX",


    // // Si tracker hits
    // "SiBarrelTrackerRecHits",
    // "SiBarrelVertexRecHits",
    // "SiEndcapTrackerRecHits",

    // "SiBarrelRawHits",
    // "SiBarrelVertexRawHits",
    // "SiEndcapTrackerRawHits",

    // "SiBarrelHits",
    // "VertexBarrelHits",
    // "TrackerEndcapHits",

    // "SiBarrelRawHitAssociations",
    // "SiBarrelVertexRawHitAssociations",
    // "SiEndcapTrackerRawHitAssociations",

#if EDM4EIC_VERSION_MAJOR >= 6
    "HadronicFinalState",
#endif

    // Kuma Checker
    
    "B0TrackerHitsWDummy",
    "BackwardMPGDEndcapHitsWDummy",
    "DIRCBarHitsWDummy",
    "DRICHHitsWDummy",
    "ForwardMPGDEndcapHitsWDummy", 
    "ForwardOffMTrackerHitsWDummy",
    "ForwardRomanPotHitsWDummy", 
    "LumiSpecTrackerHitsWDummy",
    "MPGDBarrelHitsWDummy",
    "OuterMPGDBarrelHitsWDummy", 
    "RICHEndcapNHitsWDummy",
    "SiBarrelHitsWDummy",
    "TOFBarrelHitsWDummy",
    "TOFEndcapHitsWDummy",
    "TaggerTrackerHitsWDummy",
    "TrackerEndcapHitsWDummy",
    "VertexBarrelHitsWDummy",
    // Calorimeter hits
    "B0ECalHitsWDummy",
    "EcalBarrelImagingHitsWDummy",
    "EcalBarrelScFiHitsWDummy",
    "EcalEndcapNHitsWDummy",
    "EcalEndcapPHitsWDummy",
    "EcalEndcapPInsertHitsWDummy",
    "EcalFarForwardZDCHitsWDummy",
    "EcalLumiSpecHitsWDummy",
    "HcalBarrelHitsWDummy",
    "HcalEndcapNHitsWDummy",
    "HcalEndcapPInsertHitsWDummy",
    "HcalFarForwardZDCHitsWDummy",
    "LFHCALHitsWDummy",
    "LumiDirectPCALHitsWDummy",
    // "EcalBarrelScFiRawHits",
    "B0ECalHitsContributionsWDummy",
    "EcalBarrelImagingHitsContributionsWDummy",
    "EcalBarrelScFiHitsContributionsWDummy",
    "EcalEndcapNHitsContributionsWDummy",
    "EcalEndcapPHitsContributionsWDummy",
    "EcalEndcapPInsertHitsContributionsWDummy",
    "EcalLumiSpecHitsContributionsWDummy",
    "EcalFarForwardZDCHitsContributionsWDummy",
    "HcalBarrelHitsContributionsWDummy",
    "HcalEndcapNHitsContributionsWDummy",
    "HcalEndcapPInsertHitsContributionsWDummy",
    "HcalFarForwardZDCHitsContributionsWDummy",
    "LFHCALHitsContributionsWDummy",
    "LumiDirectPCALHitsContributionsWDummy",

#if EDM4EIC_VERSION_MAJOR >= 7
    // "B0ECalRawHitAssociations",
    // "EcalBarrelScFiRawHitAssociations",
    // "EcalBarrelImagingRawHitAssociations",
    // "HcalBarrelRawHitAssociations",
    // "EcalEndcapNRawHitAssociations",
    // "HcalEndcapNRawHitAssociations",
    // "EcalEndcapPRawHitAssociations",
    // "EcalEndcapPInsertRawHitAssociations",
    // "HcalEndcapPInsertRawHitAssociations",
    // "LFHCALRawHitAssociations",
    // "EcalLumiSpecRawHitAssociations",
    // "EcalFarForwardZDCRawHitAssociations",
    // "HcalFarForwardZDCRawHitAssociations",
#endif
#if EDM4EIC_VERSION_MAJOR >= 8
    // "TrackClusterMatches",
#endif

  };
  std::vector<std::string> output_exclude_collections; // need to get as vector, then convert to set
  std::string output_include_collections = "DEPRECATED";
  japp->SetDefaultParameter("podio:output_include_collections", output_include_collections,
                            "DEPRECATED. Use podio:output_collections instead.");
  if (output_include_collections != "DEPRECATED") {
    output_collections.clear();
    JParameterManager::Parse(output_include_collections, output_collections);
    m_output_include_collections_set_addDummy = true;
  }
  japp->SetDefaultParameter(
      "podio:output_collections", output_collections,
      "Comma separated list of collection names to write out. If not set, all collections will be "
      "written (including ones from input file). Don't set this and use "
      "PODIO:OUTPUT_EXCLUDE_COLLECTIONS to write everything except a selection.");
  japp->SetDefaultParameter("podio:output_exclude_collections", output_exclude_collections,
                            "Comma separated list of collection names to not write out.");
  japp->SetDefaultParameter(
      "podio:print_collections", m_collections_to_print_addDummy,
      "Comma separated list of collection names to print to screen, e.g. for debugging.");

  m_output_collections_addDummy =
      std::set<std::string>(output_collections.begin(), output_collections.end());
  m_output_exclude_collections_addDummy =
      std::set<std::string>(output_exclude_collections.begin(), output_exclude_collections.end());
}

void addDummyEntryProcesser::Init() {

  auto* app = GetApplication();
  m_log_addDummy     = app->GetService<Log_service>()->logger("addDummyEntryProcesser");
  m_writer_addDummy  = std::make_unique<podio::ROOTWriter>(m_output_file_addDummy);
  // TODO: NWB: Verify that output file is writable NOW, rather than after event processing completes.
  //       I definitely don't trust PODIO to do this for me.

  if (m_output_include_collections_set_addDummy) {
    m_log_addDummy->error("The podio:output_include_collections was provided, but is deprecated. Use "
                 "podio:output_collections instead.");
    throw std::runtime_error("The podio:output_include_collections was provided, but is "
                             "deprecated. Use podio:output_collections instead.");
  }
}

void addDummyEntryProcesser::FindCollectionsToWrite(const std::shared_ptr<const JEvent>& event) {

  // Set up the set of collections_to_write.
  std::vector<std::string> all_collections = event->GetAllCollectionNames();

  if (m_output_collections_addDummy.empty()) {
    // User has not specified an include list, so we include _all_ PODIO collections present in the first event.
    for (const std::string& col : all_collections) {
      if (m_output_exclude_collections_addDummy.find(col) == m_output_exclude_collections_addDummy.end()) {
        m_collections_to_write_addDummy.push_back(col);
        m_log_addDummy->info("Persisting collection '{}'", col);
      }
    }
  } else {
    m_log_addDummy->debug("Persisting podio types from includes list");
    m_user_included_collections_addDummy = true;

    // We match up the include list with what is actually present in the event
    std::set<std::string> all_collections_set =
        std::set<std::string>(all_collections.begin(), all_collections.end());

    for (const auto& col : m_output_collections_addDummy) {
      if (m_output_exclude_collections_addDummy.find(col) == m_output_exclude_collections_addDummy.end()) {
        // Included and not excluded
        if (all_collections_set.find(col) == all_collections_set.end()) {
          // Included, but not a valid PODIO type
          m_log_addDummy->warn("Explicitly included collection '{}' not present in factory set, omitting.",
                      col);
        } else {
          // Included, not excluded, and a valid PODIO type
          m_collections_to_write_addDummy.push_back(col);
          m_log_addDummy->info("Persisting collection '{}'", col);
        }
      }
    }
  }
}

void addDummyEntryProcesser::Process(const std::shared_ptr<const JEvent>& event) {

    std::lock_guard<std::mutex> lock(m_mutex_addDummy);
    if (m_is_first_event_addDummy) {
        FindCollectionsToWrite(event);
    }

    // Trigger all collections once to fix the collection IDs
    // TODO: WDC: This should not be necessary, but while we await collection IDs
    //            that are determined by hash, we have to ensure they are reproducible
    //            even if the collections are filled in unpredictable order (or not at
    //            all). See also below, at "TODO: NWB:".
    for (const auto& coll_name : m_collections_to_write_addDummy) {
        try {
        [[maybe_unused]] const auto* coll_ptr = event->GetCollectionBase(coll_name);
        } catch (std::exception& e) {
        // chomp
        }
    }

    // Print the contents of some collections, just for debugging purposes
    // Do this before writing just in case writing crashes
    if (!m_collections_to_print_addDummy.empty()) {
        LOG << "========================================" << LOG_END;
        LOG << "addDummyEntryProcesser: Event " << event->GetEventNumber() << LOG_END;
    }
    for (const auto& coll_name : m_collections_to_print_addDummy) {
        LOG << "------------------------------" << LOG_END;
        LOG << coll_name << LOG_END;
        try {
        const auto* coll_ptr = event->GetCollectionBase(coll_name);
        if (coll_ptr == nullptr) {
            LOG << "missing" << LOG_END;
        } else {
            coll_ptr->print();
        }
        } catch (std::exception& e) {
        LOG << "missing" << LOG_END;
        }
    }

    m_log_addDummy->trace("==================================");
    m_log_addDummy->trace("Event #{}", event->GetEventNumber());

    // Make sure that all factories get called that need to be written into the frame.
    // We need to do this for _all_ factories unless we've constrained it by using includes/excludes.
    // Note that all collections need to be present in the first event, as podio::RootFrameWriter constrains us to write one event at a time, so there
    // is no way to add a new branch after the first event.

    // If we get an exception below while trying to add a factory for any
    // reason then mark that factory as bad and don't try running it again.
    // This is motivated by trying to write EcalBarrelSciGlass objects for
    // data simulated using the imaging calorimeter. In that case, it will
    // always throw an exception, but DD4hep also prints its own error message.
    // Thus, to prevent that error message every event, we must avoid calling
    // it.

    // Activate factories.
    // TODO: NWB: For now we run every factory every time, swallowing exceptions if necessary.
    //            We do this so that we always have the same collections created in the same order.
    //            This means that the collection IDs are stable so the writer doesn't segfault.
    //            The better fix is to maintain a map of collection IDs, or just wait for PODIO to fix the bug.
    std::vector<std::string> successful_collections;
    static std::set<std::string> failed_collections;
    for (const std::string& coll : m_collections_to_write_addDummy) {
        try {
        m_log_addDummy->trace("Ensuring factory for collection '{}' has been called.", coll);
        const auto* coll_ptr = event->GetCollectionBase(coll);
        if (coll_ptr == nullptr) {
            // If a collection is missing from the frame, the podio root writer will segfault.
            // To avoid this, we treat this as a failing collection and omit from this point onwards.
            // However, this code path is expected to be unreachable because any missing collection will be
            // replaced with an empty collection in JFactoryPodioTFixed::Create.
            if (failed_collections.count(coll) == 0) {
            m_log_addDummy->error("Omitting PODIO collection '{}' because it is null", coll);
            failed_collections.insert(coll);
            }
        } else {
            m_log_addDummy->trace("Including PODIO collection '{}'", coll);
            successful_collections.push_back(coll);
        }
        } catch (std::exception& e) {
        // Limit printing warning to just once per factory
        if (failed_collections.count(coll) == 0) {
            m_log_addDummy->error("Omitting PODIO collection '{}' due to exception: {}.", coll, e.what());
            failed_collections.insert(coll);
        }
        }
    }
    m_collections_to_write_addDummy = successful_collections;

    // Frame will contain data from all Podio factories that have been triggered,
    // including by the `event->GetCollectionBase(coll);` above.
    // Note that collections MUST be present in frame. If a collection is null, the writer will segfault.
    const auto* frame = event->GetSingle<podio::Frame>();

    // TODO: NWB: We need to actively stabilize podio collections. Until then, keep this around in case
    //            the writer starts segfaulting, so we can quickly see whether the problem is unstable collection IDs.
    /*
        m_log->info("Event {}: Writing {} collections", event->GetEventNumber(), m_collections_to_write.size());
        for (const std::string& collname : m_collections_to_write) {
            m_log->info("Writing collection '{}' with id {}", collname, frame->get(collname)->getID());
        }
        */
    m_writer_addDummy->writeFrame(*frame, "events", m_collections_to_write_addDummy);

    // Add dummy entries to the output file
    for(int iDummyEntry = 0; iDummyEntry < 5; ++iDummyEntry) {  
        podio::Frame empty_frame;
        // empty_frame.put(std::make_unique<edm4eic::TrackerHitCollection>(), "SiBarrelHits");
        // empty_frame.put(std::make_unique<edm4eic::CalorimeterHitCollection>(), "EcalBarrelHits");
        m_writer_addDummy->writeFrame(empty_frame, "events", m_collections_to_write_addDummy);

        // m_writer_addDummy->writeFrame(*frame, "events", m_collections_to_write_addDummy);

    }
    

    m_is_first_event_addDummy = false;
}

void addDummyEntryProcesser::Finish() {
  if (m_output_include_collections_set_addDummy) {
    m_log_addDummy->error("The podio:output_include_collections was provided, but is deprecated. Use "
                 "podio:output_collections instead.");
    throw std::runtime_error("The podio:output_include_collections was provided, but is "
                             "deprecated. Use podio:output_collections instead.");
  }

  m_writer_addDummy->finish();
}


