#include "JEventProcessorPODIO.h"

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSource.h>
#include <JANA/Services/JComponentManager.h>
#include <JANA/Services/JParameterManager.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <fmt/format.h>
#include <podio/CollectionBase.h>
#include <podio/Frame.h>
#include <podio/Writer.h>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <exception>
#include <functional>
#include <iterator>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include "extensions/jana/JComponentManager_compat.h"
#include "services/io/podio/JEventSourcePODIO.h"
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
      "TrackerTruthSeeds",
      "TrackerTruthSeedParameters",
      "CentralTrackerTruthSeeds",
      "CentralTrackingRecHits",
      "CentralTrackingRawHitLinks",
      "CentralTrackingRawHitAssociations",
      "CentralTrackSeeds",
      "CentralTrackSeedParameters",
      "CentralTrackerMeasurements",
      "CentralWithoutTOFTrackerMeasurements",

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

      "SiBarrelRawHitLinks",
      "SiBarrelRawHitAssociations",
      "SiBarrelVertexRawHitLinks",
      "SiBarrelVertexRawHitAssociations",
      "SiEndcapTrackerRawHitLinks",
      "SiEndcapTrackerRawHitAssociations",

      // TOF
      "TOFBarrelHits",
      "TOFBarrelClusterHits",
      "TOFEndcapClusterHits",
      "TOFBarrelADCTDC",
      "TOFEndcapHits",

      "TOFEndcapSharedHits",
      "TOFEndcapADCTDC",

      "TOFBarrelRawHitLinks",
      "TOFBarrelRawHitAssociations",
      "TOFEndcapRawHitLinks",
      "TOFEndcapRawHitAssociations",

      "CombinedTOFTruthSeededParticleIDs",
      "CombinedTOFParticleIDs",

      // DRICH
      "DRICHRawHits",
      "DRICHRawHitsLinks",
      "DRICHRawHitsAssociations",
      "DRICHAerogelTracks",
      "DRICHGasTracks",
      "DRICHAerogelIrtCherenkovParticleID",
      "DRICHGasIrtCherenkovParticleID",
      "DRICHTruthSeededParticleIDs",
      "DRICHParticleIDs",

      // PFRICH
      "RICHEndcapNRawHits",
      "RICHEndcapNRawHitsLinks",
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

      "MPGDBarrelRawHitLinks",
      "MPGDBarrelRawHitAssociations",
      "OuterMPGDBarrelRawHitLinks",
      "OuterMPGDBarrelRawHitAssociations",
      "BackwardMPGDEndcapRawHitLinks",
      "BackwardMPGDEndcapRawHitAssociations",
      "ForwardMPGDEndcapRawHitLinks",
      "ForwardMPGDEndcapRawHitAssociations",

      // LOWQ2 hits
      "TaggerTrackerHits",
      "TaggerTrackerSharedHits",
      "TaggerTrackerHitPulses",
      "TaggerTrackerCombinedPulses",
      "TaggerTrackerCombinedPulsesWithNoise",
      "TaggerTrackerRawHits",
      "TaggerTrackerRawHitLinks",
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
      "TaggerTrackerM1LocalTrackLinks",
      "TaggerTrackerM1LocalTrackAssociations",
      "TaggerTrackerM2LocalTrackLinks",
      "TaggerTrackerM2LocalTrackAssociations",
      "TaggerTrackerLocalTracks",
      "TaggerTrackerLocalTrackLinks",
      "TaggerTrackerLocalTrackAssociations",
      "TaggerTrackerReconstructedParticles",
      "TaggerTrackerReconstructedParticleLinks",
      "TaggerTrackerReconstructedParticleAssociations",

      // Forward & Far forward hits
      "B0TrackerTruthSeeds",
      "B0TrackerRecHits",
      "B0TrackerRawHits",
      "B0TrackerHits",
      "B0TrackerRawHitLinks",
      "B0TrackerRawHitAssociations",
      "B0TrackerSeeds",
      "B0TrackerSeedParameters",
      "B0TrackerMeasurements",

      "ForwardRomanPotRecHits",
      "ForwardOffMTrackerRecHits",

      "ForwardRomanPotRecParticles",
      "ForwardRomanPotStaticRecParticles",
      "ForwardOffMRecParticles",

      "ForwardRomanPotHits",
      "ForwardRomanPotRawHits",
      "ForwardRomanPotRawHitLinks",
      "ForwardRomanPotRawHitAssociations",
      "ForwardOffMTrackerHits",
      "ForwardOffMTrackerRawHits",
      "ForwardOffMTrackerRawHitLinks",
      "ForwardOffMTrackerRawHitAssociations",

      // Reconstructed data
      "GeneratedParticles",
      "GeneratedBreitFrameParticles",
      "ReconstructedParticles",
      "ReconstructedParticleLinks",
      "ReconstructedParticleAssociations",
      "ReconstructedTruthSeededChargedParticles",
      "ReconstructedTruthSeededChargedParticleLinks",
      "ReconstructedTruthSeededChargedParticleAssociations",
      "ReconstructedChargedRealPIDParticles",
      "ReconstructedChargedRealPIDParticleIDs",
      "ReconstructedChargedParticles",
      "ReconstructedChargedParticleLinks",
      "ReconstructedChargedParticleAssociations",
      "MCScatteredElectronAssociations",    // Remove if/when used internally
      "MCNonScatteredElectronAssociations", // Remove if/when used internally
      "ReconstructedBreitFrameParticles",

      "ReconstructedNeutralParticles",
      "ReconstructedNeutralParticleLinks",
      "ReconstructedNeutralParticleAssociations",
      "ReconstructedNeutralJets",

      // Central tracking
      "CentralTrackSegments",
      "CentralTrackVertices",
      "CentralCKFTruthSeededTrajectories",
      "CentralCKFTruthSeededTracks",
      "CentralCKFTruthSeededTrackLinks",
      "CentralCKFTruthSeededTrackAssociations",
      "CentralCKFTruthSeededTrackParameters",
      "CentralCKFTrajectories",
      "CentralCKFTracks",
      "CentralCKFTrackLinks",
      "CentralCKFTrackAssociations",
      "CentralCKFTrackParameters",
      // tracking properties - true seeding
      "CentralCKFTruthSeededTrajectoriesUnfiltered",
      "CentralCKFTruthSeededTracksUnfiltered",
      "CentralCKFTruthSeededTrackUnfilteredLinks",
      "CentralCKFTruthSeededTrackUnfilteredAssociations",
      "CentralCKFTruthSeededTrackParametersUnfiltered",
      // tracking properties - realistic seeding
      "CentralCKFTrajectoriesUnfiltered",
      "CentralCKFTracksUnfiltered",
      "CentralCKFTrackUnfilteredLinks",
      "CentralCKFTrackUnfilteredAssociations",
      "CentralCKFTrackParametersUnfiltered",

      // B0 tracking
      "B0TrackerCKFTruthSeededTrajectories",
      "B0TrackerCKFTruthSeededTracks",
      "B0TrackerCKFTruthSeededTrackLinks",
      "B0TrackerCKFTruthSeededTrackAssociations",
      "B0TrackerCKFTruthSeededTrackParameters",
      "B0TrackerCKFTrajectories",
      "B0TrackerCKFTracks",
      "B0TrackerCKFTrackLinks",
      "B0TrackerCKFTrackAssociations",
      "B0TrackerCKFTrackParameters",
      // tracking properties - true seeding
      "B0TrackerCKFTruthSeededTrajectoriesUnfiltered",
      "B0TrackerCKFTruthSeededTracksUnfiltered",
      "B0TrackerCKFTruthSeededTrackUnfilteredLinks",
      "B0TrackerCKFTruthSeededTrackUnfilteredAssociations",
      "B0TrackerCKFTruthSeededTrackParametersUnfiltered",
      // tracking properties - realistic seeding
      "B0TrackerCKFTrajectoriesUnfiltered",
      "B0TrackerCKFTrackParametersUnfiltered",
      "B0TrackerCKFTracksUnfiltered",
      "B0TrackerCKFTrackUnfilteredLinks",
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
      "PrimaryVerticesAMVF",
      "SecondaryVerticesAMVF",
      "BarrelClusters",
      "HadronicFinalState",

      // Track projections
      "CalorimeterTrackProjections",

      // Ecal stuff
      "EcalEndcapNRawHits",
      "EcalEndcapNRecHits",
      "EcalEndcapNTruthClusters",
      "EcalEndcapNTruthClusterLinks",
      "EcalEndcapNTruthClusterAssociations",
      "EcalEndcapNClusters",
      "EcalEndcapNClusterLinks",
      "EcalEndcapNClusterAssociations",
      "EcalEndcapNSplitMergeClusters",
      "EcalEndcapNSplitMergeClusterLinks",
      "EcalEndcapNSplitMergeClusterAssociations",
      "EcalEndcapPRawHits",
      "EcalEndcapPRecHits",
      "EcalEndcapPTruthClusters",
      "EcalEndcapPTruthClusterLinks",
      "EcalEndcapPTruthClusterAssociations",
      "EcalEndcapPClusters",
      "EcalEndcapPClusterLinks",
      "EcalEndcapPClusterAssociations",
      "EcalEndcapPSplitMergeClusters",
      "EcalEndcapPSplitMergeClusterLinks",
      "EcalEndcapPSplitMergeClusterAssociations",
      "EcalBarrelClusters",
      "EcalBarrelClusterLinks",
      "EcalBarrelClusterAssociations",
      "EcalBarrelTruthClusters",
      "EcalBarrelTruthClusterLinks",
      "EcalBarrelTruthClusterAssociations",
      "EcalBarrelImagingProcessedHits",
      "EcalBarrelImagingProcessedHitContributions",
      "EcalBarrelImagingRawHits",
      "EcalBarrelImagingRawHitLinks",
      "EcalBarrelImagingRawHitAssociations",
      "EcalBarrelImagingRecHits",
      "EcalBarrelImagingClusters",
      "EcalBarrelImagingClusterLinks",
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
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 7)
      "EcalBarrelScFiPCALOROCHits",
      "EcalBarrelScFiNCALOROCHits",
#endif
      "EcalBarrelScFiRecHits",
      "EcalBarrelScFiClusters",
      "EcalBarrelScFiClusterLinks",
      "EcalBarrelScFiClusterAssociations",
      "EcalLumiSpecRawHits",
      "EcalLumiSpecRecHits",
      "EcalLumiSpecTruthClusters",
      "EcalLumiSpecTruthClusterLinks",
      "EcalLumiSpecTruthClusterAssociations",
      "EcalLumiSpecClusters",
      "EcalLumiSpecClusterLinks",
      "EcalLumiSpecClusterAssociations",
      "HcalEndcapNRawHits",
      "HcalEndcapNRecHits",
      "HcalEndcapNMergedHits",
      "HcalEndcapNClusters",
      "HcalEndcapNClusterLinks",
      "HcalEndcapNClusterAssociations",
      "HcalEndcapNSplitMergeClusters",
      "HcalEndcapNSplitMergeClusterLinks",
      "HcalEndcapNSplitMergeClusterAssociations",
      "HcalEndcapPInsertRawHits",
      "HcalEndcapPInsertRecHits",
      "HcalEndcapPInsertMergedHits",
      "HcalEndcapPInsertClusters",
      "HcalEndcapPInsertClusterLinks",
      "HcalEndcapPInsertClusterAssociations",
      "LFHCALRawHits",
      "LFHCALRecHits",
      "LFHCALClusters",
      "LFHCALClusterLinks",
      "LFHCALClusterAssociations",
      "LFHCALSplitMergeClusters",
      "LFHCALSplitMergeClusterLinks",
      "LFHCALSplitMergeClusterAssociations",
      "HcalBarrelRawHits",
      "HcalBarrelRecHits",
      "HcalBarrelMergedHits",
      "HcalBarrelClusters",
      "HcalBarrelClusterLinks",
      "HcalBarrelClusterAssociations",
      "HcalBarrelSplitMergeClusters",
      "HcalBarrelSplitMergeClusterLinks",
      "HcalBarrelSplitMergeClusterAssociations",
      "B0ECalRawHits",
      "B0ECalRecHits",
      "B0ECalClusters",
      "B0ECalClusterLinks",
      "B0ECalClusterAssociations",
      "HcalEndcapNTruthClusters",
      "HcalEndcapNTruthClusterLinks",
      "HcalEndcapNTruthClusterAssociations",
      "HcalBarrelTruthClusters",
      "HcalBarrelTruthClusterLinks",
      "HcalBarrelTruthClusterAssociations",

      //ZDC Ecal
      "EcalFarForwardZDCRawHits",
      "EcalFarForwardZDCRecHits",
      "EcalFarForwardZDCClusters",
      "EcalFarForwardZDCClusterLinks",
      "EcalFarForwardZDCClusterAssociations",
      "EcalFarForwardZDCTruthClusters",
      "EcalFarForwardZDCTruthClusterLinks",
      "EcalFarForwardZDCTruthClusterAssociations",

      //ZDC HCal
      "HcalFarForwardZDCRawHits",
      "HcalFarForwardZDCRecHits",
      "HcalFarForwardZDCSubcellHits",
      "HcalFarForwardZDCClusters",
      "HcalFarForwardZDCClusterLinks",
      "HcalFarForwardZDCClusterAssociations",
      "HcalFarForwardZDCClustersBaseline",
      "HcalFarForwardZDCClusterLinksBaseline",
      "HcalFarForwardZDCClusterAssociationsBaseline",
      "HcalFarForwardZDCTruthClusters",
      "HcalFarForwardZDCTruthClusterLinks",
      "HcalFarForwardZDCTruthClusterAssociations",
      "ReconstructedHcalFarForwardZDCNeutrals",
      "ReconstructedB0EcalNeutrals",
      "ReconstructedEcalEndcapPNeutrals",
      "ReconstructedLFHCALNeutrals",
      "ReconstructedLambdas",
      "ReconstructedLambdaDecayProductsCM",

      // DIRC
      "DIRCRawHits",
      "DIRCTruthSeededParticleIDs",
      "DIRCParticleIDs",

      "EcalEndcapPTrackClusterMatches",
      "LFHCALTrackClusterMatches",
      "HcalEndcapPInsertClusterMatches",
      "EcalBarrelTrackClusterMatches",
      "HcalBarrelTrackClusterMatches",
      "EcalEndcapNTrackClusterMatches",
      "HcalEndcapNTrackClusterMatches",

      // particle flow
      "EcalBarrelRemnantClusters",
      "EcalBarrelExpectedClusters",
      "EcalBarrelTrackExpectedClusterLinks",
      "EcalBarrelTrackExpectedClusterMatches",
      "EcalEndcapNRemnantClusters",
      "EcalEndcapNExpectedClusters",
      "EcalEndcapNTrackExpectedClusterLinks",
      "EcalEndcapNTrackExpectedClusterMatches",
      "EcalEndcapPRemnantClusters",
      "EcalEndcapPExpectedClusters",
      "EcalEndcapPTrackExpectedClusterLinks",
      "EcalEndcapPTrackExpectedClusterMatches",
      "HcalBarrelRemnantClusters",
      "HcalBarrelExpectedClusters",
      "HcalBarrelTrackExpectedClusterLinks",
      "HcalBarrelTrackExpectedClusterMatches",
      "HcalEndcapNRemnantClusters",
      "HcalEndcapNExpectedClusters",
      "HcalEndcapNTrackExpectedClusterLinks",
      "HcalEndcapNTrackExpectedClusterMatches",
      "LFHCALRemnantClusters",
      "LFHCALExpectedClusters",
      "LFHCALTrackExpectedClusterLinks",
      "LFHCALTrackExpectedClusterMatches",
      "HcalEndcapPInsertRemnantClusters",
      "HcalEndcapPInsertExpectedClusters",
      "HcalEndcapPInsertTrackExpectedClusterLinks",
      "HcalEndcapPInsertTrackExpectedClusterMatches",
      "EcalEndcapNTrackSplitMergeClusterMatches",
      "HcalEndcapNTrackSplitMergeClusterMatches",
      "HcalBarrelTrackSplitMergeClusterMatches",
      "EcalEndcapPTrackSplitMergeClusterMatches",
      "LFHCALTrackSplitMergeClusterMatches",
      "EndcapNChargedCandidateParticlesAlpha",
      "BarrelChargedCandidateParticlesAlpha",
      "EndcapPChargedCandidateParticlesAlpha",
      "EndcapPInsertChargedCandidateParticlesAlpha",
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
  japp->SetDefaultParameter(
      "podio:output_backend", m_output_backend,
      "Output backend: 'root' for TTree (default) or 'rntuple' for RNTuple format");

  m_output_collections =
      std::set<std::string>(output_collections.begin(), output_collections.end());
  m_output_exclude_collections =
      std::set<std::string>(output_exclude_collections.begin(), output_exclude_collections.end());
}

void JEventProcessorPODIO::Init() {

  auto* app = GetApplication();
  m_log     = app->GetService<Log_service>()->logger("JEventProcessorPODIO");

  // Convert backend selection to lowercase for case-insensitive comparison
  std::string backend_lower = m_output_backend;
  std::transform(backend_lower.begin(), backend_lower.end(), backend_lower.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  m_log->info("Using '{}' backend for output file: {}", backend_lower, m_output_file);

  // Create writer using podio::makeWriter
  try {
    m_writer = std::make_unique<podio::Writer>(podio::makeWriter(m_output_file, backend_lower));
  } catch (const std::exception& e) {
    throw std::runtime_error(
        fmt::format("Failed to create writer with backend '{}': {}", backend_lower, e.what()));
  }
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
          m_log->debug("Persisting collection '{}'", col);
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

void JEventProcessorPODIO::PropagateNonEventCategories() {
  // Propagate all non-event frames from input to output
  auto* app                 = GetApplication();
  auto component_manager    = app->GetService<JComponentManager>();
  const auto& event_sources = eicrecon::jana_compat::GetEventSources(component_manager);
  for (auto* source : event_sources) {
    auto* podio_source = dynamic_cast<JEventSourcePODIO*>(source);
    if (podio_source == nullptr)
      continue;
    for (const auto& _category : podio_source->getAvailableCategories()) {
      std::string category{_category};
      if (category == "events")
        continue;
      std::size_t n = podio_source->getEntries(category);
      for (std::size_t i = 0; i < n; ++i) {
        m_writer->writeFrame(podio_source->getFrame(category, i), category);
      }
      m_log->info("Propagated {} '{}' frame(s) to output file", n, category);
    }
  }
}

void JEventProcessorPODIO::Finish() {
  PropagateNonEventCategories();
  m_writer->finish();
}
