// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#include "MPGDHitReconstruction.h"

#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <algorithms/logger.h>
#include <edm4eic/CovDiag3f.h>

#include <edm4eic/EDM4eicVersion.h>

#include <edm4hep/Vector3f.h>
#include <JANA/JException.h>
// Access "algorithms:GeoSvc"
#include <algorithms/geo.h>
#include <fmt/core.h>
#include <cstddef>
#include <iterator>
#include <vector>
#include <algorithm>

using namespace dd4hep;

namespace eicrecon {

void MPGDHitReconstruction::init() {

  // ***** Parse IDDescriptor
  // Access id decoder
  const dd4hep::Detector* detector = m_geo.detector();
  if (m_cfg.readout.empty()) {
    throw std::runtime_error("Readout is empty");
  }
  try {
    m_id_dec = detector->readout(m_cfg.readout).idSpec().decoder();
  } catch (...) {
    critical(R"(Failed to load ID decoder for "{}" readout.)", m_cfg.readout);
    throw std::runtime_error("Failed to load ID decoder");
  }
  parseIDDescriptor();
}

void MPGDHitReconstruction::process(const Input& input, const Output& output) const {
  using dd4hep::mm;

  const auto [raw_hits] = input;
  auto [rec_hits]       = output;

  // Reorder input raw_hits
  // Sort them in ascending order of channel# on a per SUBVOLUME * (p|n) basis.
  int nRawHits = raw_hits->size();
  std::vector<size_t> idcs(nRawHits);
  size_t idx(0);
  std::generate(idcs.begin(), idcs.end(), [&] { return idx++; });
  std::sort(idcs.begin(), idcs.end(), [&](int idxa, int idxb) {
    CellID cIDa = raw_hits->at(idxa).getCellID(), vIDa = cIDa & m_subVolBits;
    CellID cIDb = raw_hits->at(idxb).getCellID(), vIDb = cIDb & m_subVolBits;
    if (vIDa != vIDb)
      return vIDa < vIDb;
    int pna = (vIDa & m_pStripBit) ? 0 : 1, pnb = (vIDb & m_pStripBit) ? 0 : 1;
    if (pna != pnb)
      return pna < pnb;
    std::int16_t hIDa = pna == 0 ? cIDa >> m_coordOffsets[0] : cIDa >> m_coordOffsets[1];
    std::int16_t hIDb = pnb == 0 ? cIDb >> m_coordOffsets[0] : cIDb >> m_coordOffsets[1];
    return hIDa < hIDb;
  });

  CellID prvID; // CellID of previous
  // Current cluster (cc): in the making or to be stored
  int currentPN;
  size_t currentNDims;
  Position clusPos; // cc: weighted position
  std::vector<double> clusDim(
      3); // cc: uncertainty = resolution along measurement axis, else weighted dimension
  double clusCharge(0); // cc: sum of charges
  double sW(0);         // cc: sum of Weights (= "clusCharge", as of 2025/12)
  double clusChMx(0);
  int clusChMxIdx(-1);  // cc: Max. charge and index of: determine timing
  int clusFirstIdx(-1); // cc: Index of 1st RawHit contributing
  // Loop on ordered raw_hits + a last iteration to store last cluster
  int jdx;
  for (jdx = 0, prvID = 0, currentPN = -1, sW = 0; jdx <= nRawHits; jdx++) {
    bool lastHit = jdx == nRawHits;
    bool newCluster;
    CellID cID;
    int idx;
    if (!lastHit) {
      idx                 = idcs[jdx];
      const auto& raw_hit = raw_hits->at(idx);
      cID                 = raw_hit.getCellID();
      if ((cID & m_subVolBits) != (prvID & m_subVolBits) ||
          (cID & m_pStripBit) != (prvID & m_pStripBit)) {
        newCluster = true;
      } else {
        // Cluster continuation? Require channel#+1
        std::int16_t prvHID, hID;
        if (cID & m_pStripBit) {
          prvHID = prvID >> m_coordOffsets[0];
          hID    = cID >> m_coordOffsets[0];
        } else {
          prvHID = prvID >> m_coordOffsets[1];
          hID    = cID >> m_coordOffsets[1];
        }
        newCluster = hID != prvHID + 1;
      }
      if (prvID && !newCluster) {
        // ***** ADD HIT TO CURRENT CLUSTER
        double weight = raw_hit.getCharge();
        clusPos += m_converter->position(cID) * weight;
        sW += weight;
        std::vector<double> dim = m_converter->cellDimensions(cID);
        size_t nDims            = std::size(dim);
        for (size_t i = 0; i < nDims; i++)
          clusDim[i] += dim[i] * weight;
        clusCharge += weight;
        if (weight > clusChMx) {
          clusChMxIdx = idx;
          clusChMx    = weight;
        }
        if (idx < clusFirstIdx) {
          clusFirstIdx = idx;
        }
        if (nDims != currentNDims) {
          critical(
              R"(RawHits from "{}" readout have different #dims: cellID 0x{:0>16x} = %u, cellID 0x{:0>16x} = %u.)",
              m_cfg.readout, prvID, currentNDims, cID, nDims);
          throw std::runtime_error("Inconsistent RawHits");
        }
        prvID = cID;
        continue; // ***** TRY AND ADD YET ANOTHER HIT
      }
    } else
      newCluster = false;
    if (prvID) {
      // ***** STORE (and log) CURRENT CLUSTER
      // Timing = timing of max. charge.
      const auto& clusHit = raw_hits->at(clusChMxIdx);
      double clusTime     = clusHit.getTimeStamp();
      // ID = ID of max. charge by convention; it's not very meaningful in fact.
      CellID clusID = clusHit.getCellID();
      // Averaging
      clusPos /= sW * mm;
      // Variance
      for (int i = 0; i < currentNDims; i++) {
        if (i == currentPN) { // Measurement axis
          clusDim[i] = m_cfg.stripResolutions[currentPN];
        } else {
          constexpr const double sqrt_12 = 3.4641016151;
          clusDim[i] /= sW * sqrt_12;
        }
        clusDim[i] /= mm;
        clusDim[i] *= clusDim[i];
      }
      // >oO trace
      if (level() == algorithms::LogLevel::kDebug) {
        debug("cellID = 0x{:08x}, 0x{:08x}", clusID >> 32, (std::uint32_t)clusID);
        debug("position x={:.2f} y={:.2f} z={:.2f} [mm]: ", clusPos.x(), clusPos.y(), clusPos.z());
        debug("dimension size: {}", currentNDims);
        for (size_t j = 0; j < currentNDims; ++j) {
          debug(" - dimension {:<5} size: {:.2}", j, clusDim[j]);
        }
      }

      // Note about variance:
      //    The variance is used to obtain a diagonal covariance matrix.
      //    Note that the covariance matrix is written in DD4hep surface coordinates,
      //    *NOT* global position coordinates. This implies that:
      //      - XY segmentation: xx -> sigma_x, yy-> sigma_y, zz -> 0, tt -> 0
      //      - XZ segmentation: xx -> sigma_x, yy-> sigma_z, zz -> 0, tt -> 0
      //      - XYZ segmentation: xx -> sigma_x, yy-> sigma_y, zz -> sigma_z, tt -> 0
      //    This is properly in line with how we get the local coordinates for the hit
      //    in the TrackerSourceLinker.
      // CartesianGridUV:
      // - Covariance matrix is along U and V. I(Y.B.) don't know whether this
      //  is what is meant by "DD4hep surface coordinates" above. And don't
      //  know whether it's in line w/ TrackerSourceLinker.
      // - But I do know that the matrix has to be rotated along X,Y (i.e.
      //  the reference frame local to the DD4hep volume) lest hit-to-track
      //  association fail completely. At present (2025/12), this is done in
      //  TrackerMeasurementFromHits. It cannot be done here, since it would
      //  result in a non-diagonal matrix (and a violently non-diagonal one:
      //  precision along one of U or V being very large for a strip hit).
      auto rec_hit = rec_hits->create(
          clusID, // Raw DD4hep cell ID of largest cluster's hit.
          edm4hep::Vector3f{static_cast<float>(clusPos.x()), static_cast<float>(clusPos.y()),
                            static_cast<float>(clusPos.z())}, // mm
          edm4eic::CovDiag3f{clusDim[0],
                             clusDim[1], // variance (see note above)
                             currentNDims > 2 ? clusDim[2] : 0.},
          static_cast<float>(clusTime / 1000.0),  // ns
          m_cfg.timeResolution,                   // in ns
          static_cast<float>(clusCharge / 1.0e6), // Collected energy (GeV)
          0.0F);                                  // Error on the energy
      // ********** REC <- RAW ASSOCIATION
      // - In EDM4eic, there's room for ONLY ONE ASSOCIATED RAW.
      // - Here we NEED MORE.
      // - Temporarily, simply put the earliest one. So that subsequent RawHits
      //  that should also be associated can easily be guessed.
      // - But imho(Y.B), "EDM4eic::TrackerHit" should be modified.
      const auto& firstHit = raw_hits->at(clusFirstIdx);
      rec_hit.setRawHit(firstHit);
    }
    if (newCluster) {
      const auto& raw_hit     = raw_hits->at(idx);
      double weight           = raw_hit.getCharge();
      clusPos                 = m_converter->position(cID) * weight;
      sW                      = weight;
      std::vector<double> dim = m_converter->cellDimensions(cID);
      size_t nDims            = std::size(dim);
      for (size_t i = 0; i < nDims; i++)
        clusDim[i] = dim[i] * weight;
      clusCharge   = weight;
      clusChMx     = weight;
      clusChMxIdx  = idx;
      currentPN    = (cID & m_pStripBit) ? 0 : 1;
      currentNDims = nDims;
      prvID        = cID;
      clusFirstIdx = idx;
    }
  }
}
void MPGDHitReconstruction::parseIDDescriptor() {
  // Parse IDDescriptor: Retrieve cellIDs of relevant fields.
  // (As an illustration, here is the IDDescriptor of CyMBaL (as of 2025/12):
  // <id>system:8,layer:4,module:12,sensor:2,strip:28:4,phi:-16,z:-16</id>.)

  // - "m_(p|n)stripBit".
  debug(R"(Parsing IDDescriptor for "{}" readout)", m_cfg.readout);
  const char fieldName[] = "strip";
  CellID stripBits[2]    = {0, 0};
  FieldID fieldID        = 0;
  try {
    fieldID = m_id_dec->get(~((CellID)0x0), fieldName);
  } catch (const std::runtime_error& error) {
    critical(R"(No field "{}" in IDDescriptor of readout "{}".)", fieldName, m_cfg.readout);
    throw std::runtime_error("Invalid IDDescriptor");
  }
  const BitFieldElement& fieldElement = (*m_id_dec)[fieldName];
  // Coordinates are assigned specific bits by convention
  FieldID bits[2] = {0x1, 0x2};
  for (int coord = 0; coord < 2; coord++) {
    stripBits[coord] = bits[coord] << fieldElement.offset();
  }
  // CellIDs derived from above
  m_pStripBit = stripBits[0];
  m_nStripBit = stripBits[1];
  // Require coordinate fields to start @ bits 32 and 48: this ensures that
  // these fields fit into std::int16_t and is taken advantage of by debug
  // messages to cleanly separate coordinates from the rest of CellID.
  m_coordOffsets[0] = m_coordOffsets[1] = 64;
  for (int pn = 0; pn < 2; pn++) {
    std::string coordName;
    if (m_cfg.readout == "MPGDBarrelHits") {
      coordName = pn ? "z" : "phi";
    } else if (m_cfg.readout == "OuterMPGDBarrelHits") {
      coordName = pn ? "v" : "u";
    } else {
      coordName = pn ? "y" : "x";
    }
    try {
      m_id_dec->index(coordName);
    } catch (const std::runtime_error& error) {
      critical(R"(No "{}" field in IDDescriptor of readout "{}")", coordName, m_cfg.readout);
      throw std::runtime_error("Invalid IDDescriptor");
    }
    const BitFieldElement& fieldElement = (*m_id_dec)[coordName];
    int offset                          = fieldElement.offset();
    if (offset < m_coordOffsets[pn])
      m_coordOffsets[pn] = offset;
  }
  if (m_coordOffsets[0] != 32 || m_coordOffsets[1] != 48) {
    critical(R"(Coordinate fields in IDDescriptor of readout "{}" do not start @ bits 32 and 48.)",
             m_cfg.readout);
    throw std::runtime_error("Invalid IDDescriptor");
  }
  for (int i = 0; i < 32; i++)
    // Now that offsets have been checked, we can safely define "m_subVolBits"
    // to span [0,32[.
    m_subVolBits |= ((CellID)0x1) << i;
}

} // namespace eicrecon
