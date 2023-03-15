
#include "JEventProcessorPODIO.h"
#include <services/log/Log_service.h>
#include <JANA/Services/JComponentManager.h>

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

            // Ecal stuff
            "EcalEndcapNRawHits",
            "EcalEndcapNRecHits",
            "EcalEndcapNTruthClusters",
            "EcalEndcapNClusters",
            "EcalEndcapNMergedClusters",
            "EcalEndcapNTruthClusterAssociations",
            "EcalEndcapNClusterAssociations",
            "EcalEndcapNMergedClusterAssociations",
            "EcalEndcapPRawHits",
            "EcalEndcapPRecHits",
            "EcalEndcapPTruthClusters",
            "EcalEndcapPClusters",
            "EcalEndcapPMergedClusters",
            "EcalEndcapPTruthClusterAssociations",
            "EcalEndcapPClusterAssociations",
            "EcalEndcapPMergedClusterAssociations",
            "EcalEndcapPInsertRawHits",
            "EcalEndcapPInsertRecHits",
            "EcalEndcapPInsertTruthClusters",
            "EcalEndcapPInsertClusters",
            "EcalEndcapPInsertMergedClusters",
            "EcalEndcapPInsertTruthClusterAssociations",
            "EcalEndcapPInsertClusterAssociations",
            "EcalEndcapPInsertMergedClusterAssociations",
            "EcalBarrelSciGlassRawHits",
            "EcalBarrelSciGlassRecHits",
            "EcalBarrelSciGlassClusters",
            "EcalBarrelSciGlassMergedClusters",
            "EcalBarrelSciGlassTruthClusters",
            "EcalBarrelSciGlassMergedTruthClusters",
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
            "HcalEndcapPMergedClusterAssociations",
            "HcalEndcapPInsertRawHits",
            "HcalEndcapPInsertRecHits",
            "HcalEndcapPInsertMergedHits",
            "HcalEndcapPInsertClusters",
            "HcalBarrelRawHits",
            "HcalBarrelRecHits",
            "HcalBarrelClusters",
            "B0ECalRawHits",
            "B0ECalRecHits",
            "B0ECalClusters",
            "ZDCEcalRawHits",
            "ZDCEcalRecHits",
            "ZDCEcalClusters",
            "ZDCEcalMergedClusters",
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

    m_output_include_collections = std::set<std::string>(output_include_collections.begin(),
                                                         output_include_collections.end());
    m_output_exclude_collections = std::set<std::string>(output_exclude_collections.begin(),
                                                         output_exclude_collections.end());

}


void JEventProcessorPODIO::Init() {

    auto app = GetApplication();
    m_log = app->GetService<Log_service>()->logger("JEventProcessorPODIO");
    m_log->set_level(spdlog::level::debug);
    m_writer = std::make_unique<podio::ROOTFrameWriter>(m_output_filename);
    // TODO: Does ROOTFrameWriter validate that the output filename is indeed writable BEFORE attempting to close it
    //       after all processing is complete?

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

    if (m_user_included_collections) {
        // Activate factories
        static std::set<std::string> failing_collections;
        for (const std::string& coll : m_collections_to_write) {
            if (failing_collections.count(coll) != 0) { continue; }
            try {
                m_log->trace("Ensuring factory for collection '{}' has been called.", coll);
                event->GetCollectionBase(coll);
            }
            catch(std::exception &e) {
                // Limit printing warning to just once per factory
                failing_collections.insert(coll);
                m_log->warn("Exception adding PODIO collection '{}': {}.", coll, e.what());
            }
        }
    }

    // Frame will contain data from all Podio factories that have been triggered,
    // including by the `event->GetCollectionBase(coll);` above.
    auto* frame = event->GetSingle<podio::Frame>();
    m_writer->writeFrame(*frame, "events", m_collections_to_write);
    m_is_first_event = false;
}

void JEventProcessorPODIO::Finish() {
    m_writer->finish();
}
