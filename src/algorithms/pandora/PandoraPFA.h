// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <Pandora/Pandora.h>
#include <string>
#include <string_view>

#include "PandoraPFAConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

/// Input: (ecalBarrel, ecalEndcap, hcalBarrel, hcalEndcap calo hits) + track projections
/// Output: reconstructed particles from PandoraPFA
using PandoraPFAAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::CalorimeterHitCollection, // ECAL barrel
                                            edm4eic::CalorimeterHitCollection, // ECAL endcap
                                            edm4eic::CalorimeterHitCollection, // HCAL barrel
                                            edm4eic::CalorimeterHitCollection, // HCAL endcap
                                            edm4eic::TrackSegmentCollection>,  // track projections
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

// ============================================================================
// PandoraPFA particle-flow algorithm wrapper
// ============================================================================
/*! A JANA2-compatible algorithm wrapper around the PandoraSDK particle-flow
 *  framework.  One pandora::Pandora instance is owned per algorithm instance
 *  (i.e. per JANA thread); geometry is registered once during init().
 *
 *  For each event, calorimeter hits and track projections are fed into
 *  Pandora via PandoraInputMapper, PandoraApi::ProcessEvent() is called,
 *  and the resulting PFOs are extracted by PandoraOutputMapper.
 */
class PandoraPFA : public PandoraPFAAlgorithm, public WithPodConfig<PandoraPFAConfig> {
public:
  PandoraPFA(std::string_view name)
      : PandoraPFAAlgorithm{name,
                            {"inputECalBarrelHits", "inputECalEndcapHits", "inputHCalBarrelHits",
                             "inputHCalEndcapHits", "inputTrackProjections"},
                            {"outputParticles"},
                            "PandoraPFA particle-flow algorithm"} {}

  /// Execute the algorithm for a single event.
  void process(const Input&, const Output&) const override;

private:
  /// Lazy initialization of Pandora instance (called once on first process)
  void initializePandora() const;

  /// Log which parameter overrides are active
  void logParameterOverrides() const;

  /// The owned Pandora instance (one per algorithm instance / thread).
  mutable std::unique_ptr<pandora::Pandora> m_pandora;

  /// Reference to the DD4hep detector (obtained from GeoSvc).
  mutable const dd4hep::Detector* m_detector{nullptr};

  /// Initialization flag
  mutable std::once_flag m_initOnce;
}; // end PandoraPFA

} // namespace eicrecon
