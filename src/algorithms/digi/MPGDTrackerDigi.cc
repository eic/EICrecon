// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov, Yann Bedfer

/*
  Digitization specific to MPGDs.
  - What's special in MPGDs is their 2D-strip readout: i.e. simultaneous
   registering along two sets of coordinate strips.
  - The fact imposes strong constraints on digitization, stemming from DD4hep's
   provision that one and only one readout surface be associated to a sensitive
   volume. The one-and-only-one rule is enforced through to ACTS's TrackState,
   forbidding simple solutions exploiting MultiSegmentation to create two strip
   hits on a same surface.
  - Present solution is based on multiple sensitive SUBVOLUMES. FIVE of them:
    + TWO persistent ones, acting as READOUT SURFACES (i.e. to which raw hits
     are assigned, in both RealData and MC): very thin and sitting very close
     to mid-plane,
    + THREE HELPER ones: TWO RADIATORS, thick, serving temporarily to collect
     energy deposit and ONE REFERENCE, thin and sitting exactly at mid-plane.
     (The REFERENCE is not essential, but found to help rationalize the source
     code.)
  - Each SUBVOLUME has a distinct ID, w/ a distinctive "STRIP" FIELD, but all
   share a common XML <readout> thanks to a MultiSegmentation based on the
   "STRIP" FIELD.
  - Several subHits are created (up to five, one per individual SUBVOLUME)
   corresponding to a SINGLE I[ONISATION]+A[MPLIFICATION] process, inducing in
   turn CORRELATED SIGNALS ON TWO COORDINATES.
  - These subHits are extrapolated to the REFERENCE SUBVOLUME (its mid-plane),
   so as to reconstitute the proper hit position of the TRAVERSING particle.
   This, for particles that are determined to be indeed traversing.
  - We then want to preserve the I+A correlation when accumulating hits on the
   readout channels. This cannot be obtained by accumulating directly subHits (
   as is done in "SiliconTrackerDigi", as of commit #151496af). We have to
   first COALESCE separately subHits into hits corresponding to a given I+A.
   Then apply a common smearing simulating I+A, then apply independent
   smearings simulating CHARGE SHARING and CHARGE SPREADING. Then accumulate
   channel by channel. Then, possibly, apply smearing simulating FE electronics.
  - Therefore we have a DOUBLE LOOP ON INPUT SimTrackerHits, to collect those
   subHits deemed to arise from the same I+A. I.e. SAME P[ARTICLE], M[ODULE] (a
   particle can fire two overlapping modules, not to be mixed) and O[RIGIN] (
   direct or via secondary).
  - The double loop is optimized for speed (by keeping track of the start index
   of the second loop) in order to not waste time on high multiplicity events.
  - A priori, subHits with same PMO should come in sequence. We nevertheless
   provide for them to be intertwined. This, except for subHits of secondary
   origin.
  - A number of checks are conducted before undertaking subHit COALESCENCE:
    I) SubHits should be TRAVERSING, i.e. exiting/entering through opposite
     walls, as opposed to exiting/entering through the edge or dying/being-born
     within their SUBVOLUME, see methods "(c|b)Traversing".
    II) SubHits have SAME PMO, see "samePMO".
    III) SubHits extrapolate to a common point, see "outInDistance".
    All these checks are done to be on the safe side. A subset of them,
   hinging on (III), should be sufficient. But there are so many cases to take
   into account that it's difficult to settle on a minimal subset.
  - Evaluation:
    + With 3 mm overall sensitive volume (=> ~1.5 mm SUBVOLUME), MIPs turn out
     firing most of the time only one SUBVOLUME.
    + MIPs are found to be properly handled: hits are assigned to mid-plane,
     are COALESCED when needed...
     ...Except for few cases, see "flagUnexpected".
    + For lower energy particles, there are at times ambiguous cases where
     seemingly related subHits turn out to not be COALESCED, because they fail
     to pass above-mentioned checks due to their erratic trajectories.
      Note that even if they are genuinely related, this is not dramatic:
     subHits will still be directly accumulated on their common (within pitch
     precision) cell, only that we miss the correlation.
    + The special case of a particle exiting and reEntering the same SUBVOLUME,
     in a cylindrical setup, is catered for, but has not been evaluated.
  - Imperfections:
    + In order to validate (III), one has to fix a tolerance. The ideal would
     be to fix it based on multiscattering. This not done. Instead a, somewhat
     arbitrary, built-in value is used.
  - Future developments:
   In addition to DIGITIZATION proper, the method involves SIMULATION and
   (re)SEGMENTATION.
    + SIMULATION will eventually involve simulating the amplitude and timing
     correlation of the two coordinates, and the spreading of the charge on
     adjacent strips producing multi-hit clusters. But present version is
     preliminary: single-hit clusters, with identical timing and same
     amplitude.
    + DIGITIZATION follows the standard steps. Remains to agree on the handling
     of the discrimination threshold, see Issue #1722.
 */

#include "MPGDTrackerDigi.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Handle.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DD4hep/VolumeManager.h>
#include <DD4hep/detail/SegmentationsInterna.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JException.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <Parsers/Primitives.h>
// Access "algorithms:GeoSvc"
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <edm4hep/EDM4hepVersion.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4eic/unit_system.h>
#include <fmt/core.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <gsl/pointers>
#include <initializer_list>
#include <iterator>
#include <random>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/digi/MPGDTrackerDigiConfig.h"

#include "DDRec/SurfaceManager.h"
#include "DDRec/SurfaceHelper.h"
#include "DDRec/Surface.h"

using namespace dd4hep;

namespace eicrecon {

void MPGDTrackerDigi::init() {
  // Access id decoder
  m_detector = algorithms::GeoSvc::instance().detector();

  const dd4hep::BitFieldCoder* m_id_dec = nullptr;
  if (m_cfg.readout.empty()) {
    throw JException("Readout is empty");
  }
  try {
    m_seg    = m_detector->readout(m_cfg.readout).segmentation();
    m_id_dec = m_detector->readout(m_cfg.readout).idSpec().decoder();
  } catch (...) {
    critical("Failed to load ID decoder for \"{}\" readout.", m_cfg.readout);
    throw JException("Failed to load ID decoder");
  }
  // Method "process" relies on an assumption on the IDDescriptor's strip field.
  // Let's check.
  debug(R"(Find valid "strip" field in IDDescriptor for "{}" readout.)", m_cfg.readout);
  if (m_id_dec->get(m_stripBits, "strip") != 0xf) {
    critical(R"(Missing or invalid "strip" field in IDDescriptor for "{}" readout.)",
             m_cfg.readout);
    throw JException("Invalid IDDescriptor");
  }

  // Ordering of SUBVOLUMES (based on "STRIP" FIELD)
  m_stripRank = [=, this](CellID vID) {
    int rank;
    CellID sID = vID & m_stripBits;
    for (rank = 0; rank < 5; rank++)
      if (sID == m_stripIDs[rank])
        return rank;
    return -1;
  };
  m_orientation = [=, this](CellID vID, CellID vJD) {
    int ranki = m_stripRank(vID), rankj = m_stripRank(vJD);
    if (rankj > ranki)
      return +1;
    else if (rankj < ranki)
      return -1;
    else
      return 0;
  };
  m_isUpstream = [](int orientation, unsigned int status) {
    // Outgoing particle exits...
    bool isUpstream =
        (orientation < 0 && (status & 0x2)) ||           // ...lower wall
        (orientation > 0 && (status & 0x8)) ||           // ...upper wall
        (orientation == 0 && (status & 0x102) == 0x102); // ...lower wall and can reEnter
    return isUpstream;
  };
  m_isDownstream = [](int orientation, unsigned int status) {
    // Incoming particle enters...
    bool isDownstream =
        (orientation > 0 && (status & 0x1)) ||           // ...lower wall
        (orientation < 0 && (status & 0x4)) ||           // ...upper wall
        (orientation == 0 && (status & 0x101) == 0x101); // ...lower wall and can be reEntering
    return isDownstream;
  };
}

// Interfaces
void getLocalPosMom(const edm4hep::SimTrackerHit& sim_hit, const TGeoHMatrix& toModule,
                    double* lpos, double* lmom);
unsigned int cTraversing(const double* lpos, const double* lmom, double path,
                         bool isSecondary,                          // Input subHit
                         double rMin, double rMax,                  // Current instance of SUBVOLUME
                         double dZ, double startPhi, double endPhi, // Module parameters
                         double lintos[][3], double louts[][3], double* lpini, double* lpend);
bool cExtrapolate(const double* lpos, const double* lmom, // Input subHit
                  double rT,                              // Target radius
                  double* lext);                          // Extrapolated position @ <rT>
double getRef2Cur(DetElement refVol, DetElement curVol);
unsigned int bTraversing(const double* lpos, const double* lmom, double ref2Cur, double path,
                         bool isSecondary,     // Input subHit
                         double dZ,            // Current instance of SUBVOLUME
                         double dX, double dY, // Module parameters
                         double lintos[][3], double louts[][3], double* lpini, double* lpend);
bool bExtrapolate(const double* lpos, const double* lmom, // Input subHit
                  double zT,                              // Target Z
                  double* lext);                          // Extrapolated position @ <zT>
std::string inconsistency(const edm4hep::EventHeader& event, unsigned int status, CellID cID,
                          const double* lpos, const double* lmom);
std::string oddity(const edm4hep::EventHeader& event, unsigned int status, double dist, CellID cID,
                   const double* lpos, const double* lmom, CellID cJD, const double* lpoj,
                   const double* lmoj);
double outInDistance(int shape, int orientation, double lintos[][3], double louts[][3],
                     double* lmom, double* lmoj);
void flagUnexpected(const edm4hep::EventHeader& event, int shape, double expected,
                    const edm4hep::SimTrackerHit& sim_hit, double* lpini, double* lpend,
                    double* lpos, double* lmom);

void MPGDTrackerDigi::process(const MPGDTrackerDigi::Input& input,
                              const MPGDTrackerDigi::Output& output) const {

  const auto [headers, sim_hits] = input;
  auto [raw_hits, associations]  = output;

  // local random generator
  auto seed = m_uid.getUniqueID(*headers, name());
  std::default_random_engine generator(seed);
  std::normal_distribution<double> gaussian;

  // A map of unique cellIDs with temporary structure RawHit
  std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit> cell_hit_map;
  // A map of strip cellIDs with vector of contributing cellIDs
  std::map<std::uint64_t, std::vector<std::uint64_t>> stripID2cIDs;
  // Prepare for strip segmentation
  const Position dummy(0, 0, 0);
  const VolumeManager& volman = m_detector->volumeManager();

  // Reference to event, to be used to document "critical" error messages
  // (N.B.: I don't know how to properly handle these "headers": may there
  // be more than one? none?...)
  const edm4hep::EventHeader& header = headers->at(0);

  std::vector<int> usedHits;
  size_t sim_size = sim_hits->size();
  for (int idx = 0; idx < (int)sim_size; idx++) {
    const edm4hep::SimTrackerHit& sim_hit = sim_hits->at(idx);

    // ***** TIME SMEARING
    // - Simplistic treatment.
    // - A more realistic one would have to distinguish a smearing common to
    //  both coordinates of the 2D-strip readout (mainly due to the drifting of
    //  the leading primary electrons of the I+A process) from other smearing
    //  effects, specific to each coordinate.
    double time_smearing = gaussian(generator) * m_cfg.timeResolution;

    // ***** USED HIT?
    int usedHit = 0;
    for (int jdx : usedHits) {
      if (jdx == idx) {
        usedHit = 1;
        break;
      }
    }
    if (usedHit)
      continue;
    else
      usedHits.push_back(idx); // useful?...

    // ***** SEGMENTATION
    // - The two cellIDs are encoded via a "dd4hep::MultiSegmentation"
    //  discriminating on the strip field, w/ "strip" setting of 0x1 (
    //  called 'p') or 0x2 (called 'n') and 0x3/0x4 for the inner/outer
    //  RADIATORS, and 0x0 for the REFERENCE SUBVOLUME.
    // - They are evaluated based on "sim_hit" Cartesian position and momentum.
    //   + Extrapolating to REFERENCE SUBVOLUME.
    //   + COALESCING all subHits with SAME PMO.
    //   Given that all the segmentation classes foreseen for MPGDs (
    //  "CartesianGrid.." for Outer and EndCaps, "CylindricalGridPhiZ" for
    //  "CyMBaL") disregard the _global_ position argument to
    //  "dd4hep::Segmentation::cellID", we need the _local_ position and
    //  only that.
    CellID vID        = sim_hit.getCellID() & m_volumeBits;
    CellID refID      = vID & m_moduleBits; // => the middle slice
    DetElement refVol = volman.lookupDetElement(refID);
    // TGeoHMatrix: In order to avoid a "dangling-reference" warning,
    // let's take a copy of the matrix instead of a reference to it.
    const TGeoHMatrix toRefVol = refVol.nominal().worldTransformation();
    double lpos[3], lmom[3];
    getLocalPosMom(sim_hit, toRefVol, lpos, lmom);
    const double edmm = edm4eic::unit::mm, ed2dd = dd4hep::mm / edmm;
    using dd4hep::mm;
    using edm4eic::unit::eV, edm4eic::unit::GeV;
    // Hit in progress
    double eDep = sim_hit.getEDep(), time = sim_hit.getTime();
    // ***** COALESCE ALL MUTUALLY CONSISTENT SUBHITS
    //       EXTEND TRAVERSING SUBHITS
    // - Needed because we want to preserve the correlation between 'p' and
    //  'n' strip hits resulting from a given I+A process (which is lost when
    //  one accumulates hits based on cellID).
    std::vector<int> same_idcs;
    std::vector<std::uint64_t> cIDs;
    const auto& shape = refVol.solid();
    if (!strcmp(shape.type(), "TGeoTubeSeg")) {
      // ********** TUBE GEOMETRY
      const Tube& tRef = refVol.solid(); // REFERENCE SUBVOLUME
      double dZ        = tRef.dZ();
      // phi?
      // In "https://root.cern.ch/root/html534/guides/users-guide/Geometry.html"
      // TGeoTubeSeg: "phi1 is converted to [0,360] (but still expressed in
      // radian, as far as I can tell) and phi2 > phi1."
      // => Convert it to [-pi,+pi].
      double startPhi = tRef.startPhi() * radian;
      startPhi -= 2 * TMath::Pi();
      double endPhi = tRef.endPhi() * radian;
      endPhi -= 2 * TMath::Pi();
      // Get current SUBVOLUME
      DetElement curVol = volman.lookupDetElement(vID);
      const Tube& tCur  = curVol.solid();
      double rMin = tCur.rMin(), rMax = tCur.rMax();
      // Is TRAVERSING?
      double lintos[2][3], louts[2][3], lpini[3], lpend[3], lmend[3];
      std::copy(std::begin(lmom), std::end(lmom), std::begin(lmend));
      unsigned int status =
          cTraversing(lpos, lmom, sim_hit.getPathLength() * ed2dd, sim_hit.isProducedBySecondary(),
                      rMin, rMax, dZ, startPhi, endPhi, lintos, louts, lpini, lpend);
      if (status & 0xff000) { // Inconsistency => Drop current "sim_hit"
        critical(inconsistency(header, status, sim_hit.getCellID(), lpos, lmom));
        continue;
      }
      cIDs.push_back(sim_hit.getCellID());
      if (level() >= algorithms::LogLevel::kDebug) {
        same_idcs.push_back(idx);
      }
      // Continuations. Particle may exit and then re-enter through "rMin".
      bool isContinuation  = status & 0x5;
      bool hasContinuation = status & 0xa;
      bool canReEnter      = status & 0x100;
      int rank             = m_stripRank(vID);
      if (!canReEnter) {
        if (rank == 0 && (status & 0x1))
          isContinuation = false;
        if (rank == 0 && (status & 0x2))
          hasContinuation = false;
      }
      if (rank == 4 && (status & 0x4))
        isContinuation = false;
      if (rank == 4 && (status & 0x8))
        hasContinuation = false;
      if (hasContinuation) {
        // ***** LOOP OVER HITS
        int jdx, unbroken /* unbroken succession of indices */;
        CellID vIDPrv = vID;
        for (jdx = idx + 1, unbroken = 1; jdx < (int)sim_size; jdx++) {
          const edm4hep::SimTrackerHit& sim_hjt = sim_hits->at(jdx);
          // Used hit? If indeed, it's going to be discarded anyway in the
          // following: current "sim_hit" is by construction at variance to it.
          CellID vJD = sim_hjt.getCellID() & m_volumeBits;
          // Particle may start inward and re-enter, being then outward-going.
          // => Orientation has to be evaluated w.r.t. previous vID.
          int orientation = m_orientation(vIDPrv, vJD);
          bool isUpstream = m_isUpstream(orientation, status);
          bool pmoStatus  = samePMO(sim_hit, sim_hjt, unbroken);
          if (!pmoStatus || !isUpstream) {
            if ((pmoStatus && !isUpstream) && !sim_hit.isProducedBySecondary())
              // Bizarre: let's flag the case while debugging
              debug(inconsistency(header, 0, sim_hit.getCellID(), lpos, lmom));
            if (unbroken)
              idx = jdx - 1;
            unbroken = 0;
            continue;
          }
          // Get 'j' radii
          curVol           = volman.lookupDetElement(vJD);
          const Tube& tubj = curVol.solid();
          rMin             = tubj.rMin();
          rMax             = tubj.rMax();
          double lpoj[3], lmoj[3];
          getLocalPosMom(sim_hjt, toRefVol, lpoj, lmoj);
          // Is TRAVERSING through the (quasi-)common wall?
          double ljns[2][3], lovts[2][3], lpjni[3], lpfnd[3];
          status = cTraversing(lpoj, lmoj, sim_hjt.getPathLength() * ed2dd,
                               sim_hit.isProducedBySecondary(), rMin, rMax, dZ, startPhi, endPhi,
                               ljns, lovts, lpjni, lpfnd);
          if (status & 0xff000) { // Inconsistency => Drop current "sim_hjt"
            critical(inconsistency(header, status, sim_hjt.getCellID(), lpoj, lmoj));
            if (unbroken)
              idx = jdx - 1;
            unbroken = 0;
            continue;
          }
          // ij-Compatibility: status
          bool jsDownstream = m_isDownstream(orientation, status);
          if (!jsDownstream) {
            if (sim_hit.isProducedBySecondary())
              break;
            else { // Allow for primary hits to not come in unbroken sequence
              if (unbroken)
                idx = jdx - 1;
              unbroken = 0;
              continue;
            }
          }
          // ij-Compatibility: close exit/entrance-distance
          double dist            = outInDistance(0, orientation, ljns, louts, lmom, lmoj);
          const double tolerance = 25 * dd4hep::um;
          bool isCompatible      = dist > 0 && dist < tolerance;
          if (!isCompatible) {
            if (!sim_hit.isProducedBySecondary())
              debug(oddity(header, status, dist, sim_hit.getCellID(), lpos, lmom,
                           /* */ sim_hjt.getCellID(), lpoj, lmoj));
            if (unbroken)
              idx = jdx - 1;
            unbroken = 0;
            continue;
          }
          // ***** UPDATE
          vIDPrv = vJD;
          eDep += sim_hjt.getEDep();
          for (int i = 0; i < 3; i++) { // Update end point position/momentum.
            lpend[i] = lpfnd[i];
            lmend[i] = lmoj[i];
          }
          // ***** BOOK-KEEPING
          usedHits.push_back(jdx);
          cIDs.push_back(sim_hjt.getCellID());
          if (level() >= algorithms::LogLevel::kDebug) {
            same_idcs.push_back(jdx);
          }
          // ***** CONTINUATION?
          hasContinuation = status & 0xa;
          canReEnter      = status & 0x100;
          if (!canReEnter && m_stripRank(vJD) == 4)
            hasContinuation = false;
          if (!hasContinuation) {
            jdx++;
            break;
          } else { // Update outgoing position/momentum for next iteration.
            for (int i = 0; i < 3; i++) {
              louts[0][i] = lovts[0][i];
              louts[1][i] = lovts[1][i];
            }
          }
        }
        if (unbroken)
          idx = jdx - 1;
      }
      // ***** EXTENSION?...
      if (sim_hit.isProducedBySecondary() && cIDs.size() < 2)
        if (denyExtension(sim_hit, tCur.rMax() - tCur.rMin())) {
          isContinuation = hasContinuation = false;
        }
      for (int io = 0; io < 2; io++) { // ...into/out-of
        if ((io == 0 && !isContinuation) || (io == 1 && !hasContinuation))
          continue;
        int direction = io ? +1 : -1;
        status        = extendHit(refID, direction, lpini, lmom, lpend, lmend);
        if (status & 0xff000) { // Inconsistency => Drop current "sim_hit"
          critical(inconsistency(header, status, sim_hit.getCellID(), lpos, lmom));
          continue;
        }
      }
      // ***** FLAG CASES W/ UNEXPECTED OUTCOME
      flagUnexpected(header, 0, (tRef.rMin() + tRef.rMax()) / 2, sim_hit, lpini, lpend, lpos, lmom);
      // ***** UPDATE (local position <lpos>, DoF
      double DoF2 = 0, dir = 0;
      for (int i = 0; i < 3; i++) {
        double neu = (lpini[i] + lpend[i]) / 2, alt = lpos[i];
        lpos[i]  = neu;
        double d = neu - alt;
        dir += d * lmom[i];
        DoF2 += d * d;
      }
      // Update time by ToF from original subHit to extended/COALESCED.
      time += ((dir > 0) ? 1 : ((dir < 0) ? -1 : 0)) * sqrt(DoF2) / dd4hep::c_light;
    } else if (!strcmp(shape.type(), "TGeoBBox")) {
      // ********** BOX GEOMETRY
      const Box& bRef = refVol.solid(); // REFERENCE SUBVOLUME
      double dX = bRef.x(), dY = bRef.y();
      // Get current SUBVOLUME
      DetElement curVol = volman.lookupDetElement(vID);
      const Box& bCur   = curVol.solid();
      double dZ         = bCur.z();
      double ref2Cur    = getRef2Cur(refVol, curVol);
      // Is TRAVERSING?
      double lintos[2][3], louts[2][3], lpini[3], lpend[3], lmend[3];
      std::copy(std::begin(lmom), std::end(lmom), std::begin(lmend));
      unsigned int status =
          bTraversing(lpos, lmom, ref2Cur, sim_hit.getPathLength() * ed2dd,
                      sim_hit.isProducedBySecondary(), dZ, dX, dY, lintos, louts, lpini, lpend);
      if (status & 0xff000) { // Inconsistency => Drop current "sim_hit"
        critical(inconsistency(header, status, sim_hit.getCellID(), lpos, lmom));
        continue;
      }
      cIDs.push_back(sim_hit.getCellID());
      if (level() >= algorithms::LogLevel::kDebug) {
        same_idcs.push_back(idx);
      }
      // Continuations.
      int rank             = m_stripRank(vID);
      bool isContinuation  = status & 0x5;
      bool hasContinuation = status & 0xa;
      if ((rank == 0 && (status & 0x1)) || (rank == 4 && (status & 0x4)))
        isContinuation = false;
      if ((rank == 0 && (status & 0x2)) || (rank == 4 && (status & 0x8)))
        hasContinuation = false;
      if (hasContinuation) {
        // ***** LOOP OVER SUBHITS
        int jdx, unbroken /* unbroken succession of indices */;
        CellID vIDPrv = vID;
        for (jdx = idx + 1, unbroken = 1; jdx < (int)sim_size; jdx++) {
          const edm4hep::SimTrackerHit& sim_hjt = sim_hits->at(jdx);
          // Used hit? If indeed, it's going to be discarded anyway in the
          // following: current "sim_hit" is by construction at variance to it.
          CellID vJD      = sim_hjt.getCellID() & m_volumeBits;
          int orientation = m_orientation(vIDPrv, vJD);
          bool isUpstream = m_isUpstream(orientation, status);
          bool pmoStatus  = samePMO(sim_hit, sim_hjt, unbroken);
          if (!pmoStatus || !isUpstream) {
            if ((pmoStatus && !isUpstream) && !sim_hit.isProducedBySecondary())
              // Bizarre: let's flag the case while debugging
              debug(inconsistency(header, 0, sim_hit.getCellID(), lpos, lmom));
            if (unbroken)
              idx = jdx - 1;
            unbroken = 0;
            continue;
          }
          // Get 'j' Z
          curVol          = volman.lookupDetElement(vJD); // 'j' SUBVOLUME
          const Box& boxj = curVol.solid();
          dZ              = boxj.z();
          double ref2j    = getRef2Cur(refVol, curVol);
          // Is TRAVERSING through the (quasi)-common border?
          double lpoj[3], lmoj[3];
          getLocalPosMom(sim_hjt, toRefVol, lpoj, lmoj);
          double ljns[2][3], lovts[2][3], lpjni[3], lpfnd[3];
          status =
              bTraversing(lpoj, lmoj, ref2j, sim_hjt.getPathLength() * ed2dd,
                          sim_hit.isProducedBySecondary(), dZ, dX, dY, ljns, lovts, lpjni, lpfnd);
          if (status & 0xff000) { // Inconsistency => Drop current "sim_hjt"
            critical(inconsistency(header, status, sim_hjt.getCellID(), lpoj, lmoj));
            if (unbroken)
              idx = jdx - 1;
            unbroken = 0;
            continue;
          }
          // ij-Compatibility: status
          bool jsDownstream = m_isDownstream(orientation, status);
          if (!jsDownstream) {
            if (sim_hit.isProducedBySecondary())
              break;
            else {
              if (unbroken)
                idx = jdx - 1;
              unbroken = 0;
              continue;
            }
          }
          // ij-Compatibility: close exit/entrance-distance
          double dist            = outInDistance(1, orientation, ljns, louts, lmom, lmoj);
          const double tolerance = 25 * dd4hep::um;
          bool isCompatible      = dist > 0 && dist < tolerance;
          if (!isCompatible) {
            if (!sim_hit.isProducedBySecondary())
              debug(oddity(header, status, dist, sim_hit.getCellID(), lpos, lmom,
                           /* */ sim_hjt.getCellID(), lpoj, lmoj));
            if (unbroken)
              idx = jdx - 1;
            unbroken = 0;
            continue;
          }
          // ***** UPDATE
          vIDPrv = vJD;
          eDep += sim_hjt.getEDep();
          for (int i = 0; i < 3; i++) { // Update end point position/momentum.
            lpend[i] = lpfnd[i];
            lmend[i] = lmoj[i];
          }
          // ***** BOOK-KEEPING
          usedHits.push_back(jdx);
          cIDs.push_back(sim_hjt.getCellID());
          if (level() >= algorithms::LogLevel::kDebug) {
            same_idcs.push_back(jdx);
          }
          // ***** CONTINUATION?
          hasContinuation = status & 0xa;
          if (!hasContinuation) {
            jdx++;
            break;
          } else { // Update outgoing position/momentum for next iteration.
            for (int i = 0; i < 3; i++) {
              louts[0][i] = lovts[0][i];
              louts[1][i] = lovts[1][i];
            }
          }
        }
        if (unbroken)
          idx = jdx - 1;
      }
      // ***** EXTENSION?...
      if (sim_hit.isProducedBySecondary() && cIDs.size() < 2)
        if (denyExtension(sim_hit, bCur.z())) {
          isContinuation = hasContinuation = false;
        }
      for (int io = 0; io < 2; io++) { // ...into/out-of
        if ((io == 0 && !isContinuation) || (io == 1 && !hasContinuation))
          continue;
        int direction = io ? +1 : -1;
        status        = extendHit(refID, direction, lpini, lmom, lpend, lmend);
        if (status & 0xff000) { // Inconsistency => Drop current "sim_hit"
          critical(inconsistency(header, status, sim_hit.getCellID(), lpos, lmom));
          continue;
        }
      }
      // ***** FLAG CASES W/ UNEXPECTED OUTCOME
      flagUnexpected(header, 1, 0, sim_hit, lpini, lpend, lpos, lmom);
      // ***** UPDATE (local position <lpos>, DoF)
      double DoF2 = 0, dir = 0;
      for (int i = 0; i < 3; i++) {
        double neu = (lpini[i] + lpend[i]) / 2, alt = lpos[i];
        lpos[i]  = neu;
        double d = neu - alt;
        dir += d * lmom[i];
        DoF2 += d * d;
      }
      // Update time by ToF from original subHit to extended/COALESCED.
      time += ((dir > 0) ? 1 : ((dir < 0) ? -1 : 0)) * sqrt(DoF2) / dd4hep::c_light;
    } else {
      critical("Bad input data: CellID {:x} has invalid shape (=\"{}\")", refID, shape.type());
      throw JException("Bad input data: invalid geometry");
    }
    // ***** CELLIDS of (p|n)-STRIP HITS
    Position locPos(lpos[0], lpos[1], lpos[2]); // Simplification: strip surface = REFERENCE surface
    // p "strip"
    CellID vIDp = refID | m_pStripBit;
    CellID cIDp = m_seg->cellID(locPos, dummy, vIDp);
    // n "strip"
    CellID vIDn         = refID | m_nStripBit;
    CellID cIDn         = m_seg->cellID(locPos, dummy, vIDn);
    double result_time  = time + time_smearing;
    auto hit_time_stamp = (std::int32_t)(result_time * 1e3);

    // ***** DEBUGGING INFO
    if (level() >= algorithms::LogLevel::kDebug) {
      debug("--------------------");
      for (CellID cID : {cIDp, cIDn}) {
        std::string sCellID = cID == cIDp ? "cellIDp" : "cellIDn";
        CellID hID = cID >> 32, vID = cID & m_volumeBits, sID = vID >> 28 & 0xff;
        debug("Hit {} = 0x{:08x}, 0x{:08x} 0x{:02x}", sCellID, hID, vID, sID);
        Position stripPos = m_seg->position(cID);
        Position globPos  = refVol.nominal().localToWorld(stripPos);
        debug("  position  = ({:7.2f},{:7.2f},{:7.2f}) [mm]", globPos.X() / mm, globPos.Y() / mm,
              globPos.Z() / mm);
      }
      debug("  edep = {:.0f} [eV]", eDep / eV);
      debug("  time = {:.2f} [ns]", time);
#if EDM4HEP_BUILD_VERSION >= EDM4HEP_VERSION(0, 99, 0)
      debug("  particle time = {} [ns]", sim_hit.getParticle().getTime());
#else
      debug("  particle time = {} [ns]", sim_hit.getMCParticle().getTime());
#endif
      debug("  time smearing: {:.2f}, resulting time = {:.2f} [ns]", time_smearing, result_time);
      debug("  hit_time_stamp: {} [~ps]", hit_time_stamp);
      for (int ldx = 0; ldx < (int)same_idcs.size(); ldx++) {
        int jdx                               = same_idcs[ldx];
        const edm4hep::SimTrackerHit& sim_hjt = sim_hits->at(jdx);
        CellID cIDk                           = sim_hjt.getCellID();
        CellID hIDk = cIDk >> 32, vIDk = cIDk & m_volumeBits, sIDk = vIDk >> 28 & 0xff;
        debug("Hit cellID{:d} = 0x{:08x}, 0x{:08x} 0x{:02x}", ldx, hIDk, vIDk, sIDk);
        debug("  position  = ({:7.2f},{:7.2f},{:7.2f}) [mm]", sim_hjt.getPosition().x / edmm,
              sim_hjt.getPosition().y / edmm, sim_hjt.getPosition().z / edmm);
        debug("  xy_radius = {:.2f}",
              std::hypot(sim_hjt.getPosition().x, sim_hjt.getPosition().y) / edmm);
        debug("  momentum  = ({:.2f}, {:.2f}, {:.2f}) [GeV]", sim_hjt.getMomentum().x / GeV,
              sim_hjt.getMomentum().y / GeV, sim_hjt.getMomentum().z / GeV);
        debug("  edep = {:.0f} [eV]", sim_hjt.getEDep() / eV);
        debug("  time = {:.2f} [ns]", sim_hjt.getTime());
      }
    }

    // ***** APPLY THRESHOLD
    if (eDep < m_cfg.threshold) {
      debug("  edep is below threshold of {:.2f} [keV]", m_cfg.threshold / keV);
      continue;
    }

    // ***** HIT ACCUMULATION
    for (CellID cID : {cIDp, cIDn}) {
      stripID2cIDs[cID] = cIDs;
      if (!cell_hit_map.contains(cID)) {
        // This cell doesn't have hits
        cell_hit_map[cID] = {
            cID, (std::int32_t)std::llround(eDep * 1e6),
            hit_time_stamp // ns->ps
        };
      } else {
        // There is previous values in the cell
        auto& hit = cell_hit_map[cID];
        debug("  Hit already exists in cell ID={}, prev. hit time: {}", cID, hit.getTimeStamp());

        // keep earliest time for hit
        hit.setTimeStamp(std::min(hit_time_stamp, hit.getTimeStamp()));

        // sum deposited energy
        auto charge = hit.getCharge();
        hit.setCharge(charge + (std::int32_t)std::llround(
                                   eDep * 1e6)); // TODO: accumulate charge: shouldn't it be float?
      }
    }
  }

  // ***** raw_hit INSTANTIATION AND raw<-sim_hit's ASSOCIATION:
  for (auto item : cell_hit_map) {
    raw_hits->push_back(item.second);
    CellID stripID = item.first;
    const auto is  = stripID2cIDs.find(stripID);
    if (is == stripID2cIDs.end()) {
      critical("Inconsistency: CellID {:x} not found in \"stripID2cIDs\" map", stripID);
      throw JException("Inconsistency in the handling of \"stripID2cIDs\" map");
    }
    std::vector<std::uint64_t> cIDs = is->second;
    for (CellID cID : cIDs) {
      for (const auto& sim_hit : *sim_hits) {
        if (sim_hit.getCellID() == cID) {
          // set association
          auto hitassoc = associations->create();
          hitassoc.setWeight(1.0);
          hitassoc.setRawHit(item.second);
          hitassoc.setSimHit(sim_hit);
        }
      }
    }
  }
}

void getLocalPosMom(const edm4hep::SimTrackerHit& sim_hit, const TGeoHMatrix& toModule,
                    double* lpos, double* lmom) {
  const edm4hep::Vector3d& pos = sim_hit.getPosition();
  // Length: Inputs are in EDM4eic units. Let's move to DD4hep units.
  const double edmm = edm4eic::unit::mm, ed2dd = dd4hep::mm / edmm;
  const double gpos[3]         = {pos.x * ed2dd, pos.y * ed2dd, pos.z * ed2dd};
  const edm4hep::Vector3f& mom = sim_hit.getMomentum();
  const double gmom[3]         = {mom.x, mom.y, mom.z};
  toModule.MasterToLocal(gpos, lpos);
  toModule.MasterToLocalVect(gmom, lmom);
}

// ******************** TRAVERSING?
// Particle can be born/dead (then its position is not (entrance+exit)/2).
// Or it can exit through the edge.
// - Returned bit pattern:
//   0x1: Enters through lower wall
//   0x2: Exits  through lower wall
//   0x4: Enters through upper wall
//   0x8: Exits  through upper wall
//   0x100: Can reEnter (in a cylindrical volume)
//     Also, for internal use:
//     0x10: Enters through edge
//     0x20: Exits  through edge
//   0xff000: Inconsistency...
// - <lintos>/<louts>: Positions @ lower/upper wall upon Enter-/Exit-ing (when endorsed by <status>)
// - <lpini>/<lpend>: Positions of extrema
// - Tolerance? For MIPs, a tolerance of 1 µM works fine. But for lower energy,
//  looks like we need something somewhat larger. The ideal would be to base
//  the value on Molière width. Here, I use a somewhat arbitrary, built-in, of
//  25 µm.
//  - If particle found to reach wall, w/in tolerance, assign end points to
//   walls (instead of <lpos>+/-path/2). It will make so that the eventual
//   extrapolated position falls exactly at mid-plane, even in the case of a
//   low energy particle, where path may be affected by multiscattering. This,
//   provided that particle is not a secondary.
unsigned int cTraversing(const double* lpos, const double* lmom, double path,
                         bool isSecondary,                          // Input subHit
                         double rMin, double rMax,                  // Current instance of SUBVOLUME
                         double dZ, double startPhi, double endPhi, // Module parameters
                         double lintos[][3], double louts[][3], double* lpini, double* lpend) {
  unsigned int status = 0;
  double Mx = lpos[0], My = lpos[1], Mz = lpos[2], M2 = Mx * Mx + My * My;
  double Px = lmom[0], Py = lmom[1], Pz = lmom[2];
  // Intersection w/ the edge in phi
  double tIn = 0, tOut = 0;
  for (double phi : {startPhi, endPhi}) {
    // M+t*P = 0 + t'*U. t = (My*Ux-Mx*Uy)/(Px*Uy-Py*Ux);
    double Ux = cos(phi), Uy = sin(phi);
    double D = Px * Uy - Px * Ux;
    if (D) { // If P not // to U
      double t  = (My * Ux - Mx * Uy) / D;
      double Ex = Mx + t * Px, Ey = My + t * Py, rE = sqrt(Ex * Ex + Ey * Ey), Ez = Mz + t * Pz;
      if (rMin < rE && rE < rMax && fabs(Ez) < dZ) {
        if (t < 0) {
          status |= 0x10;
          tIn = t;
        } else {
          status |= 0x20;
          tOut = t;
        }
      }
    }
  }
  // Intersection w/ the edge in Z
  double zLow = -dZ, zUp = +dZ;
  for (double Z : {zLow, zUp}) {
    // Mz+t*Pz = Z
    if (Pz) {
      double t  = (Z - Mz) / Pz;
      double Ex = Mx + t * Px, Ey = My + t * Py, rE = sqrt(Ex * Ex + Ey * Ey);
      double phi = atan2(Ey, Ex);
      if (rMin < rE && rE < rMax && startPhi < phi && phi < endPhi) {
        if (t < 0) {
          if (!(status & 0x10) || ((status & 0x10) && t > tIn)) {
            status |= 0x10;
            tIn = t;
          }
        } else if (t > 0) {
          if (!(status & 0x20) || ((status & 0x20) && t < tOut)) {
            status |= 0x20;
            tOut = t;
          }
        }
      }
    }
  }
  // Intersection w/ tube walls
  double ts[3 /* rMin/rMax/edge */][2 /* In/Out */] = {
      {0, 0}, {0, 0}, {tIn, tOut}}; // Up to two intersections
  double a = Px * Px + Py * Py, b = Px * Mx + Py * My;
  for (int lu = 0; lu < 2; lu++) { // rMin/rMax
    double R;
    unsigned int statGene;
    if (lu == 1) {
      R        = rMax;
      statGene = 0x4;
    } else {
      R        = rMin;
      statGene = 0x1;
    }
    double c = M2 - R * R;
    if (!a) { // P is // to Z. Yet no intersect w/ Z edge.
      if ((status & 0x30) != 0x30)
        status |= 0x1000;
      continue;      // Inconsistency
    } else if (!c) { // Hit is on wall: inconsistency.
      status |= 0x2000;
      continue;
    } else {
      double det = b * b - a * c;
      if (det < 0) {
        if (lu == 1) { // No intersection w/ outer wall: inconsistency.
          status |= 0x4000;
          continue;
        }
      } else {
        double sqdet = sqrt(det);
        for (int is = 0; is < 2; is++) {
          int s     = 1 - 2 * is;
          double t  = (-b + s * sqdet) / a;
          double Ix = Mx + t * Px, Iy = My + t * Py, Iz = Mz + t * Pz, phi = atan2(Iy, Ix);
          if (fabs(Iz) > dZ || phi < startPhi || endPhi < phi)
            continue;
          if (t < 0) {
            // Two rMin intersects in same back/forward direction may happen
            // (one and and only one of them may then be hidden by edge).
            // => Have to allow wall intersect to coexist w/ edge intersect.
            //   This only for rMin, but for simplicity's sake...
            if (status & statGene) { // Two <0 ts: can only happen when rMin
              double tPrv = ts[lu][0];
              if (t > tPrv) {
                ts[lu][0] = t;
                ts[lu][1] = tPrv; // Current is actually IN, previous is OUT despite being <0
              }
              status |= statGene << 1;
            } else {
              ts[lu][0] = t;
              status |= statGene;
            }
          } else {                        // (if t > 0)
            if (status & statGene << 1) { // Two >0 ts: can only happen when rMin
              double tPrv = ts[lu][1];
              if (t < tPrv) {
                ts[lu][1] = t;
                ts[lu][0] = tPrv; // Current is actually OUT, previous is IN despite being >0
              }
              status |= statGene;
            } else {
              ts[lu][1] = t;
              status |= statGene << 1;
            }
          }
        }
      }
    }
  }
  // Combine w/ edge in/out, based on "t"
  for (int lu = 0; lu < 2; lu++) { // rMin/rMax
    unsigned int statGene = lu ? 0x4 : 0x1;
    if (status & statGene) {
      double t = ts[lu][0];
      if (t < 0) {
        if (status & 0x10) {
          if (t < 0 && t > tIn)
            status |= 0x10000;
          status &= ~statGene;
        }
      } else { // if (t > 0)
        if (status & 0x20) {
          if (t > 0 && t < tOut)
            status |= 0x20000;
          status &= ~statGene;
        }
      }
    }
    if (status & statGene << 1) {
      double t = ts[lu][1];
      if (t < 0) {
        if (status & 0x10) {
          if (t < 0 && t > tIn)
            status |= 0x40000;
          status &= ~(statGene << 1);
        }
      } else { // if (t > 0)
        if (status & 0x20) {
          if (t > 0 && t < tOut)
            status |= 0x80000;
          status &= ~(statGene << 1);
        }
      }
    }
  }
  // Is particle born/dead prior to entering/exiting?
  // - sim_hit must have been assigned the mean position: (entrance+exit)/2
  // - Let's then check entrance/exit against sim_hit's position +/- path/2.
  //   When the latter is too short, it means that the particle firing the
  //  hit gets born or dies in the SUBVOLUME.
  // => We remove the corresponding bit in the <status> pattern.
  // - Note that we not only require that the path be long enough, but also
  //  that it matches exactly distances to entrance/exit.
  bool isReEntering = (status & 0x3) == 0x3;
  if (isReEntering)
    status |= 0x100; // Remember that particle can re-enter.
  double norm = sqrt(a + Pz * Pz), at = path / 2 / norm;
  unsigned int statws = 0;
  for (int is = 0; is < 2; is++) {
    int s     = 1 - 2 * is;
    double Ix = s * at * Px, Iy = s * at * Py, Iz = s * at * Pz;
    for (int lu = 0; lu < 2; lu++) { // Lower/upper wall
      unsigned int statvs = lu ? 0x4 : 0x1;
      for (int io = 0; io < 2; io++) {
        statvs <<= io;
        if (status & statvs) {
          double t = ts[lu][io];
          if (t * s < 0)
            continue;
          double dIx = t * Px - Ix, dIy = t * Py - Iy, dIz = t * Pz - Iz;
          double dist            = sqrt(dIx * dIx + dIy * dIy + dIz * dIz);
          const double tolerance = 20 * dd4hep::um;
          if (dist < tolerance)
            statws |= statvs;
        }
      }
    }
  }
  if (!(statws & 0x5)) /* No entrance */
    status &= ~0x5;
  if (!(statws & 0xa)) /* No exit */
    status &= ~0xa;
  // ***** End points
  // Assign end points to walls, if not a secondary and provided it's not a
  // reEntrance case, which case is more difficult to handle and we leave aside.
  if (((status & 0x5) == 0x1 || (status & 0x5) == 0x4) && !isSecondary) {
    double tIn = (status & 0x1) ? ts[0][0] : ts[1][0];
    lpini[0]   = Mx + tIn * Px;
    lpini[1]   = My + tIn * Py;
    lpini[2]   = Mz + tIn * Pz;
  } else {
    lpini[0] = Mx - at * Px;
    lpini[1] = My - at * Py;
    lpini[2] = Mz - at * Pz;
  }
  if (((status & 0xa) == 0x2 || (status & 0xa) == 0x8) && !isSecondary) {
    double tOut = (status & 0x2) ? ts[0][1] : ts[1][1];
    lpend[0]    = Mx + tOut * Px;
    lpend[1]    = My + tOut * Py;
    lpend[2]    = Mz + tOut * Pz;
  } else {
    lpend[0] = Mx + at * Px;
    lpend[1] = My + at * Py;
    lpend[2] = Mz + at * Pz;
  }
  // End points when on the walls
  for (int lu = 0; lu < 2; lu++) {
    unsigned int statvs = lu ? 0x4 : 0x1;
    double tIn = ts[lu][0], tOut = ts[lu][1];
    if (status & statvs) {
      lintos[lu][0] = Mx + tIn * Px;
      lintos[lu][1] = My + tIn * Py;
      lintos[lu][2] = Mz + tIn * Pz;
    }
    statvs <<= 1;
    if (status & statvs) {
      louts[lu][0] = Mx + tOut * Px;
      louts[lu][1] = My + tOut * Py;
      louts[lu][2] = Mz + tOut * Pz;
    }
  }
  return status;
}
unsigned int bTraversing(const double* lpos, const double* lmom, double ref2Cur, double path,
                         bool isSecondary,     // Input subHit
                         double dZ,            // Current instance of SUBVOLUME
                         double dX, double dY, // Module parameters
                         double lintos[][3], double louts[][3], double* lpini, double* lpend) {
  unsigned int status = 0;
  double Mx = lpos[0], My = lpos[1], Mxy[2] = {Mx, My};
  double Px = lmom[0], Py = lmom[1], Pxy[2] = {Px, Py};
  double Mz = lpos[2] + ref2Cur, Pz = lmom[2];
  // Intersection w/ the edge in X,Y
  double tIn = 0, tOut = 0;
  double xyLow[2] = {-dX, +dX}, xyUp[2] = {-dY, +dY};
  for (int xy = 0; xy < 2; xy++) {
    int yx       = 1 - xy;
    double a_Low = xyLow[xy], a_Up = xyUp[xy], Pa = Pxy[xy];
    double b_Low = xyLow[yx], b_Up = xyUp[yx], Mb = Mxy[yx], Pb = Pxy[yx];
    for (double A : {a_Low, a_Up}) {
      // Mz+t*Pz = A
      if (Pa) {
        double t  = (A - Mz) / Pa;
        double Eb = Mb + t * Pb, Ez = Mz + t * Pz;
        if (b_Low < Eb && Eb < b_Up && fabs(Ez) < dZ) {
          if (t < 0) {
            if (!(status & 0x10) || ((status & 0x10) && t > tIn)) {
              status |= 0x10;
              tIn = t;
            }
          } else if (t > 0) {
            if (!(status & 0x20) || ((status & 0x20) && t < tOut)) {
              status |= 0x20;
              tOut = t;
            }
          }
        }
      }
    }
  }
  if (status) {
    printf("DEBUG: Edge 0x%x\n", status);
  }
  // Intersection w/ box walls
  for (int lu = 0; lu < 2; lu++) {
    int s                 = 2 * lu - 1;
    double Z              = s * dZ;
    unsigned int statGene = lu ? 0x4 : 0x1;
    // Mz+t*Pz = Z
    if (Pz) {
      double t = (Z - Mz) / Pz;
      if (t < 0) {
        if (!(status & 0x10) || ((status & 0x10) && t > tIn)) {
          status |= statGene;
          tIn = t;
        }
      } else if (t > 0) {
        if (!(status & 0x20) || ((status & 0x20) && t < tOut)) {
          status |= statGene << 1;
          tOut = t;
        }
      }
    }
  }
  // Is particle born/dead prior to entering/exiting?
  // - sim_hit must have been assigned the mean position: (entrance+exit)/2
  // - Let's then check entrance/exit against sim_hit's position +/- path/2.
  //   When the latter is too short, it means that the particle firing the
  //  hit gets born or dies in the SUBVOLUME.
  // => We remove the corresponding bit in the <status> pattern.
  // - Note that we not only require that the path be long enough, but also
  //  that it matches exactly distances to entrance/exit.
  double norm = sqrt(Px * Px + Py * Py + Pz * Pz), at = path / 2 / norm;
  unsigned int statws = 0;
  for (int is = 0; is < 2; is++) {
    int s     = 1 - 2 * is;
    double Ix = s * at * Px, Iy = s * at * Py, Iz = s * at * Pz;
    for (int lu = 0; lu < 2; lu++) { // Lower/upper wall
      unsigned int statvs = lu ? 0x4 : 0x1;
      for (int io = 0; io < 2; io++) {
        statvs <<= io;
        if (status & statvs) {
          double t = io ? tOut : tIn;
          if (t * s < 0)
            continue;
          double dIx = t * Px - Ix, dIy = t * Py - Iy, dIz = t * Pz - Iz;
          double dist            = sqrt(dIx * dIx + dIy * dIy + dIz * dIz);
          const double tolerance = 20 * dd4hep::um;
          if (dist < tolerance)
            statws |= statvs;
        }
      }
    }
  }
  if (!(statws & 0x5)) /* No entrance */
    status &= ~0x5;
  if (!(statws & 0xa)) /* No exit */
    status &= ~0xa;
  // ***** OUTPUT POSITIONS
  Mz -= ref2Cur; // Go back to REFERENCE SUBVOLUME
  // End points:
  // Assign end points to walls, if not a secondary.
  if ((status & 0x5) && !isSecondary) {
    lpini[0] = Mx + tIn * Px;
    lpini[1] = My + tIn * Py;
    lpini[2] = Mz + tIn * Pz;
  } else {
    lpini[0] = Mx - at * Px;
    lpini[1] = My - at * Py;
    lpini[2] = Mz - at * Pz;
  }
  if ((status & 0xa) && !isSecondary) {
    lpend[0] = Mx + tOut * Px;
    lpend[1] = My + tOut * Py;
    lpend[2] = Mz + tOut * Pz;
  } else {
    lpend[0] = Mx + at * Px;
    lpend[1] = My + at * Py;
    lpend[2] = Mz + at * Pz;
  }
  // End points when on the walls:
  for (int lu = 0; lu < 2; lu++) {
    unsigned int statvs = lu ? 0x4 : 0x1;
    if (status & statvs) {
      lintos[lu][0] = Mx + tIn * Px;
      lintos[lu][1] = My + tIn * Py;
      lintos[lu][2] = Mz + tIn * Pz;
    }
    statvs <<= 1;
    if (status & statvs) {
      louts[lu][0] = Mx + tOut * Px;
      louts[lu][1] = My + tOut * Py;
      louts[lu][2] = Mz + tOut * Pz;
    }
  }
  return status;
}

// ***** EXTRAPOLATE
bool cExtrapolate(const double* lpos, const double* lmom, // Input subHit
                  double rT,                              // Target radius
                  double* lext)                           // Extrapolated position @ <rT>
{
  bool ok   = false;
  double Mx = lpos[0], My = lpos[1], Mz = lpos[2], M2 = Mx * Mx + My * My;
  double Px = lmom[0], Py = lmom[1], Pz = lmom[2];
  double a = Px * Px + Py * Py, b = Px * Mx + Py * My, c = M2 - rT * rT;
  double tF = 0;
  if (!c)
    ok = true;
  else if (a) { // P is not // to Z
    double det = b * b - a * c;
    if (det >= 0) {
      double sqdet = sqrt(det);
      for (int is = 0; is < 2; is++) {
        int s    = 1 - 2 * is;
        double t = (-b + s * sqdet) / a;
        if (t < 0)
          continue;
        if (!ok ||
            // Two intersects: let's retain the earliest one.
            (ok && fabs(t) < fabs(tF))) {
          tF = t;
          ok = true;
        }
      }
    }
  }
  if (ok) {
    lext[0] = Mx + tF * Px;
    lext[1] = My + tF * Py;
    lext[2] = Mz + tF * Pz;
  }
  return ok;
}
bool bExtrapolate(const double* lpos, const double* lmom, // Input subHit
                  double zT,                              // Target Z
                  double* lext)                           // Extrapolated position @ <zT>
{
  bool ok   = false;
  double Mx = lpos[0], My = lpos[1], Mz = lpos[2];
  double Px = lmom[0], Py = lmom[1], Pz = lmom[2];
  double tF = 0;
  if (Pz) {
    tF = (zT - Mz) / Pz;
    ok = tF > 0;
  }
  if (ok) {
    lext[0] = Mx + tF * Px;
    lext[1] = My + tF * Py;
    lext[2] = Mz + tF * Pz;
  }
  return ok;
}

// ***** EXTENSION
// At variance to EXTRAPOLATION, we take edges (phi,Z/X,Y) into account.
// - Returns 0x1 if
//   - there is an extrapolation between position <lpos> and target,
//   - within edge limits,
//   - along momentum <lmom>,
//   - in direction <direction>.
// - Returns something in the 0xff000: Inconsistency...
// - <lext> contains the position of farthest extension.
unsigned int MPGDTrackerDigi::cExtension(double const* lpos, double const* lmom, // Input subHit
                                         double rT,                              // Target radius
                                         int direction, double dZ, double startPhi,
                                         double endPhi, // Module parameters
                                         double* lext) const {
  unsigned int status = 0;
  double Mx = lpos[0], My = lpos[1], Mz = lpos[2];
  double Px = lmom[0], Py = lmom[1], Pz = lmom[2], norm = sqrt(Px * Px + Py * Py + Pz * Pz);
  // Move some distance away from <lpos>, which is expected to be sitting on
  // the wall of the SUBVOLUME to be ``extended''.
  const double margin = 10 * dd4hep::um;
  double t            = direction * margin / norm;
  Mx += t * Px;
  My += t * Py;
  Mz += t * Pz;
  double M2 = Mx * Mx + My * My, rIni = sqrt(M2), rLow, rUp;
  if (rIni < rT) {
    rLow = rIni;
    rUp  = rT;
  } else {
    rLow = rT;
    rUp  = rIni;
  }
  // Intersection w/ the edge in phi
  double tF = 0;
  for (double phi : {startPhi, endPhi}) {
    // M+t*P = 0 + t'*U. t = (My*Ux-Mx*Uy)/(Px*Uy-Py*Ux);
    double Ux = cos(phi), Uy = sin(phi);
    double D = Px * Uy - Px * Ux;
    if (D) { // If P not // to U
      double t = (My * Ux - Mx * Uy) / D;
      if (t * direction < 0)
        continue;
      double Ex = Mx + t * Px, Ey = My + t * Py, rE = sqrt(Ex * Ex + Ey * Ey), Ez = Mz + t * Pz;
      if (rLow < rE && rE < rUp && fabs(Ez) < dZ) {
        status |= 0x1;
        tF = t;
      }
    }
  }
  // Intersection w/ the edge in Z
  double zLow = -dZ, zUp = +dZ;
  for (double Z : {zLow, zUp}) {
    // Mz+t*Pz = Z
    if (Pz) {
      double t = (Z - Mz) / Pz;
      if (t * direction < 0)
        continue;
      double Ex = Mx + t * Px, Ey = My + t * Py, rE = sqrt(Ex * Ex + Ey * Ey);
      double phi = atan2(Ey, Ex);
      if (rLow < rE && rE < rUp && startPhi < phi && phi < endPhi) {
        if (t < 0) {
          if (!status || (status && t > tF)) {
            status |= 0x1;
            tF = t;
          }
        } else if (t > 0) {
          if (!status || (status && t < tF)) {
            status |= 0x1;
            tF = t;
          }
        }
      }
    }
  }
  // Else intersection w/ target radius
  if (!status) {
    double a = Px * Px + Py * Py, b = Px * Mx + Py * My, c = M2 - rT * rT;
    if (!a) { // P is // to Z
      if (!status)
        status |= 0x1000; // Inconsistency
    } else if (!c) {      // Hit is on target
      status |= 0x2000;   // Inconsistency
    } else {
      double det = b * b - a * c;
      if (det >= 0) {
        double sqdet = sqrt(det);
        for (int is = 0; is < 2; is++) {
          int s    = 1 - 2 * is;
          double t = (-b + s * sqdet) / a;
          if (t * direction < 0)
            continue;
          double Ix = Mx + t * Px, Iy = My + t * Py, Iz = Mz + t * Pz, phi = atan2(Iy, Ix);
          if (fabs(Iz) > dZ || phi < startPhi || endPhi < phi)
            continue;
          if (!(status & 0x1) ||
              // Two intersects: let's retain the earliest one.
              ((status & 0x1) && fabs(t) < fabs(tF))) {
            tF = t;
            status |= 0x1;
          }
        }
      }
    }
  }
  if (status) {
    lext[0] = Mx + tF * Px;
    lext[1] = My + tF * Py;
    lext[2] = Mz + tF * Pz;
  }
  return status;
}
unsigned int MPGDTrackerDigi::bExtension(const double* lpos, const double* lmom, // Input subHit
                                         double zT,                              // Target Z
                                         int direction, double dX, double dY, // Module parameters
                                         double* lext) const {
  unsigned int status = 0;
  double Mx = lpos[0], My = lpos[1], Mxy[2] = {Mx, My};
  double Px = lmom[0], Py = lmom[1], Pxy[2] = {Px, Py};
  double Mz = lpos[2], Pz = lmom[2];
  double norm = sqrt(Px * Px + Py * Py + Pz * Pz);
  // Move some distance away from <lpos>, which is expected to be sitting on
  // the wall of the SUBVOLUME to be ``extended''.
  const double margin = 10 * dd4hep::um;
  double t            = direction * margin / norm;
  Mx += t * Px;
  My += t * Py;
  Mz += t * Pz;
  double &zIni = Mz, zLow, zUp;
  if (zIni < zT) {
    zLow = zIni;
    zUp  = zT;
  } else {
    zLow = zT;
    zUp  = zIni;
  }
  // Intersection w/ the edge in X,Y
  double tF       = 0;
  double xyLow[2] = {-dX, +dX}, xyUp[2] = {-dY, +dY};
  for (int xy = 0; xy < 2; xy++) {
    int yx       = 1 - xy;
    double a_Low = xyLow[xy], a_Up = xyUp[xy], Pa = Pxy[xy];
    double b_Low = xyLow[yx], b_Up = xyUp[yx], Mb = Mxy[yx], Pb = Pxy[yx];
    for (double A : {a_Low, a_Up}) {
      // Mz+t*Pz = A
      if (Pa) {
        double t = (A - Mz) / Pa;
        if (t * direction < 0)
          continue;
        double Eb = Mb + t * Pb, Ez = Mz + t * Pz;
        if (zLow < Ez && Ez < zUp && b_Low < Eb && Eb < b_Up) {
          if (!status || (status && fabs(t) < fabs(tF))) {
            status |= 0x1;
            tF = t;
          }
        }
      }
    }
  }
  // Else intersection w/ target Z
  if (!status) {
    if (Pz) {
      tF = (zT - Mz) / Pz;
      if (tF * direction > 0)
        status = 0x1;
    }
  }
  if (status) {
    lext[0] = Mx + tF * Px;
    lext[1] = My + tF * Py;
    lext[2] = Mz + tF * Pz;
  }
  return status;
}

double getRef2Cur(DetElement refVol, DetElement curVol) {
  // TGeoHMatrix: In order to avoid a "dangling-reference" warning,
  // let's take a copy of the matrix instead of a reference to it.
  const TGeoHMatrix toRefVol = refVol.nominal().worldTransformation();
  const TGeoHMatrix toCurVol = curVol.nominal().worldTransformation();
  const double* TRef         = toRefVol.GetTranslation();
  const double* TCur         = toCurVol.GetTranslation();
  // For some reason, it has to be "Ref-Cur", while I (Y.B) would have expected the opposite...
  double gdT[3];
  for (int i = 0; i < 3; i++)
    gdT[i] = TRef[i] - TCur[i];
  double ldT[3];
  toRefVol.MasterToLocalVect(gdT, ldT);
  return ldT[2];
}

std::string inconsistency(const edm4hep::EventHeader& event, unsigned int status, CellID cID,
                          const double* lpos, const double* lmom) {
  using edm4eic::unit::GeV, dd4hep::mm;
  return fmt::format("Event {}#{}, SimHit 0x{:016x} @ {:.2f},{:.2f},{:.2f} mm, P = "
                     "{:.2f},{:.2f},{:.2f} GeV inconsistency 0x{:x}",
                     event.getRunNumber(), event.getEventNumber(), cID, lpos[0] / mm, lpos[1] / mm,
                     lpos[2] / mm, lmom[0] / GeV, lmom[1] / GeV, lmom[2] / GeV, status);
}
std::string oddity(const edm4hep::EventHeader& event, unsigned int status, double dist, CellID cID,
                   const double* lpos, const double* lmom, CellID cJD, const double* lpoj,
                   const double* lmoj) {
  using edm4eic::unit::GeV, dd4hep::mm;
  return fmt::format("Event {}#{}, Bizarre SimHit sequence: 0x{:016x} @ {:.4f},{:.4f},{:.4f} mm, P "
                     "= {:.2f},{:.2f},{:.2f} GeV and 0x{:016x} @ {:.4f},{:.4f},{:.4f} mm, P = "
                     "{:.2f},{:.2f},{:.2f} GeV: status 0x{:x}, distance {:.4f}",
                     event.getRunNumber(), event.getEventNumber(), cID, lpos[0] / mm, lpos[1] / mm,
                     lpos[2] / mm, lmom[0] / GeV, lmom[1] / GeV, lmom[2] / GeV, cJD, lpoj[0] / mm,
                     lpoj[1] / mm, lpoj[2] / mm, lmoj[0] / GeV, lmoj[1] / GeV, lmoj[2] / GeV,
                     status, dist);
}

bool MPGDTrackerDigi::samePMO(const edm4hep::SimTrackerHit& sim_hit,
                              const edm4hep::SimTrackerHit& sim_hjt, int unbroken) const {
  // Status:
  // 0: Same Particle, same Module, same Origin
  // 0x1: Not same
  // Particle
  using MCParticle = edm4hep::MCParticle;
#if EDM4HEP_BUILD_VERSION >= EDM4HEP_VERSION(0, 99, 0)
  bool sameParticle = sim_hjt.getParticle() == sim_hit.getParticle();
#else
  bool sameParticle = sim_hjt.getMCParticle() == sim_hit.getMCParticle();
#endif
  // Module
  CellID vID      = sim_hit.getCellID() & m_volumeBits;
  CellID refID    = vID & m_moduleBits; // => the middle slice
  CellID vJD      = sim_hjt.getCellID() & m_volumeBits;
  CellID refJD    = vJD & m_moduleBits; // => the middle slice
  bool sameModule = refJD == refID;
  // Origin
  // Note: edm4hep::SimTrackerHit possesses an "Overlay" quality. Since I don't
  // know what this is, I ignore it.
  bool isSecondary = sim_hit.isProducedBySecondary();
  bool jsSecondary = sim_hjt.isProducedBySecondary();
  bool sameOrigin  = jsSecondary == isSecondary;
  if (isSecondary) { // Secondary subHit
    // A given primary particle can give rise to several secondaries, producing
    // unrelated subHits that we do not want to mix.
    // => Let's impose more condition in that case...
    sameOrigin &= unbroken; // ...Unbroken sequence of subHits
  }
  return sameParticle && sameModule && sameOrigin;
}

double outInDistance(int shape, int orientation, double lintos[][3], double louts[][3],
                     double* lmom, double* lmoj) {
  // Outgoing/incoming distance
  bool ok;
  double lExt[3];
  double lmOI[3];
  for (int i = 0; i < 3; i++)
    lmOI[i] = (lmom[i] + lmoj[i]) / 2;
  double *lOut, *lInto;
  if (orientation > 0) {
    lOut  = louts[1];
    lInto = lintos[0];
  } else if (orientation < 0) {
    lOut  = louts[0];
    lInto = lintos[1];
  } else {
    lOut  = louts[0];
    lInto = lintos[0];
  }
  if (shape == 0) { // "TGeoTubeSeg"
    double rInto = sqrt(lInto[0] * lInto[0] + lInto[1] * lInto[1]);
    ok           = cExtrapolate(lOut, lmOI, rInto, lExt);
  } else { // "TGeoBBox"
    ok = bExtrapolate(lOut, lmOI, lInto[2], lExt);
  }
  if (ok) {
    double dist2 = 0;
    for (int i = 0; i < 3; i++) {
      double d = lExt[i] - lInto[i];
      dist2 += d * d;
    }
    return sqrt(dist2);
  } else
    return -1;
}

unsigned int MPGDTrackerDigi::extendHit(CellID refID, int direction, double* lpini, double* lmini,
                                        double* lpend, double* lmend) const {
  unsigned int status         = 0;
  const VolumeManager& volman = m_detector->volumeManager();
  DetElement refVol           = volman.lookupDetElement(refID);
  const auto& shape           = refVol.solid();
  double *lpoE, *lmoE; // Starting position/momentum
  if (direction < 0) {
    lpoE = lpini;
    lmoE = lmini;
  } else {
    lpoE = lpend;
    lmoE = lmend;
  }
  // Let's test both extremes of the SUBVOLUMES, in order to catch cases where
  // particle re-enters.
  for (int rankE : {0, 4}) {
    CellID vIDE     = refID | m_stripIDs[rankE];
    DetElement volE = volman.lookupDetElement(vIDE);
    double lext[3];
    if (!strcmp(shape.type(), "TGeoTubeSeg")) {
      const Tube& tExt = volE.solid();
      double R         = rankE == 0 ? tExt.rMin() : tExt.rMax();
      double startPhi  = tExt.startPhi() * radian;
      startPhi -= 2 * TMath::Pi();
      double endPhi = tExt.endPhi() * radian;
      endPhi -= 2 * TMath::Pi();
      double dZ = tExt.dZ();
      status    = cExtension(lpoE, lmoE, R, direction, dZ, startPhi, endPhi, lext);
    } else if (!strcmp(shape.type(), "TGeoBBox")) {
      double ref2E    = getRef2Cur(refVol, volE);
      const Box& bExt = volE.solid();
      double Z        = rankE == 0 ? -bExt.z() : +bExt.z();
      Z -= ref2E;
      double dX = bExt.x(), dY = bExt.y();
      status = bExtension(lpoE, lmoE, Z, direction, dX, dY, lext);
    } else
      status = 0x10000;
    if (status & 0xff000) { // Inconsistency => Drop current "sim_hit"
      return status;
    }
    if (status != 0x1)
      continue;
    if (direction < 0) {
      for (int i = 0; i < 3; i++)
        lpini[i] = lext[i];
    } else {
      for (int i = 0; i < 3; i++)
        lpend[i] = lext[i];
    }
    break;
  }
  return status;
}

bool MPGDTrackerDigi::denyExtension(const edm4hep::SimTrackerHit& sim_hit, double depth) const {
  // Non COALESCED secondary: do not extend...
  //  ...if in HELPER SUBVOLUME: if it is TRAVERSING, it's probably
  //   merely because SUBVOLUME is very thin.
  CellID vID          = sim_hit.getCellID() & m_volumeBits;
  bool isHelperVolume = m_stripRank(vID) != 0 && m_stripRank(vID) != 4;
  //  ...else if path length is negligible compared to potential
  //    extension (here, we cannot avoid using a built-in: 10%).
  const double fraction = .10;
  const double edmm = edm4eic::unit::mm, ed2dd = dd4hep::mm / edmm;
  bool smallPathLength = sim_hit.getPathLength() * ed2dd < fraction * depth;
  return isHelperVolume || smallPathLength;
}

void MPGDTrackerDigi::flagUnexpected(const edm4hep::EventHeader& event, int shape, double expected,
                                     const edm4hep::SimTrackerHit& sim_hit, double* lpini,
                                     double* lpend, double* lpos, double* lmom) const {
  // Expectation:
  // - Primary particle: position = middle of overall sensitive volume.
  // - Secondary particle: no diff w.r.t. initial.
  // N.B: At times, when, supposedly, delta ray is created w/in SUBVOLUME, path
  //  of primary does not span the whole volume. The present, simplistic,
  //  version of "flagUnexpected" does flag the case. This, even though
  //  everything is perfectly under control.
  double Rnew2 = 0, Znew, diff2 = 0;
  for (int i = 0; i < 3; i++) {
    double neu = (lpini[i] + lpend[i]) / 2, alt = lpos[i];
    double d = neu - alt;
    diff2 += d * d;
    if (i != 2)
      Rnew2 += neu * neu;
    if (i == 2)
      Znew = neu;
  }
  double found = shape ? Znew : sqrt(Rnew2), residual = found - expected;
  bool isSecondary = sim_hit.isProducedBySecondary();
  bool isPrimary =
      !isSecondary && sqrt(lmom[0] * lmom[0] + lmom[1] * lmom[1] + lmom[2] * lmom[2]) > .1 * GeV;
  if ((fabs(residual) > .000001 && isPrimary) || (sqrt(diff2) > .000001 && isSecondary)) {
    debug("Event {}#{}, SimHit 0x{:016x} origin {:d}: d{:x} = {:.5f} diff = {:.5f}",
          event.getRunNumber(), event.getEventNumber(), sim_hit.getCellID(), shape ? 'Z' : 'R',
          isSecondary, residual, sqrt(diff2));
  }
}

} // namespace eicrecon
