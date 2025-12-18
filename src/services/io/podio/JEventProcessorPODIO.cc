
#include "JEventProcessorPODIO.h"

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Services/JParameterManager.h>
#include <JANA/Utils/JTypeInfo.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <podio/CollectionBase.h>
#include <podio/Frame.h>
#include <podio/ROOTWriter.h>
#include <algorithm>
#include <exception>
#include <functional>
#include <iterator>
#include <regex>
#include <sstream>

#include "services/log/Log_service.h"

JEventProcessorPODIO::JEventProcessorPODIO() {
  SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

  japp->SetDefaultParameter("podio:output_file", m_output_file,
                            "Name of EDM4hep/podio output file to write to. Setting this will "
                            "cause the output file to be created and written to.");

  // Allow user to set PODIO:OUTPUT_FILE to "1" to specify using the default name.
  if (m_output_file == "1") {
    auto* param = japp->GetJParameterManager()->FindParameter("podio:output_file");
    if (param != nullptr) {
      param->SetValue(param->GetDefault());
      m_output_file = param->GetDefault();
    }
  }

  // Get the output directory path for creating a second copy of the output file at the end of processing.
  // (this is duplicating similar functionality in Juggler/Gaudi so assume it is useful).
  japp->SetDefaultParameter("podio:output_file_copy_dir", m_output_file_copy_dir,
                            "Directory name to make an additional copy of the output file to. Copy "
                            "will be done at end of processing. Default is empty string which "
                            "means do not make a copy. No check is made on path existing.");

  // Get the list of output collections to include/exclude
  std::vector<std::string> output_collections = {
      // Header and other metadata
      "EventHeader",

      // Truth record
      "MCParticles",
      "MCBeamElectrons",
      "MCBeamProtons",
      "MCScatteredElectrons",
      "MCScatteredProtons",
      "MCParticlesHeadOnFrameNoBeamFX",

      // Central tracking hits combined
      "CentralTrackerTruthSeeds",
      "CentralTrackingRecHits",
      "CentralTrackingRawHitAssociations",
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

      "SiBarrelRawHitAssociations",
      "SiBarrelVertexRawHitAssociations",
      "SiEndcapTrackerRawHitAssociations",

      // TOF
      "TOFBarrelRecHits",
      "TOFEndcapRecHits",

      "TOFBarrelRawHits",
      "TOFEndcapRawHits",

      "TOFBarrelHits",
      "TOFBarrelClusterHits",
      "TOFBarrelADCTDC",
      "TOFEndcapHits",

      "TOFEndcapSharedHits",
      "TOFEndcapADCTDC",

      "TOFBarrelRawHitAssociations",
      "TOFEndcapRawHitAssociations",

      "CombinedTOFTruthSeededParticleIDs",
      "CombinedTOFParticleIDs",

      // DRICH
      "DRICHRawHits",
      "DRICHRawHitsAssociations",
      "DRICHAerogelTracks",
      "DRICHGasTracks",
      "DRICHAerogelIrtCherenkovParticleID",
      "DRICHGasIrtCherenkovParticleID",
      "DRICHTruthSeededParticleIDs",
      "DRICHParticleIDs",

      // PFRICH
      "RICHEndcapNRawHits",
      "RICHEndcapNRawHitsAssociations",
      "RICHEndcapNTruthSeededParticleIDs",
      "RICHEndcapNParticleIDs",

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

      "MPGDBarrelRawHitAssociations",
      "OuterMPGDBarrelRawHitAssociations",
      "BackwardMPGDEndcapRawHitAssociations",
      "ForwardMPGDEndcapRawHitAssociations",

      // LOWQ2 hits
      "TaggerTrackerHits",
      "TaggerTrackerSharedHits",
      "TaggerTrackerHitPulses",
      "TaggerTrackerCombinedPulses",
      "TaggerTrackerCombinedPulsesWithNoise",
      "TaggerTrackerRawHits",
      "TaggerTrackerRawHitAssociations",
      "TaggerTrackerM1L0ClusterPositions",
      "TaggerTrackerM1L1ClusterPositions",
      "TaggerTrackerM1L2ClusterPositions",
      "TaggerTrackerM1L3ClusterPositions",
      "TaggerTrackerM2L0ClusterPositions",
      "TaggerTrackerM2L1ClusterPositions",
      "TaggerTrackerM2L2ClusterPositions",
      "TaggerTrackerM2L3ClusterPositions",
      "TaggerTrackerM1LocalTracks",
      "TaggerTrackerM2LocalTracks",
      "TaggerTrackerM1LocalTrackAssociations",
      "TaggerTrackerM2LocalTrackAssociations",
      "TaggerTrackerLocalTracks",
      "TaggerTrackerLocalTrackAssociations",
      "TaggerTrackerReconstructedParticles",
      "TaggerTrackerReconstructedParticleAssociations",

      // Forward & Far forward hits
      "B0TrackerTruthSeeds",
      "B0TrackerRecHits",
      "B0TrackerRawHits",
      "B0TrackerHits",
      "B0TrackerRawHitAssociations",
      "B0TrackerSeedingResults",
      "B0TrackerMeasurements",

      "ForwardRomanPotRecHits",
      "ForwardOffMTrackerRecHits",

      "ForwardRomanPotRecParticles",
      "ForwardRomanPotStaticRecParticles",
      "ForwardOffMRecParticles",

      "ForwardRomanPotRawHits",
      "ForwardRomanPotRawHitAssociations",
      "ForwardOffMTrackerRawHits",
      "ForwardOffMTrackerRawHitAssociations",

      // Reconstructed data
      "GeneratedParticles",
      "GeneratedBreitFrameParticles",
      "ReconstructedParticles",
      "ReconstructedParticleAssociations",
      "ReconstructedTruthSeededChargedParticles",
      "ReconstructedTruthSeededChargedParticleAssociations",
      "ReconstructedChargedRealPIDParticles",
      "ReconstructedChargedRealPIDParticleIDs",
      "ReconstructedChargedParticles",
      "ReconstructedChargedParticleAssociations",
      "MCScatteredElectronAssociations",    // Remove if/when used internally
      "MCNonScatteredElectronAssociations", // Remove if/when used internally
      "ReconstructedBreitFrameParticles",

      // Central tracking
      "CentralTrackSegments",
      "CentralTrackVertices",
      "CentralCKFTruthSeededTrajectories",
      "CentralCKFTruthSeededTracks",
      "CentralCKFTruthSeededTrackAssociations",
      "CentralCKFTruthSeededTrackParameters",
      "CentralCKFTrajectories",
      "CentralCKFTracks",
      "CentralCKFTrackAssociations",
      "CentralCKFTrackParameters",
      // tracking properties - true seeding
      "CentralCKFTruthSeededTrajectoriesUnfiltered",
      "CentralCKFTruthSeededTracksUnfiltered",
      "CentralCKFTruthSeededTrackUnfilteredAssociations",
      "CentralCKFTruthSeededTrackParametersUnfiltered",
      // tracking properties - realistic seeding
      "CentralCKFTrajectoriesUnfiltered",
      "CentralCKFTracksUnfiltered",
      "CentralCKFTrackUnfilteredAssociations",
      "CentralCKFTrackParametersUnfiltered",

      // B0 tracking
      "B0TrackerCKFTruthSeededTrajectories",
      "B0TrackerCKFTruthSeededTracks",
      "B0TrackerCKFTruthSeededTrackAssociations",
      "B0TrackerCKFTruthSeededTrackParameters",
      "B0TrackerCKFTrajectories",
      "B0TrackerCKFTracks",
      "B0TrackerCKFTrackAssociations",
      "B0TrackerCKFTrackParameters",
      // tracking properties - true seeding
      "B0TrackerCKFTruthSeededTrajectoriesUnfiltered",
      "B0TrackerCKFTruthSeededTracksUnfiltered",
      "B0TrackerCKFTruthSeededTrackUnfilteredAssociations",
      "B0TrackerCKFTruthSeededTrackParametersUnfiltered",
      // tracking properties - realistic seeding
      "B0TrackerCKFTrajectoriesUnfiltered",
      "B0TrackerCKFTrackParametersUnfiltered",
      "B0TrackerCKFTracksUnfiltered",
      "B0TrackerCKFTrackUnfilteredAssociations",

      "CentralAndB0TrackVertices",

      // Inclusive kinematics
      "InclusiveKinematicsDA",
      "InclusiveKinematicsJB",
      "InclusiveKinematicsML",
      "InclusiveKinematicsSigma",
      "InclusiveKinematicseSigma", // Deprecated, use ESigma
      "InclusiveKinematicsESigma",
      "InclusiveKinematicsElectron",
      "InclusiveKinematicsTruth",
      "GeneratedJets",
      "GeneratedChargedJets",
      "GeneratedCentauroJets",
      "ReconstructedJets",
      "ReconstructedChargedJets",
      "ReconstructedCentauroJets",
      "ReconstructedElectrons",
      "ScatteredElectronsTruth",
      "ScatteredElectronsEMinusPz",
      "PrimaryVertices",
      "SecondaryVerticesHelix",
      "BarrelClusters",
      "HadronicFinalState",

      // Track projections
      "CalorimeterTrackProjections",

      // Ecal stuff
      "EcalEndcapNRawHits",
      "EcalEndcapNRecHits",
      "EcalEndcapNTruthClusters",
      "EcalEndcapNTruthClusterAssociations",
      "EcalEndcapNClusters",
      "EcalEndcapNClusterAssociations",
      "EcalEndcapNSplitMergeClusters",
      "EcalEndcapNSplitMergeClusterAssociations",
      "EcalEndcapPRawHits",
      "EcalEndcapPRecHits",
      "EcalEndcapPTruthClusters",
      "EcalEndcapPTruthClusterAssociations",
      "EcalEndcapPClusters",
      "EcalEndcapPClusterAssociations",
      "EcalEndcapPSplitMergeClusters",
      "EcalEndcapPSplitMergeClusterAssociations",
      "EcalBarrelClusters",
      "EcalBarrelClusterAssociations",
      "EcalBarrelTruthClusters",
      "EcalBarrelTruthClusterAssociations",
      "EcalBarrelImagingRawHits",
      "EcalBarrelImagingRecHits",
      "EcalBarrelImagingClusters",
      "EcalBarrelImagingClusterAssociations",
      "EcalBarrelScFiPAttenuatedHits",
      "EcalBarrelScFiPAttenuatedHitContributions",
      "EcalBarrelScFiNAttenuatedHits",
      "EcalBarrelScFiNAttenuatedHitContributions",
      "EcalBarrelScFiRawHits",
      "EcalBarrelScFiPPulses",
      "EcalBarrelScFiNPulses",
      "EcalBarrelScFiPCombinedPulses",
      "EcalBarrelScFiNCombinedPulses",
      "EcalBarrelScFiPCombinedPulsesWithNoise",
      "EcalBarrelScFiNCombinedPulsesWithNoise",
      "EcalBarrelScFiRecHits",
      "EcalBarrelScFiClusters",
      "EcalBarrelScFiClusterAssociations",
      "EcalBarrelScFiTopoClusters", // ScFi clusters based in Topological Clustering
      "EcalBarrelScFiTopoClusterAssociations",
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
      "HcalEndcapNSplitMergeClusters",
      "HcalEndcapNSplitMergeClusterAssociations",
      "HcalEndcapPInsertRawHits",
      "HcalEndcapPInsertRecHits",
      "HcalEndcapPInsertMergedHits",
      "HcalEndcapPInsertClusters",
      "HcalEndcapPInsertClusterAssociations",
      "LFHCALRawHits",
      "LFHCALRecHits",
      "LFHCALClusters",
      "LFHCALClusterAssociations",
      "LFHCALSplitMergeClusters",
      "LFHCALSplitMergeClusterAssociations",
      "HcalBarrelRawHits",
      "HcalBarrelRecHits",
      "HcalBarrelMergedHits",
      "HcalBarrelClusters",
      "HcalBarrelClusterAssociations",
      "HcalBarrelSplitMergeClusters",
      "HcalBarrelSplitMergeClusterAssociations",
      "B0ECalRawHits",
      "B0ECalRecHits",
      "B0ECalClusters",
      "B0ECalClusterAssociations",
      "HcalEndcapNTruthClusters",
      "HcalEndcapNTruthClusterAssociations",
      "HcalBarrelTruthClusters",
      "HcalBarrelTruthClusterAssociations",

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
      "ReconstructedFarForwardZDCNeutrals",
      "ReconstructedFarForwardZDCLambdas",
      "ReconstructedFarForwardZDCLambdaDecayProductsCM",

      // DIRC
      "DIRCRawHits",
      "DIRCTruthSeededParticleIDs",
      "DIRCParticleIDs",

      "B0ECalRawHitAssociations",
      "EcalBarrelScFiRawHitAssociations",
      "EcalBarrelImagingRawHitAssociations",
      "HcalBarrelRawHitAssociations",
      "EcalEndcapNRawHitAssociations",
      "HcalEndcapNRawHitAssociations",
      "EcalEndcapPRawHitAssociations",
      "HcalEndcapPInsertRawHitAssociations",
      "LFHCALRawHitAssociations",
      "EcalLumiSpecRawHitAssociations",
      "EcalFarForwardZDCRawHitAssociations",
      "HcalFarForwardZDCRawHitAssociations",
      "EcalEndcapPTrackClusterMatches",
      "LFHCALTrackClusterMatches",
      "HcalEndcapPInsertClusterMatches",
      "EcalBarrelTrackClusterMatches",
      "HcalBarrelTrackClusterMatches",
      "EcalEndcapNTrackClusterMatches",
      "HcalEndcapNTrackClusterMatches",

  };
  std::vector<std::string> output_exclude_collections; // need to get as vector, then convert to set
  japp->SetDefaultParameter(
      "podio:output_collections", output_collections,
      "Comma separated list of collection names to write out. If not set, all collections will be "
      "written (including ones from input file). Don't set this and use "
      "PODIO:OUTPUT_EXCLUDE_COLLECTIONS to write everything except a selection.");
  japp->SetDefaultParameter("podio:output_exclude_collections", output_exclude_collections,
                            "Comma separated list of collection names to not write out.");
  japp->SetDefaultParameter(
      "podio:print_collections", m_collections_to_print,
      "Comma separated list of collection names to print to screen, e.g. for debugging.");

  m_output_collections =
      std::set<std::string>(output_collections.begin(), output_collections.end());
  m_output_exclude_collections =
      std::set<std::string>(output_exclude_collections.begin(), output_exclude_collections.end());
}

void JEventProcessorPODIO::Init() {

  auto* app = GetApplication();
  m_log     = app->GetService<Log_service>()->logger("JEventProcessorPODIO");
  m_writer  = std::make_unique<podio::ROOTWriter>(m_output_file);
}

void JEventProcessorPODIO::FindCollectionsToWrite(const std::shared_ptr<const JEvent>& event) {

  // Set up the set of collections_to_write.
  std::vector<std::string> all_collections = event->GetAllCollectionNames();

  if (m_output_collections.empty()) {
    // User has not specified an include list, so we include _all_ PODIO collections present in the first event.
    for (const std::string& col : all_collections) {
      if (m_output_exclude_collections.find(col) == m_output_exclude_collections.end()) {
        m_collections_to_write.push_back(col);
        m_log->debug("Persisting collection '{}'", col);
      }
    }
  } else {
    m_log->debug("Persisting podio types from includes list");

    // We match up the include list with what is actually present in the event
    std::set<std::string> all_collections_set =
        std::set<std::string>(all_collections.begin(), all_collections.end());

    // Turn regexes among output collections into actual collection names
    std::set<std::string> matching_collections_set;
    std::vector<std::regex> output_collections_regex(m_output_collections.size());
    std::ranges::transform(m_output_collections, output_collections_regex.begin(),
                           [](const std::string& r) { return std::regex(r); });
    std::ranges::copy_if(all_collections_set,
                         std::inserter(matching_collections_set, matching_collections_set.end()),
                         [&](const std::string& c) {
                           return std::ranges::any_of(
                               output_collections_regex,

                               [&](const std::regex& r) { return std::regex_match(c, r); });
                         });

    for (const auto& col : matching_collections_set) {
      if (m_output_exclude_collections.find(col) == m_output_exclude_collections.end()) {
        // Included and not excluded
        if (all_collections_set.find(col) == all_collections_set.end()) {
          // Included, but not a valid PODIO type
          m_log->warn("Explicitly included collection '{}' not present in factory set, omitting.",
                      col);
        } else {
          // Included, not excluded, and a valid PODIO type
          m_collections_to_write.push_back(col);
          m_log->info("Persisting collection '{}'", col);
        }
      }
    }
  }
}

void JEventProcessorPODIO::Process(const std::shared_ptr<const JEvent>& event) {

  // Find all collections to write from the first event
  std::call_once(m_is_first_event, &JEventProcessorPODIO::FindCollectionsToWrite, this, event);

  // Print the contents of some collections, just for debugging purposes
  // Do this before writing just in case writing crashes
  if (!m_collections_to_print.empty()) {
    m_log->info("========================================");
    m_log->info("JEventProcessorPODIO: Event {}", event->GetEventNumber());
    ;
  }
  for (const auto& coll_name : m_collections_to_print) {
    m_log->info("------------------------------");
    m_log->info("{}", coll_name);
    try {
      const auto* coll_ptr = event->GetCollectionBase(coll_name);
      if (coll_ptr == nullptr) {
        m_log->info("missing");
      } else {
        std::stringstream ss;
        coll_ptr->print(ss);
        m_log->info(ss.str());
      }
    } catch (std::exception& e) {
      m_log->info("missing");
    }
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
  std::vector<std::string> successful_collections;
  std::set<std::string> failed_collections;
  for (const std::string& coll : m_collections_to_write) {
    try {
      m_log->trace("Ensuring factory for collection '{}' has been called.", coll);
      const auto* coll_ptr = event->GetCollectionBase(coll);
      if (coll_ptr == nullptr) {
        // If a collection is missing from the frame, the podio root writer will segfault.
        // To avoid this, we treat this as a failing collection and omit from this point onwards.
        // However, this code path is expected to be unreachable because any missing collection will be
        // replaced with an empty collection in JFactoryPodioTFixed::Create.
        if (!failed_collections.contains(coll)) {
          m_log->error("Omitting PODIO collection '{}' because it is null", coll);
          failed_collections.insert(coll);
        }
      } else {
        m_log->trace("Including PODIO collection '{}'", coll);
        successful_collections.push_back(coll);
      }
    } catch (std::exception& e) {
      // Limit printing warning to just once per factory
      if (!failed_collections.contains(coll)) {
        m_log->error("Omitting PODIO collection '{}' due to exception: {}.", coll, e.what());
        failed_collections.insert(coll);
      }
    }
  }

  // Frame will contain data from all Podio factories that have been triggered,
  // including by the `event->GetCollectionBase(coll);` above.
  // Note that collections MUST be present in frame. If a collection is null, the writer will segfault.
  const auto* frame = event->GetSingle<podio::Frame>();
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_writer->writeFrame(*frame, "events", m_collections_to_write);
  }
}

void JEventProcessorPODIO::Finish() { m_writer->finish(); }
