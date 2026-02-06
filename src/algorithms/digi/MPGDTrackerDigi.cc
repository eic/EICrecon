// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov, Yann Bedfer

/*
  Digitization specific to MPGDs.
  - What's special in MPGDs is their 2D-strip readout: i.e. simultaneous
   registering along two sets of coordinate strips.
  - The "process" method involves a combination of segmentation, simulation and
   digitization.
  - The segmentation done when producing "sim_hits", and stored as a pixel
   cellID, is overwritten by strip cellIDs, via "dd4hep::MultiSegmentation".
  - The simulation will eventually involve simulating the amplitude and timing
   correlation of the two coordinates, and the spreading of the charge on
   adjacent strips producing multi-hit clusters.
    The preliminary version is rudimentary: single-hit clusters, with identical
   timing and uncorrelated amplitudes.
  - The digitization follows the standard steps, only that it needs to be
   convoluted with the simulation.
  NOTA BENE: The file could be simplified, see issue #1702.
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
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <Parsers/Primitives.h>
// Access "algorithms:GeoSvc"
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <fmt/format.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <gsl/pointers>
#include <initializer_list>
#include <iterator>
#include <random>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/digi/MPGDTrackerDigiConfig.h"

using namespace dd4hep;

namespace eicrecon {

void MPGDTrackerDigi::init() {
  // Access id decoder
  m_detector                            = algorithms::GeoSvc::instance().detector();
  const dd4hep::BitFieldCoder* m_id_dec = nullptr;
  if (m_cfg.readout.empty()) {
    throw std::runtime_error("Readout is empty");
  }
  try {
    m_seg    = m_detector->readout(m_cfg.readout).segmentation();
    m_id_dec = m_detector->readout(m_cfg.readout).idSpec().decoder();
  } catch (...) {
    critical("Failed to load ID decoder for \"{}\" readout.", m_cfg.readout);
    throw std::runtime_error(
        fmt::format("Failed to load ID decoder for \"{}\" readout.", m_cfg.readout));
  }
  // Method "process" relies on a strict assumption on the IDDescriptor:
  // - Must have a "strip" field.
  // - That "strip" field includes bits 30|31.
  // Let's check.
  if (m_id_dec->get(((CellID)0x3) << 30, "strip") != 0x3) {
    critical(R"(Missing or invalid "strip" field in IDDescriptor for "{}"
        readout.)",
             m_cfg.readout);
    throw std::runtime_error(
        fmt::format("Invalid IDDescriptor for \"{}\" readout.", m_cfg.readout));
  }
  debug(R"(Find valid "strip" field in IDDescriptor for "{}" readout.)", m_cfg.readout);
}

void MPGDTrackerDigi::process(const MPGDTrackerDigi::Input& input,
                              const MPGDTrackerDigi::Output& output) const {

  // ********** SIMULATE THE 2D-strip READOUT of MPGDs.
  // - Overwrite and extend segmentation stored in "sim_hit", which is anyway
  //  expected to be along a single coordinate (this happens to allow one to
  //  reconstruct data w/ a segmentation differing from that used when
  //  generating the data).
  // - New segmentation is along two coordinates, described by two cellID's
  //  with each a distinctive "strip" field.
  //   N.B.: Assumptions on the IDDescriptor: the "strip" specification
  //  is fixed = cellID>>32&0x3.
  // - The simulation is simplistic: single-hit cluster per coordinate.

  const auto [headers, sim_hits] = input;
  auto [raw_hits, associations]  = output;

  // local random generator
  auto seed = m_uid.getUniqueID(*headers, name());
  std::default_random_engine generator(seed);
  std::normal_distribution<double> gaussian;

  // A map of unique cellIDs with temporary structure RawHit
  std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit> cell_hit_map;
  // Prepare for strip segmentation
  const Position dummy(0, 0, 0);
  const VolumeManager& volman = m_detector->volumeManager();

  using CellIDs = std::pair<CellID, CellID>;
  using Sim2IDs = std::vector<CellIDs>;
  Sim2IDs sim2IDs;
  for (const edm4hep::SimTrackerHit& sim_hit : *sim_hits) {

    // ***** TIME SMEARING
    // - Simplistic treatment.
    // - A more realistic one would have to distinguish a smearing common to
    //  both coordinates of the 2D-strip readout (due to the drifting of the
    //  leading primary electrons) from other smearing effects, specific to
    //  each coordinate.
    double time_smearing = gaussian(generator) * m_cfg.timeResolution;
    double result_time   = sim_hit.getTime() + time_smearing;
    auto hit_time_stamp  = (std::int32_t)(result_time * 1e3);

    // ***** SEGMENTATION
    // - The two cellID's are encoded via a "dd4hep::MultiSegmentation"
    //  discriminating on the strip field, w/ "strip" setting of 0x1 (
    //  called 'p') and 0x2 (called 'n').
    // - They are evaluated based on "sim_hit" Cartesian coordinates
    //  positions
    //   Given that all the segmentation classes foreseen for MPGDs (
    //  "CartesianGrid.." for Outer and EndCaps, "CylindricalGridPhiZ" for
    //  "CyMBaL") disregard the _global_ position argument to
    //  "dd4hep::Segmentation::cellID", we need the _local_ position and
    //  only that.
    const edm4hep::Vector3d& pos = sim_hit.getPosition();
    using dd4hep::mm;
    Position gpos(pos.x * mm, pos.y * mm, pos.z * mm);
    CellID vID = // Note: Only the bits corresponding to the volumeID will
        // be used. The rest, encoding the segmentation stored in "sim_hit",
        // being disregared.
        sim_hit.getCellID();
    DetElement local = volman.lookupDetElement(vID);
    const auto lpos  = local.nominal().worldToLocal(gpos);
    // p "strip"
    CellID stripBitp = ((CellID)0x1) << 30;
    CellID vIDp      = vID | stripBitp;
    CellID cIDp      = m_seg->cellID(lpos, dummy, vIDp);
    // n "strip"
    CellID stripBitn = ((CellID)0x2) << 30;
    CellID vIDn      = vID | stripBitn;
    CellID cIDn      = m_seg->cellID(lpos, dummy, vIDn);

    sim2IDs.emplace_back(cIDp, cIDn); // Remember cellIDs.
    // ***** DEBUGGING INFO
    if (level() >= algorithms::LogLevel::kDebug) {
      CellID hIDp = cIDp >> 32;
      CellID sIDp = cIDp >> 30 & 0x3;
      debug("--------------------");
      debug("Hit cellIDp  = 0x{:08x}, 0x{:08x} 0x{:02x}", hIDp, vIDp, sIDp);
      CellID hIDn = cIDn >> 32;
      CellID sIDn = cIDn >> 30 & 0x3;
      debug("Hit cellIDn  = 0x{:08x}, 0x{:08x} 0x{:02x}", hIDn, vIDn, sIDn);
      debug("   position  = ({:.2f}, {:.2f}, {:.2f})", sim_hit.getPosition().x,
            sim_hit.getPosition().y, sim_hit.getPosition().z);
      debug("   xy_radius = {:.2f}", std::hypot(sim_hit.getPosition().x, sim_hit.getPosition().y));
      debug("   momentum  = ({:.2f}, {:.2f}, {:.2f})", sim_hit.getMomentum().x,
            sim_hit.getMomentum().y, sim_hit.getMomentum().z);
      debug("   edep = {:.2f}", sim_hit.getEDep());
      debug("   time = {:.4f}[ns]", sim_hit.getTime());
      debug("   particle time = {}[ns]", sim_hit.getParticle().getTime());
      debug("   time smearing: {:.4f}, resulting time = {:.4f} [ns]", time_smearing, result_time);
      debug("   hit_time_stamp: {} [~ps]", hit_time_stamp);
    }

    // ***** APPLY THRESHOLD
    if (sim_hit.getEDep() < m_cfg.threshold) {
      debug("  edep is below threshold of {:.2f} [keV]", m_cfg.threshold / keV);
      continue;
    }

    // ***** HIT ACCUMULATION
    for (CellID cID : {cIDp, cIDn}) {
      if (!cell_hit_map.contains(cID)) {
        // This cell doesn't have hits
        cell_hit_map[cID] = {
            cID, (std::int32_t)std::llround(sim_hit.getEDep() * 1e6),
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
        hit.setCharge(charge + (std::int32_t)std::llround(sim_hit.getEDep() * 1e6));
      }
    }
  }

  // ***** raw_hit INSTANTIATION AND raw<-sim_hit's ASSOCIATION
  for (const auto& [cell_id, raw_hit] : cell_hit_map) {
    raw_hits->push_back(raw_hit);
    auto sim_it = sim2IDs.cbegin();
    for (const auto& sim_hit : *sim_hits) {
      CellIDs cIDs = *sim_it++;
      for (CellID cID : {cIDs.first, cIDs.second}) {
        if (cell_id == cID) {
          // set association
          auto hitassoc = associations->create();
          hitassoc.setWeight(1.0);
          hitassoc.setRawHit(raw_hit);
          hitassoc.setSimHit(sim_hit);
        }
      }
    }
  }
}

} // namespace eicrecon
