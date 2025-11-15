// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov, Yann Bedfer

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/Segmentations.h>
#include <Parsers/Primitives.h>
#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
//#define MC_PARTICLE_ASSOCIATION
#ifdef MC_PARTICLE_ASSOCIATION
#include <edm4hep/MCParticle.h>
#endif
#include <functional>
#include <string>
#include <string_view>

#include "MPGDTrackerDigiConfig.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using MPGDTrackerDigiAlgorithm = algorithms::Algorithm<
#ifdef MC_PARTICLE_ASSOCIATION
    algorithms::Input<edm4hep::EventHeaderCollection, edm4hep::SimTrackerHitCollection,
                      podio::CollectionBase>,
#else
    algorithms::Input<edm4hep::EventHeaderCollection, edm4hep::SimTrackerHitCollection>,
#endif
    algorithms::Output<edm4eic::RawTrackerHitCollection,
                       edm4eic::MCRecoTrackerHitAssociationCollection>>;

class MPGDTrackerDigi : public MPGDTrackerDigiAlgorithm,
                        public WithPodConfig<MPGDTrackerDigiConfig> {

public:
  MPGDTrackerDigi(std::string_view name)
      : MPGDTrackerDigiAlgorithm{
            name,
            {"eventHeaderCollection", "inputHitCollection"},
            {"outputRawHitCollection", "outputHitAssociations"},
            "2D-strip segmentation, apply threshold, digitize within ADC range, "
            "convert time with smearing resolution."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::UniqueIDGenSvc& m_uid = algorithms::UniqueIDGenSvc::instance();

  // Member methods for checking the consistency of subHits to be accumulated
  unsigned int extendHit(dd4hep::CellID modID, int direction, double* lpini, double* lmini,
                         double* lpend, double* lmend) const;
  unsigned int cExtension(double const* lpos, double const* lmom, // Input subHit
                          double rT,                              // Target radius
                          int direction, double dZ, double startPhi,
                          double endPhi, // Module parameters
                          double* lext) const;
  unsigned int bExtension(const double* lpos, const double* lmom, // Input subHit
                          double zT,                              // Target Z
                          int direction, double dX, double dY,    // Module parameters
                          double* lext) const;
  bool samePMO(const edm4hep::SimTrackerHit&, const edm4hep::SimTrackerHit&, int unbroken) const;
  bool denyExtension(const edm4hep::SimTrackerHit& sim_hit, double depth) const;
  void flagUnexpected(const edm4hep::EventHeader& event, int shape, double expected,
                      const edm4hep::SimTrackerHit& sim_hit, double* lpini, double* lpend,
                      double* lpos, double* lmom) const;

  /** Segmentation */
  const dd4hep::Detector* m_detector{nullptr};
  dd4hep::Segmentation m_seg;
  // Built-in constants specifying IDDescriptor fields.
  // The "init" should method double-check these are actually implemented in the XMLs of MPGDs.
  static constexpr dd4hep::CellID m_volumeBits  = 0xffffffff; // 32 least weight bits
  static constexpr dd4hep::CellID m_stripBits   = ((dd4hep::CellID)0xf) << 28;
  static constexpr dd4hep::CellID m_stripMask   = ~m_stripBits;
  static constexpr dd4hep::CellID m_moduleBits  = m_volumeBits & m_stripMask;
  static constexpr dd4hep::CellID m_pStripBit   = ((dd4hep::CellID)0x1) << 28;
  static constexpr dd4hep::CellID m_nStripBit   = ((dd4hep::CellID)0x2) << 28;
  static constexpr dd4hep::CellID m_stripIDs[5] = {((dd4hep::CellID)0x3) << 28, m_pStripBit, 0,
                                                   m_nStripBit, ((dd4hep::CellID)0x4) << 28};
  /** Ordering of Multiple Sensitive Volumes */
  std::function<int(dd4hep::CellID)> m_stripRank;
  std::function<int(dd4hep::CellID, dd4hep::CellID)> m_orientation;
  std::function<bool(int, unsigned int)> m_isUpstream;
  std::function<bool(int, unsigned int)> m_isDownstream;
};

} // namespace eicrecon
