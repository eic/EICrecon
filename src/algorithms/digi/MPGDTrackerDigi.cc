// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov
// https://github.com/eic/EICrecon/blob/6a948d54af18666526ccd5611ad2c818121f290f/src/algorithms/fardetectors/FarDetectorTrackerCluster.cc#L36 
// https://github.com/AIDASoft/DD4hep/issues/1297#issuecomment-2230272779
#include "MPGDTrackerDigi.h"

#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <JANA/JException.h>
// Access "algorithms:GeoSvc"
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <fmt/core.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <gsl/pointers>
#include <unordered_map>
#include <utility>

#include "algorithms/digi/MPGDTrackerDigiConfig.h"

using namespace dd4hep;

namespace eicrecon {

void MPGDTrackerDigi::init() {
    // Create random gauss function
    m_gauss = [&](){
        return m_random.Gaus(0, m_cfg.timeResolution);
        //return m_rng.gaussian<double>(0., m_cfg.timeResolution);
    };

    // Access segmentation decoder
    m_detector         = algorithms::GeoSvc::instance().detector();
    m_cellid_converter = algorithms::GeoSvc::instance().cellIDPositionConverter();
    if (m_cfg.readout.empty()) {
      throw JException("Readout is empty");
    }
    try {
      m_seg    = m_detector->readout(m_cfg.readout).segmentation();
      m_id_dec = m_detector->readout(m_cfg.readout).idSpec().decoder();
      if (!m_cfg.x_field.empty()) {
	m_x_idx = m_id_dec->index(m_cfg.x_field);
	debug("Find layer field {}, index = {}", m_cfg.x_field, m_x_idx);
      }
      if (!m_cfg.y_field.empty()) {
	m_y_idx = m_id_dec->index(m_cfg.y_field);
	debug("Find layer field {}, index = {}", m_cfg.y_field, m_y_idx);
      }
    } catch (...) {
      error("Failed to load ID decoder for {}", m_cfg.readout);
      throw JException("Failed to load ID decoder");
    }
}


void MPGDTrackerDigi::process(
        const MPGDTrackerDigi::Input& input,
        const MPGDTrackerDigi::Output& output) const {

    const auto [sim_hits] = input;
    auto [raw_hits,associations] = output;

    // A map of unique cellIDs with temporary structure RawHit
    std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit> cell_hit_map;
    // Prepare for strip segmentation
    const Position dummy(0,0,0);
    auto volman = m_detector->volumeManager();

    using CellIDs = std::pair<CellID,CellID>;
    using Sim2IDs = std::vector<CellIDs>; Sim2IDs sim2IDs;
    for (const edm4hep::SimTrackerHit& sim_hit : *sim_hits) {

        // time smearing
        double time_smearing = m_gauss();
        double result_time = sim_hit.getTime() + time_smearing;
        auto hit_time_stamp = (std::int32_t) (result_time * 1e3);

	// Segmentation: Simulate the strip, two-coordinate, readout of MPGDs.
	// - Overwrite and extend segmentation stored in "sim_hit" (which is
	//  anyway expected to be along a single coordinate).
	// - New segmentation is along the two coordinates, described by two
	//  volumeID's with each a distinctive "sensor" field.
	//  N.B.: Assumptions on the <id> bit-pattern:
	//   - volumeID: least-significant 32 bits, hitID: most-significant 32.
	//   - The bit-field defining "sensor" is fixed = cellID>>24&0x3.
	// - 1st segmentation, corresponding to a volumeID w/ a "sensor" of
	//  the input "sim_hits" parity, is re-evaluated => provides for
	//  reconstructing given simulation data w/ different segmentations.
	// - 2nd volumeID is derived from that by flipping sensorID ^= 0x1.
	// - To get the new cellID's, we need the local position.
	const edm4hep::Vector3d &pos = sim_hit.getPosition();
	Position gpos(pos.x/10,pos.y/10,pos.z/10);
	auto volman = m_detector->volumeManager();
	CellID cID = sim_hit.getCellID(), vID =  cID&0xffffffff;
	DetElement local = volman.lookupDetElement(vID);
	const auto lpos = local.nominal().worldToLocal(gpos);
	// p "sensor", w/ strip-discriminator as is from "sim_hit"
	CellID cIDp = m_seg->cellID(lpos,dummy,vID);
	if(level() >= algorithms::LogLevel::kDebug) {
	  CellID hIDp = cIDp>>32, sIDp = cIDp>>32&0x3, vIDp = cIDp&0xffffffff;
	  debug("--------------------");
	  debug("Hit cellIDp  = 0x{:08x}, 0x{:09x} 0x{:02x}", hIDp, vIDp, sIDp);
#define MPGDDigi_DEBUG
#ifdef MPGDDigi_DEBUG
	  // Let's check that we recover the cellID stored in "sim_hit" starting
	  // from the position of "sim_hit". Assuming...
	  // ...the 32 bits of the hitID field are subdivided into 2 bits for
	  //   the strip-discriminator field and the remaining 30 bits are
	  //   equally shared by the two coordinates,
	  // ...segmentations @ reconstruction and simulation time are identical.
	  CellID hID =  cID>>32,  sID = vID>>32&0x3;
	  debug("Hit cellID   = 0x{:08x}, 0x{:09x} 0x{:02x}", hID,  vID,  sID);
	  CellID xid = hID&0x7fff, xidp = hIDp&0x7fff;
	  if (xid!=xidp)
	    printf("** MPGDTrackerDigi: Strip segmentation inconsistency: m_seg(0x%lx) != sim_hit(0x%lx)\n",xidp,xid);
#endif
	}
	// n "sensor", w/ the odd parity bit of strip-discriminator set. It's expected to have strips along the 2nd coordinate.
	CellID vIDn = vID|0x100000000;
	CellID cIDn = m_seg->cellID(lpos,dummy,vIDn);
	sim2IDs.push_back({cIDp,cIDn}); // Remember cellIDs. 

	if(level() >= algorithms::LogLevel::kDebug) {
	  CellID hIDn = cIDn>>32, sIDn = cIDn>>32&0x3;
	  debug("Hit cellIDn  = 0x{:08x}, 0x{:09x} 0x{:02x}", hIDn, vIDn, sIDn);
	  debug("   position  = ({:.2f}, {:.2f}, {:.2f})", sim_hit.getPosition().x, sim_hit.getPosition().y, sim_hit.getPosition().z);
	  debug("   xy_radius = {:.2f}", std::hypot(sim_hit.getPosition().x, sim_hit.getPosition().y));
	  debug("   momentum  = ({:.2f}, {:.2f}, {:.2f})", sim_hit.getMomentum().x, sim_hit.getMomentum().y, sim_hit.getMomentum().z);
	  debug("   edep = {:.2f}", sim_hit.getEDep());
	  debug("   time = {:.4f}[ns]", sim_hit.getTime());
	  debug("   particle time = {}[ns]", sim_hit.getMCParticle().getTime());
	  debug("   time smearing: {:.4f}, resulting time = {:.4f} [ns]", time_smearing, result_time);
	  debug("   hit_time_stamp: {} [~ps]", hit_time_stamp);
	}
#define MPGDDigi_DEBUG
#ifdef MPGDDigi_DEBUG
	// Check cellID -> position
	for (CellID cID : {cIDp,cIDn}) {
	  dd4hep::DDSegmentation::Vector3D lpos = m_seg->position(cID);
	  double X = lpos.X, Y = lpos.Y, Z = lpos.Z;
	  double phi = atan2(Y,X), R = sqrt(X*X+Y*Y);
	  CellID hID = cID>>32, vID = cID&0xffffffff, sID = cID>>32&0x3;
	  CellID modID = cID>>12&0xfff, phiID = hID>>2&0x7fff, ZID = hID>>17;
	  printf("0x%08lx(0x%03lx) 0x%08lx(0x%04lx,0x%04lx) 0x%02lx: %.3f,%.3f,%.3f cm %.3f rad %.3f cm\n",
		 vID,modID,hID,phiID,ZID,sID,lpos.X/cm,lpos.Y/cm,lpos.Z/cm,phi,R/cm);
	}
#endif

        if (sim_hit.getEDep() < m_cfg.threshold) {
            debug("  edep is below threshold of {:.2f} [keV]", m_cfg.threshold / keV);
            continue;
        }

	for (CellID cID : {cIDp,cIDn}) {
	//for (CellID cID : {cIDp}) {
	    if (cell_hit_map.count(cID) == 0) {
		// This cell doesn't have hits
		cell_hit_map[cID] = {
		    cID,
		    (std::int32_t) std::llround(sim_hit.getEDep() * 1e6),
		    hit_time_stamp  // ns->ps
		};
	    } else {
		// There is previous values in the cell
		auto& hit = cell_hit_map[cID];
		debug("  Hit already exists in cell ID={}, prev. hit time: {}", cID, hit.getTimeStamp());

		// keep earliest time for hit
		auto time_stamp = hit.getTimeStamp();
		hit.setTimeStamp(std::min(hit_time_stamp, hit.getTimeStamp()));

		// sum deposited energy
		auto charge = hit.getCharge();
		hit.setCharge(charge + (std::int32_t) std::llround(sim_hit.getEDep() * 1e6));
	    }
        }
    }

    for (auto item : cell_hit_map) {
        raw_hits->push_back(item.second);	
	Sim2IDs::const_iterator sim_it = sim2IDs.cbegin();
        for (const auto& sim_hit : *sim_hits) {
	    CellIDs cIDs = *sim_it++;
	    for (CellID cID : {cIDs.first,cIDs.second}) {
	      //for (CellID cID : {cIDs.first}) {
		if (item.first == cID) {
		    // set association
		    auto hitassoc = associations->create();
		    hitassoc.setWeight(1.0);
		    hitassoc.setRawHit(item.second);
#if EDM4EIC_VERSION_MAJOR >= 6
		    hitassoc.setSimHit(sim_hit);
#else
		    hitassoc.addToSimHits(sim_hit);
#endif
		}
	    }
        }

    }
}

} // namespace eicrecon
