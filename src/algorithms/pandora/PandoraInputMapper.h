// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#pragma once

#include <Api/PandoraApi.h>
#include <Pandora/Pandora.h>
#include <Pandora/PandoraEnumeratedTypes.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>

namespace eicrecon {

/// Technology classification for calorimeter readout systems.
/// Used to apply technology-specific discrimination and granularity hints.
enum class CaloTechnology {
  DEFAULT,            ///< Backward compatibility, no special handling
  IMAGING_SI,         ///< Silicon imaging calorimeter → ECAL type
  SCINTILLATING_FIBER ///< ScFi calorimeter → DRC_SCINT type
};

/// Translates edm4eic calorimeter hits and track projections into PandoraSDK input objects.
///
/// Call addCaloHits() for each calorimeter collection before calling
/// PandoraApi::ProcessEvent(), then call addTracks() for the track projections.
class PandoraInputMapper {
public:
  /// Feed calorimeter hits from an edm4eic collection into @p pandora.
  ///
  /// @param pandora       The Pandora instance to receive the hits.
  /// @param hits          Collection of reconstructed calorimeter hits.
  /// @param hitType       Pandora hit type (ECAL or HCAL).
  /// @param hitRegion     Pandora hit region (BARREL or ENDCAP).
  /// @param mipEquivScale Conversion factor: energy [GeV] → MIP-equivalent energy [MIP].
  /// @param technology    Readout technology classification (default: DEFAULT for backward compatibility).
  static void addCaloHits(pandora::Pandora& pandora, const edm4eic::CalorimeterHitCollection& hits,
                          pandora::HitType hitType, pandora::HitRegion hitRegion,
                          float mipEquivScale, CaloTechnology technology = CaloTechnology::DEFAULT);

  /// Feed track projections from an edm4eic TrackSegment collection into @p pandora.
  ///
  /// Each TrackSegment is expected to carry at least two TrackPoint entries:
  ///   - index 0: point of closest approach (DCA)
  ///   - last index: projection to calorimeter face
  ///
  /// @param pandora    The Pandora instance to receive the tracks.
  /// @param segments   Collection of track projections at the calorimeter surface.
  static void addTracks(pandora::Pandora& pandora, const edm4eic::TrackSegmentCollection& segments);
}; // end PandoraInputMapper

} // namespace eicrecon
