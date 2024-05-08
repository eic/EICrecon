
#include "JEventProcessorPODIO.h"

#include <edm4eic/EDM4eicVersion.h>

#include <JANA/JApplication.h>
#include <JANA/JLogger.h>
#include <JANA/Services/JParameterManager.h>
#include <JANA/Utils/JTypeInfo.h>
#include <fmt/core.h>
#include <podio/CollectionBase.h>
#include <podio/Frame.h>
#include <spdlog/common.h>
#include <exception>

#include "services/log/Log_service.h"


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
            // Header and other metadata
            "EventHeader",

            // Truth record
            "MCParticles",
            "MCBeamElectrons",
            "MCBeamProtons",
            "MCScatteredElectrons",
            "MCScatteredProtons",

            // All tracking hits combined
            "CentralTrackingRecHits",
            "CentralTrackSeedingResults",
            "CentralTrackerMeasurements",

            // Si tracker hits
            "SiBarrelTrackerRecHits",
            "SiBarrelVertexRecHits",
            "SiEndcapTrackerRecHits",

            "SiBarrelRawHits",
            "SiBarrelVertexRawHits",
            "SiEndcapTrackerRawHits",

            "SiBarrelHits",
            "VertexBarrelHits",
            "TrackerEndcapHits",

            "SiBarrelHitAssociations",
            "SiBarrelVertexHitAssociations",
            "SiEndcapHitAssociations",

            // TOF
            "TOFBarrelRecHit",
            "TOFEndcapRecHits",

            "TOFBarrelRawHit",
            "TOFEndcapRawHits",

            "TOFBarrelHits",
            "TOFEndcapHits",

            "TOFBarrelHitAssociations",
            "TOFEndcapHitAssociations",

            "CombinedTOFParticleIDs",
            "CombinedTOFSeededParticleIDs",

            // DRICH
            "DRICHRawHits",
            "DRICHRawHitsAssociations",
            "DRICHAerogelTracks",
            "DRICHGasTracks",
            "DRICHAerogelIrtCherenkovParticleID",
            "DRICHGasIrtCherenkovParticleID",
            "DRICHParticleIDs",
            "DRICHSeededParticleIDs",

            // PFRICH
            "RICHEndcapNRawHits",
            "RICHEndcapNRawHitsAssociations",
            "RICHEndcapNParticleIDs",
            "RICHEndcapNSeededParticleIDs",

            // MPGD
            "MPGDBarrelRecHits",
            "OuterMPGDBarrelRecHits",
            "BackwardMPGDEndcapRecHits",
            "ForwardMPGDEndcapRecHits",

            "MPGDBarrelRawHits",
            "OuterMPGDBarrelRawHits",
            "BackwardMPGDEndcapRawHits",
            "ForwardMPGDEndcapRawHits",

            "MPGDBarrelHits",
            "OuterMPGDBarrelHits",
            "BackwardMPGDEndcapHits",
            "ForwardMPGDEndcapHits",

            "MPGDBarrelHitAssociations",
            "OuterMPGDBarrelHitAssociations",
            "BackwardMPGDEndcapAssociations",
            "ForwardMPGDHitAssociations",

            // LOWQ2 hits
            "TaggerTrackerRawHits",
            "TaggerTrackerHitAssociations",
            "TaggerTrackerM1L0ClusterPositions",
            "TaggerTrackerM1L1ClusterPositions",
            "TaggerTrackerM1L2ClusterPositions",
            "TaggerTrackerM1L3ClusterPositions",
            "TaggerTrackerM2L0ClusterPositions",
            "TaggerTrackerM2L1ClusterPositions",
            "TaggerTrackerM2L2ClusterPositions",
            "TaggerTrackerM2L3ClusterPositions",
            "TaggerTrackerM1Tracks",
            "TaggerTrackerM2Tracks",
            "TaggerTrackerProjectedTracks",

            // Forward & Far forward hits
            "B0TrackerRecHits",
            "B0TrackerRawHits",
            "B0TrackerHits",
            "B0TrackerHitAssociations",

            "ForwardRomanPotRecHits",
            "ForwardOffMTrackerRecHits",

            "ForwardRomanPotRecParticles",
            "ForwardOffMRecParticles",

            "ForwardRomanPotHitAssociations",
            "ForwardOffMTrackerHitAssociations",

            // Reconstructed data
            "GeneratedParticles",
            "ReconstructedParticles",
            "ReconstructedParticleAssociations",
            "ReconstructedChargedParticles",
            "ReconstructedChargedParticleAssociations",
            "ReconstructedChargedRealPIDParticles",
            "ReconstructedSeededChargedParticles",
            "ReconstructedSeededChargedParticleAssociations",
            "MCScatteredElectronAssociations", // Remove if/when used internally
            "MCNonScatteredElectronAssociations", // Remove if/when used internally
            "ReconstructedChargedParticleIDs",
            "ReconstructedBreitFrameParticles",
            "CentralTrackSegments",
            "CentralTrackVertices",
            "CentralCKFTrajectories",
            "CentralCKFTracks",
            "CentralCKFTrackParameters",
            "CentralCKFSeededTrajectories",
            "CentralCKFSeededTracks",
            "CentralCKFSeededTrackParameters",
            "InclusiveKinematicsDA",
            "InclusiveKinematicsJB",
            "InclusiveKinematicsSigma",
            "InclusiveKinematicseSigma",
            "InclusiveKinematicsElectron",
            "InclusiveKinematicsTruth",
            "GeneratedJets",
            "GeneratedChargedJets",
            "ReconstructedJets",
            "ReconstructedChargedJets",
            "ReconstructedElectrons",
            "ScatteredElectronsTruth",
            "ScatteredElectronsEMinusPz",
#if EDM4EIC_VERSION_MAJOR >= 6
            "HadronicFinalState",
#endif

            // Track projections
            "CalorimeterTrackProjections",

            // Ecal stuff
            "EcalEndcapNRawHits",
            "EcalEndcapNRecHits",
            "EcalEndcapNTruthClusters",
            "EcalEndcapNTruthClusterAssociations",
            "EcalEndcapNClusters",
            "EcalEndcapNClusterAssociations",
            "EcalEndcapPRawHits",
            "EcalEndcapPRecHits",
            "EcalEndcapPTruthClusters",
            "EcalEndcapPTruthClusterAssociations",
            "EcalEndcapPClusters",
            "EcalEndcapPClusterAssociations",
            "EcalEndcapPInsertRawHits",
            "EcalEndcapPInsertRecHits",
            "EcalEndcapPInsertTruthClusters",
            "EcalEndcapPInsertTruthClusterAssociations",
            "EcalEndcapPInsertClusters",
            "EcalEndcapPInsertClusterAssociations",
            "EcalBarrelClusters",
            "EcalBarrelClusterAssociations",
            "EcalBarrelTruthClusters",
            "EcalBarrelTruthClusterAssociations",
            "EcalBarrelImagingRawHits",
            "EcalBarrelImagingRecHits",
            "EcalBarrelImagingClusters",
            "EcalBarrelImagingClusterAssociations",
            "EcalBarrelScFiRawHits",
            "EcalBarrelScFiRecHits",
            "EcalBarrelScFiClusters",
            "EcalBarrelScFiClusterAssociations",
            "EcalLumiSpecRawHits",
            "EcalLumiSpecRecHits",
            "EcalLumiSpecTruthClusters",
            "EcalLumiSpecTruthClusterAssociations",
            "EcalLumiSpecClusters",
            "EcalLumiSpecClusterAssociations",
            "HcalEndcapNRawHits",
            "HcalEndcapNRecHits",
            "HcalEndcapNMergedHits",
            "HcalEndcapNClusters",
            "HcalEndcapNClusterAssociations",
            "HcalEndcapPInsertRawHits",
            "HcalEndcapPInsertRecHits",
            "HcalEndcapPInsertMergedHits",
            "HcalEndcapPInsertClusters",
            "HcalEndcapPInsertClusterAssociations",
            "LFHCALRawHits",
            "LFHCALRecHits",
            "LFHCALClusters",
            "LFHCALClusterAssociations",
            "HcalBarrelRawHits",
            "HcalBarrelRecHits",
            "HcalBarrelClusters",
            "HcalBarrelClusterAssociations",
            "B0ECalRawHits",
            "B0ECalRecHits",
            "B0ECalClusters",
            "B0ECalClusterAssociations",
            "HcalEndcapNTruthClusters",
            "HcalEndcapNTruthClusterAssociations",
            "HcalBarrelTruthClusters",
            "HcalBarrelTruthClusterAssociations",
            "B0ECalRecHits",
            "B0ECalClusters",
            "B0ECalClusterAssociations",

            //ZDC Ecal
            "EcalFarForwardZDCRawHits",
            "EcalFarForwardZDCRecHits",
            "EcalFarForwardZDCClusters",
            "EcalFarForwardZDCClusterAssociations",
            "EcalFarForwardZDCTruthClusters",
            "EcalFarForwardZDCTruthClusterAssociations",

            //ZDC HCal
            "HcalFarForwardZDCRawHits",
            "HcalFarForwardZDCRecHits",
            "HcalFarForwardZDCSubcellHits",
            "HcalFarForwardZDCClusters",
            "HcalFarForwardZDCClusterAssociations",
            "HcalFarForwardZDCClustersBaseline",
            "HcalFarForwardZDCClusterAssociationsBaseline",
            "HcalFarForwardZDCTruthClusters",
            "HcalFarForwardZDCTruthClusterAssociations",
            "HcalFarForwardZDCNeutronCandidates",

            // DIRC
            "DIRCRawHits",
            "DIRCPID",
            "DIRCParticleIDs",
            "DIRCSeededParticleIDs",
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

    auto *app = GetApplication();
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

    for (const std::string& coll : m_collections_to_write) {
        try {
            m_log->trace("Ensuring factory for collection '{}' has been called.", coll);
            const auto* coll_ptr = event->GetCollectionBase(coll);
            if (coll_ptr == nullptr) {
                m_log->critical("A null pointer received when obtaining PODIO collection '{}'.", coll);
            }
        }
        catch(std::exception &e) {
            m_log->critical("Exception caught when obtaining PODIO collection '{}': {}.", coll, e.what());
            throw;
        }
    }

    // Frame will contain data from all Podio factories that have been triggered,
    // including by the `event->GetCollectionBase(coll);` above.
    // Note that collections MUST be present in frame. If a collection is null, the writer will segfault.
    const auto* frame = event->GetSingle<podio::Frame>();

    m_writer->writeFrame(*frame, "events", m_collections_to_write);
    m_is_first_event = false;

    // Print the contents of some collections, just for debugging purposes
    // Do this before writing just in case writing crashes
    if (!m_collections_to_print.empty()) {
        LOG << "========================================" << LOG_END;
        LOG << "JEventProcessorPODIO: Event " << event->GetEventNumber() << LOG_END;
    }
    for (const auto& coll_name : m_collections_to_print) {
        LOG << "------------------------------" << LOG_END;
        LOG << coll_name << LOG_END;
        try {
            const auto* coll_ptr = event->GetCollectionBase(coll_name);
            if (coll_ptr == nullptr) {
                LOG << "missing" << LOG_END;
            } else {
                coll_ptr->print();
            }
        }
        catch(std::exception &e) {
            LOG << "missing" << LOG_END;
        }
    }

    m_log->trace("==================================");
    m_log->trace("Event #{}", event->GetEventNumber());

}

void JEventProcessorPODIO::Finish() {
    m_writer->finish();
}
