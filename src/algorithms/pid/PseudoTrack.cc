// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "PseudoTrack.h"

// AlgorithmInit
//---------------------------------------------------------------------------
void eicrecon::PseudoTrack::AlgorithmInit(
    std::function<bool(double,double,double)> within_radiator,
    std::shared_ptr<spdlog::logger>& logger
    )
{
  m_log             = logger;
  m_within_radiator = within_radiator;
  m_cfg.Print(m_log, spdlog::level::debug);
}


// AlgorithmChangeRun
//---------------------------------------------------------------------------
void eicrecon::PseudoTrack::AlgorithmChangeRun() {
}


// AlgorithmProcess
//---------------------------------------------------------------------------
std::vector<edm4eic::TrackSegment*> eicrecon::PseudoTrack::AlgorithmProcess(
    std::vector<const edm4hep::SimTrackerHit*>& in_hits
    )
{
  // logging
  m_log->trace("{:=^70}"," call PseudoTrack::AlgorithmProcess ");
  m_log->trace("number of raw sensor hits: {}", in_hits.size());

  // start output collections
  std::vector<edm4eic::TrackSegment*> out_tracks;

  // FIXME: so far this algorithm only works for single-track events!
  edm4eic::MutableTrackSegment out_track;

  // get photon emission points (vertices)
  std::vector<decltype(edm4hep::MCParticleData::vertex)> phot_vertices;
  for(const auto& in_hit : in_hits) {
    // check if it is an opticalphoton
    auto phot = in_hit->getMCParticle();
    if(phot.getPDG() != -22) {
      m_log->warn("non-opticalphoton hit: PDG = {}",phot.getPDG());
      continue;
    }
    // include this vertex only if within the radiator
    auto vtx = phot.getVertex();
    if(m_within_radiator(vtx.x, vtx.y, vtx.z))
      phot_vertices.push_back(vtx);
  }

  // sort the vertices by z-coordinate
  // FIXME: good for dRICH only; pfRICH will have opposite sign, and other detectors will need more generality
  std::sort(phot_vertices.begin(), phot_vertices.end(), [] (auto& a, auto& b) { return a.z < b.z; });

  // logging
  m_log->trace("Photon emission points (sorted):");
  for(int i=0; i<phot_vertices.size(); i++)
    m_log->trace("  {:>5}: x=( {:>10.2f} {:>10.2f} {:>10.2f} )",
        i, phot_vertices[i].x, phot_vertices[i].y, phot_vertices[i].z);

  // build psuedo-tracks from photon emission point "pins"
  m_log->trace("Pseudo-track pin points:");
  auto AddPin = [this,&out_track] (auto pin_pos) {
    edm4eic::TrackPoint pin;
    pin.position = {
      static_cast<float>(pin_pos.x),
      static_cast<float>(pin_pos.y),
      static_cast<float>(pin_pos.z)
    };
    m_log->trace("  {:>5}: x=( {:>10.2f} {:>10.2f} {:>10.2f} )",
        out_track.points_size(), pin.position.x, pin.position.y, pin.position.z);
    out_track.addToPoints(pin);
  };
  // use all photon vertices, if `m_cfg.numPoints<=0` or if there are less photons than `m_cfg.numPoints`
  if(m_cfg.numPoints<=0 || m_cfg.numPoints>=phot_vertices.size()) {
    for(auto vtx : phot_vertices)
      AddPin(vtx);
  }
  // otherwise take only `m_cfg.numPoints` of them, roughly equally spaced
  else {
    auto step  = static_cast<unsigned>(phot_vertices.size() / (m_cfg.numPoints-1));
    for(int p=0; p<m_cfg.numPoints; p++) {
      auto s = p*step;
      if(s==phot_vertices.size()) s-=1;
      AddPin(phot_vertices[s]);
    }
  }

  // append
  out_tracks.push_back(new edm4eic::TrackSegment(out_track)); // force immutable

  // cleanup
  phot_vertices.clear();

  // return
  return out_tracks;
}
