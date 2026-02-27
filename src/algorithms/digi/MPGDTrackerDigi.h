// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov, Yann Bedfer

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/Segmentations.h>
#include <Parsers/Primitives.h>
#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "MPGDTrackerDigiConfig.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#endif

namespace eicrecon {

using MPGDTrackerDigiAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::EventHeaderCollection, edm4hep::SimTrackerHitCollection>,
    algorithms::Output<edm4eic::RawTrackerHitCollection,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                       edm4eic::MCRecoTrackerHitLinkCollection,
#endif
                       edm4eic::MCRecoTrackerHitAssociationCollection>>;

class MPGDTrackerDigi : public MPGDTrackerDigiAlgorithm,
                        public WithPodConfig<MPGDTrackerDigiConfig> {

public:
  MPGDTrackerDigi(std::string_view name)
      : MPGDTrackerDigiAlgorithm{
            name,
            {"eventHeaderCollection", "inputHitCollection"},
            {"outputRawHitCollection",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
             "outputHitLinks",
#endif
             "outputHitAssociations"},
            "2D-strip segmentation, apply threshold, digitize within ADC range, "
            "convert time with smearing resolution."} {
  }

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::UniqueIDGenSvc& m_uid = algorithms::UniqueIDGenSvc::instance();

  // IDDESCRIPTOR and SEGMENTATION
  void parseIDDescriptor();
  void parseSegmentation();

  // COALESCE and EXTEND
  bool cCoalesceExtend(const Input& input, int& idx, std::vector<std::uint64_t>& cIDs, double* lpos,
                       double& eDep, double& time) const;
  bool bCoalesceExtend(const Input& input, int& idx, std::vector<std::uint64_t>& cIDs, double* lpos,
                       double& eDep, double& time) const;
  unsigned int cTraversing(const double* lpos, const double* lmom, double path,
                           bool isSecondary,         // Input subHit
                           double rMin, double rMax, // Current instance of SUBVOLUME
                           double dZ, double startPhi, double endPhi, // Module parameters
                           double lintos[][3], double louts[][3], double* lpini,
                           double* lpend) const;
  unsigned int bTraversing(const double* lpos, const double* lmom, double ref2Cur, double path,
                           bool isSecondary,     // Input subHit
                           double dZ,            // Current instance of SUBVOLUME
                           double dX, double dY, // Module parameters
                           double lintos[][3], double louts[][3], double* lpini,
                           double* lpend) const;
  void printSubHitList(const Input& input, std::vector<int>& subHitList) const;
  unsigned int extendHit(dd4hep::CellID modID, std::vector<std::uint64_t>& cIDs, int direction,
                         double* lpini, double* lmini, double* lpend, double* lmend) const;
  unsigned int cExtension(double const* lpos, double const* lmom, // Input subHit
                          double rT,                              // Target radius
                          int direction, double dZ, double startPhi,
                          double endPhi, // Module parameters
                          double* lext) const;
  unsigned int bExtension(const double* lpos, const double* lmom, // Input subHit
                          double zT,                              // Target Z
                          int direction, double dX, double dY,    // Module parameters
                          double* lext) const;
  bool samePMO(const edm4hep::SimTrackerHit&, const edm4hep::SimTrackerHit&) const;
  bool denyExtension(const edm4hep::SimTrackerHit& sim_hit, double depth) const;
  void flagUnexpected(const edm4hep::EventHeader& event, int shape, double expected,
                      const edm4hep::SimTrackerHit& sim_hit, double* lpini, double* lpend,
                      double* lpos, double* lmom) const;
  std::function<int(double)> m_toleranceFactor;

  /** Segmentation */
  const dd4hep::Detector* m_detector{nullptr};
  dd4hep::Segmentation m_seg;
  // IDDescriptor
  const dd4hep::BitFieldCoder* m_id_dec{nullptr};
  static constexpr const char* m_fieldNames[5] = // "volume": excluding channel specification
      {"system", "layer", "module", "sensor", "strip"};
  // CellIDs specifying IDDescriptor fields.
  dd4hep::CellID m_volumeBits{0}; // "volume" bits, as opposed to channel# bits
  dd4hep::CellID m_moduleBits{0}; // "volume" cleared of its "strip" bits.
  // Strip (standing here for "SUBVOLUME") related fields.
  dd4hep::CellID m_stripBits{0}; // "strip" field
  dd4hep::CellID m_pStripBit{0}; // 'p' strip
  dd4hep::CellID m_nStripBit{0}; // 'n' strip
  dd4hep::CellID m_stripIDs[5];  // Ordered from inner to outer
  /** Ordering of Multiple Sensitive Volumes */
  std::function<int(dd4hep::CellID)> m_stripRank;
  std::function<int(dd4hep::CellID, dd4hep::CellID)> m_orientation;
  std::function<bool(int, unsigned int)> m_isUpstream;
  std::function<bool(int, unsigned int)> m_isDownstream;

  /** Status code */
  static constexpr unsigned int m_intoLower     = 0x1;
  static constexpr unsigned int m_outLower      = 0x2;
  static constexpr unsigned int m_intoUpper     = 0x4;
  static constexpr unsigned int m_outUpper      = 0x8;
  static constexpr unsigned int m_canReEnter    = 0x100;
  static constexpr unsigned int m_inconsistency = 0xff000;
};

} // namespace eicrecon
