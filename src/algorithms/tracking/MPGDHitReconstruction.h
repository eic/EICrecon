// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <DDRec/CellIDPositionConverter.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <gsl/pointers>
#include <string>
#include <string_view>

#include "MPGDHitReconstructionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using MPGDHitReconstructionAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::RawTrackerHitCollection>,
                          algorithms::Output<edm4eic::TrackerHitCollection>>;

/**
 * Produces edm4eic::TrackerHit with geometric info from edm4eic::RawTrackerHit
 Specific to MPGDs.
*/
class MPGDHitReconstruction : public MPGDHitReconstructionAlgorithm,
			      public WithPodConfig<MPGDHitReconstructionConfig> {

public:
  MPGDHitReconstruction(std::string_view name)
      : MPGDHitReconstructionAlgorithm{
           name, {"inputRawHits"}, {"outputHits"}, "reconstruct raw hits into tracker hits."} {}

  /// Once in a lifetime initialization
  void init() final;

  /// Processes RawTrackerHit and produces a TrackerHit
  void process(const Input&, const Output&) const final;

private:
  /** Segmentation */
  const algorithms::GeoSvc& m_geo{algorithms::GeoSvc::instance()};
  const dd4hep::rec::CellIDPositionConverter* m_converter{m_geo.cellIDPositionConverter()};
  const dd4hep::BitFieldCoder* m_id_dec;
  // CellIDs specifying IDDescriptor fields.
  void parseIDDescriptor();
  dd4hep::CellID m_pStripBit{0}; // 'p' strip
  dd4hep::CellID m_nStripBit{0}; // 'n' strip
  dd4hep::CellID m_stripIncs[2]; // Increments
};

} // namespace eicrecon
