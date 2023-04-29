
#include "JEventProcessorPODIO.h"
#include <services/log/Log_service.h>
#include <JANA/Services/JComponentManager.h>
#include <podio/Frame.h>

#include <datamodel_glue.h>
#include <algorithm>


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
    std::vector<std::string> output_include_collections={
            "MCParticles",

            // All tracking hits combined
            "CentralTrackingRecHits",
            "CentralTrackSeedingResults",

            // Si tracker hits
            "SiBarrelTrackerRecHits",
            "SiBarrelVertexRecHits",
            "SiEndcapTrackerRecHits",

            // TOF
            "TOFBarrelRecHit",
            "TOFEndcapRecHits",

            // MPGD
            "MPGDBarrelRecHits",
            "MPGDDIRCRecHits",

            // Forward & Far forward hits
            "ForwardOffMTrackerRecHits",
            "ForwardRomanPotRecHits",
            "B0TrackerRecHits",

            //
            "ForwardRomanPotRecParticles",
            "SmearedFarForwardParticles",

            // Reconstructed data
            "GeneratedParticles",
            "ReconstructedParticles",
            "ReconstructedChargedParticles",
            "ReconstructedChargedParticlesAssociations",
            "CentralTrackSegments",
            "InclusiveKinematicsDA",
            "InclusiveKinematicsJB",
            "InclusiveKinematicsSigma",
            "InclusiveKinematicseSigma",
            "InclusiveKinematicsElectron",
            "InclusiveKinematicsTruth",
            "GeneratedJets",
            "ReconstructedJets",

            // Ecal stuff
            "EcalEndcapNRawHits",
            "EcalEndcapNRecHits",
            "EcalEndcapNTruthClusters",
            "EcalEndcapNClusters",
            "EcalEndcapNTruthClusterAssociations",
            "EcalEndcapNClusterAssociations",
            "EcalEndcapPRawHits",
            "EcalEndcapPRecHits",
            "EcalEndcapPTruthClusters",
            "EcalEndcapPClusters",
            "EcalEndcapPTruthClusterAssociations",
            "EcalEndcapPClusterAssociations",
            "EcalEndcapPInsertRawHits",
            "EcalEndcapPInsertRecHits",
            "EcalEndcapPInsertTruthClusters",
            "EcalEndcapPInsertClusters",
            "EcalEndcapPInsertTruthClusterAssociations",
            "EcalEndcapPInsertClusterAssociations",
            "EcalBarrelSciGlassRawHits",
            "EcalBarrelSciGlassRecHits",
            "EcalBarrelSciGlassClusters",
            "EcalBarrelSciGlassTruthClusters",
            "EcalBarrelImagingRawHits",
            "EcalBarrelImagingRecHits",
            "EcalBarrelImagingClusters",
            "EcalBarrelImagingMergedClusters",
            "EcalBarrelScFiRawHits",
            "EcalBarrelScFiRecHits",
            "EcalBarrelScFiMergedHits",
            "EcalBarrelScFiClusters",
            "HcalEndcapNRawHits",
            "HcalEndcapNRecHits",
            "HcalEndcapNMergedHits",
            "HcalEndcapNClusters",
            "HcalEndcapPRawHits",   // this causes premature exit of eicrecon
            "HcalEndcapPRecHits",
            "HcalEndcapPMergedHits",
            "HcalEndcapPClusters",
            "HcalEndcapPTruthClusterAssociations",
            "HcalEndcapPClusterAssociations",
            "HcalEndcapPInsertRawHits",
            "HcalEndcapPInsertRecHits",
            "HcalEndcapPInsertMergedHits",
            "HcalEndcapPInsertClusters",
            "LFHCALRawHits",
            "LFHCALRecHits",
            "LFHCALClusters",
            "HcalBarrelRawHits",
            "HcalBarrelRecHits",
            "HcalBarrelClusters",
            "B0ECalRawHits",
            "B0ECalRecHits",
            "B0ECalClusters",
            "ZDCEcalRawHits",
            "ZDCEcalRecHits",
            "ZDCEcalClusters",
            "HcalEndcapNTruthClusters",
//            "HcalEndcapPTruthClusters",  // This gives lots of errors from volume manager on "unknown identifier"
            "HcalBarrelTruthClusters",
            "B0ECalRecHits",
            "B0ECalClusters",
            "ZDCEcalTruthClusters"
    };
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
    japp->SetDefaultParameter(
            "podio:print_collections",
            m_collections_to_print,
            "Comma separated list of collection names to print to screen, e.g. for debugging."
    );

    m_output_include_collections = std::set<std::string>(output_include_collections.begin(),
                                                         output_include_collections.end());
    m_output_exclude_collections = std::set<std::string>(output_exclude_collections.begin(),
                                                         output_exclude_collections.end());

}


void JEventProcessorPODIO::Init() {

    auto app = GetApplication();
    m_log = app->GetService<Log_service>()->logger("JEventProcessorPODIO");
    m_log->set_level(spdlog::level::debug);
    m_writer = std::make_unique<podio::ROOTFrameWriter>(m_output_file);
    // TODO: NWB: Verify that output file is writable NOW, rather than after event processing completes.
    //       I definitely don't trust PODIO to do this for me.

}


void JEventProcessorPODIO::FindCollectionsToWrite(const std::shared_ptr<const JEvent>& event) {

    // Set up the set of collections_to_write.
    std::vector<std::string> all_collections = event->GetAllCollectionNames();

    if (m_output_include_collections.empty()) {
        // User has not specified an include list, so we include _all_ PODIO collections present in the first event.
        for (const std::string& col : all_collections) {
            if (m_output_exclude_collections.find(col) == m_output_exclude_collections.end()) {
                m_collections_to_write.push_back(col);
                m_log->info("Persisting collection '{}'", col);
            }
        }
    }
    else {
        m_log->debug("Persisting podio types from includes list");
        m_user_included_collections = true;

        // We match up the include list with what is actually present in the event
        std::set<std::string> all_collections_set = std::set<std::string>(all_collections.begin(), all_collections.end());

        for (const auto& col : m_output_include_collections) {
            if (m_output_exclude_collections.find(col) == m_output_exclude_collections.end()) {
                // Included and not excluded
                if (all_collections_set.find(col) == all_collections_set.end()) {
                    // Included, but not a valid PODIO type
                    m_log->warn("Explicitly included collection '{}' not present in factory set, omitting.", col);
                }
                else {
                    // Included, not excluded, and a valid PODIO type
                    m_collections_to_write.push_back(col);
                    m_log->info("Persisting collection '{}'", col);
                }
            }
        }
    }

}

void JEventProcessorPODIO::Process(const std::shared_ptr<const JEvent> &event) {

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_is_first_event) {
        FindCollectionsToWrite(event);
    }

    // Print the contents of some collections, just for debugging purposes
    // Do this before writing just in case writing crashes
    if (!m_collections_to_print.empty()) {
        LOG << "========================================" << LOG_END;
        LOG << "JEventProcessorPODIO: Event " << event->GetEventNumber() << LOG_END;
    }
    for (const auto& coll_name : m_collections_to_print) {
        LOG << "------------------------------" << LOG_END;
        LOG << coll_name << LOG_END;
        event->GetCollectionBase(coll_name)->print();
    }

    m_log->trace("==================================");
    m_log->trace("Event #{}", event->GetEventNumber());


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
    // std::vector<std::string> successful_collections;  // TODO: NWB: Re-enable me
    static std::set<std::string> failed_collections;
    for (const std::string& coll : m_collections_to_write) {
        try {
            m_log->trace("Ensuring factory for collection '{}' has been called.", coll);
            const auto* coll_ptr = event->GetCollectionBase(coll);
            if (coll_ptr == nullptr) {
                // If a collection is missing from the frame, the podio root writer will segfault.
                // To avoid this, we treat this as a failing collection and omit from this point onwards.
                // However, this code path is expected to be unreachable because any missing collection will be
                // replaced with an empty collection in JFactoryPodioTFixed::Create.
                if (failed_collections.count(coll) == 0) {
                    m_log->error("Omitting PODIO collection '{}' because it is null", coll);
                    failed_collections.insert(coll);
                }
            }
            else {
                // successful_collections.push_back(coll); // TODO: NWB: Re-enable me
            }
        }
        catch(std::exception &e) {
            // Limit printing warning to just once per factory
            if (failed_collections.count(coll) == 0) {
                m_log->error("Omitting PODIO collection '{}' due to exception: {}.", coll, e.what());
                failed_collections.insert(coll);
            }
        }
    }
    // m_collections_to_write = successful_collections; // TODO: NWB: Re-enable me

    // Frame will contain data from all Podio factories that have been triggered,
    // including by the `event->GetCollectionBase(coll);` above.
    // Note that collections MUST be present in frame. If a collection is null, the writer will segfault.
    auto* frame = event->GetSingle<podio::Frame>();

    // TODO: NWB: We need to actively stabilize podio collections. Until then, keep this around in case
    //            the writer starts segfaulting, so we can quickly see whether the problem is unstable collection IDs.
    /*
    m_log->info("Event {}: Writing {} collections", event->GetEventNumber(), m_collections_to_write.size());
    for (const std::string& collname : m_collections_to_write) {
        m_log->info("Writing collection '{}' with id {}", collname, frame->get(collname)->getID());
    }
    */
    m_writer->writeFrame(*frame, "events", m_collections_to_write);
    m_is_first_event = false;

}

void JEventProcessorPODIO::Finish() {
    m_writer->finish();
}
