// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 EIC-FT
//
//  RandomNoise.cc
//
//  Adds synthetic electronic noise to a RawTrackerHit collection.
//
//  The file contains
//      • process()                – main event loop
//      • geometry helpers         – ScanDetectorElement / ScanComponent
//      • inject_noise_hits()      – creates the actual noise hits
//
// SPDX-License-Identifier: LGPL-3.0-or-later
// RandomNoise.cc  –  inject synthetic electronic noise hits
// ------------------------------------------------------------------

#include "RandomNoise.h"
#include <edm4hep/EventHeader.h>

/* DD4hep / segmentation ------------------------------------------------ */
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <DDSegmentation/CartesianGrid.h>
#include <DDSegmentation/CylindricalSegmentation.h>
#include <DDSegmentation/PolarGrid.h>
#include <DDSegmentation/CylindricalGridPhiZ.h>
#include <DDSegmentation/PolarGridRPhi2.h>
#include <DDSegmentation/PolarGridRPhi.h>
#include <TGeoBBox.h>
#include <TGeoNode.h>
#include <algorithms/geo.h>

/* STL / helpers -------------------------------------------------------- */
#include <algorithm>
#include <cmath>
#include <iostream>
#include <optional>
#include <random>
#include <unordered_set>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <set>

namespace eicrecon {

// ======================================================================
//  0.  Helper scanners (Cartesian / Cylindrical / Polar / Generic)
// ======================================================================
namespace {

  template <typename SEG>
  void scanCartesian(const SEG& seg, const TGeoBBox* box, std::set<dd4hep::CellID>& unique) {
    const double DX = box->GetDX(), DY = box->GetDY(), DZ = box->GetDZ();
    double step = 1.0;
    for (double d : seg.cellDimensions(0))
      if (d > 1e-9)
        step = std::min(step, d);

    auto nStep = [&](double L, double c) -> unsigned {
      return std::max<unsigned>(2, std::min<unsigned>(60, std::ceil(L / c)));
    };
    const double eps = 1e-9;
    unsigned nx = nStep(2 * DX, step), ny = nStep(2 * DY, step), nz = nStep(2 * DZ, step);

    for (unsigned ix = 0; ix < nx; ++ix) {
      double x = (-DX + eps) + ix * (2 * DX - 2 * eps) / (nx - 1);
      for (unsigned iy = 0; iy < ny; ++iy) {
        double y = (-DY + eps) + iy * (2 * DY - 2 * eps) / (ny - 1);
        for (unsigned iz = 0; iz < nz; ++iz) {
          double z = (-DZ + eps) + iz * (2 * DZ - 2 * eps) / (nz - 1);
          unique.insert(seg.cellID({x, y, z}, {x, y, z}, 0));
        }
      }
    }
  }

  template <typename SEG>
  void scanCylindrical(const SEG& seg, const TGeoBBox* box, std::set<dd4hep::CellID>& unique) {
    /* 최대 반지름을 바운딩 박스에서 추정 */
    const double R   = std::min(box->GetDX(), box->GetDY()) - 1e-3;
    const double DZ  = box->GetDZ();
    const double eps = 1e-9;

    /* cellDimensions(0) → {R*dPhi, dZ} */
    double dPhi = 0.05; // default 0.05 rad (~3°)
    double dZ   = 1.0;  // default 1 mm
    auto dims   = seg.cellDimensions(0);
    if (dims.size() >= 1 && dims[0] > 1e-9)
      dPhi = dims[0] / R;
    if (dims.size() >= 2 && dims[1] > 1e-9)
      dZ = dims[1];

    unsigned nPhi = std::max<unsigned>(6, std::ceil(2 * M_PI / dPhi));
    unsigned nZ   = std::max<unsigned>(2, std::ceil(2 * DZ / dZ));

    for (unsigned ip = 0; ip < nPhi; ++ip) {
      double phi = (ip + 0.5) * 2 * M_PI / nPhi;
      double x   = R * std::cos(phi);
      double y   = R * std::sin(phi);
      for (unsigned iz = 0; iz < nZ; ++iz) {
        double z = (-DZ + eps) + (iz + 0.5) * (2 * DZ - 2 * eps) / nZ;
        unique.insert(seg.cellID({x, y, z}, {x, y, z}, 0));
      }
    }
  }

  /* ------------------------------------------------------------------ */
  /* PolarGridRPhi / PolarGridRPhi2  –  R-dependent Δφ                  */
  /* ------------------------------------------------------------------ */
  template <typename POLAR_RPHI>
  void scanPolarRPhi(const POLAR_RPHI& seg, std::set<dd4hep::CellID>& unique) {
    const auto& rBins    = seg.gridRValues();
    const auto& dPhi     = seg.gridPhiValues();
    const double offPhi  = seg.offsetPhi();
    const std::size_t nR = rBins.size() - 1;

    for (std::size_t iR = 0; iR < nR; ++iR) {
      const double rMid      = 0.5 * (rBins[iR] + rBins[iR + 1]);
      const double dP        = dPhi[iR];
      const std::size_t nPhi = std::max<std::size_t>(1, std::round(2 * M_PI / dP));

      for (std::size_t ip = 0; ip < nPhi; ++ip) {
        const double phi = offPhi + (ip + 0.5) * dP;
        const double x   = rMid * std::cos(phi);
        const double y   = rMid * std::sin(phi);
        unique.insert(seg.cellID({x, y, 0}, {x, y, 0}, 0));
      }
    }
  }

  /* -------------------------------------------------------------- */
  /* PolarGridRPhi  (uniform ΔR, Δφ)                                */
  /* -------------------------------------------------------------- */
  template <typename POLAR_UNI>
  void scanPolarUniform(const POLAR_UNI& seg, const TGeoBBox* box,
                        std::set<dd4hep::CellID>& unique) {
    const double dR     = seg.gridSizeR();
    const double dPhi   = seg.gridSizePhi();
    const double offR   = seg.offsetR();
    const double offPhi = seg.offsetPhi();

    const double Rmax = std::hypot(box->GetDX(), box->GetDY());

    const std::size_t nR   = std::max<std::size_t>(1, std::ceil((Rmax - offR) / dR));
    const std::size_t nPhi = std::max<std::size_t>(1, std::round(2 * M_PI / dPhi));

    for (std::size_t iR = 0; iR < nR; ++iR) {
      const double r = offR + (iR + 0.5) * dR;
      for (std::size_t iP = 0; iP < nPhi; ++iP) {
        const double phi = offPhi + (iP + 0.5) * dPhi;
        const double x   = r * std::cos(phi);
        const double y   = r * std::sin(phi);
        unique.insert(seg.cellID({x, y, 0}, {x, y, 0}, 0));
      }
    }
  }

  /* Monte-Carlo fallback */
  void scanGeneric(const dd4hep::Segmentation& seg, const TGeoBBox* box,
                   std::set<dd4hep::CellID>& unique) {
    static thread_local std::mt19937_64 rng{0xFEDCBA98u};
    std::uniform_real_distribution<> ux(-box->GetDX(), box->GetDX());
    std::uniform_real_distribution<> uy(-box->GetDY(), box->GetDY());
    std::uniform_real_distribution<> uz(-box->GetDZ(), box->GetDZ());

    for (unsigned i = 0; i < 20000; ++i) {
      dd4hep::Position p{ux(rng), uy(rng), uz(rng)};
      unique.insert(seg.cellID(p, p, 0));
      if (unique.size() > 60000)
        break;
    }
  }

  std::string lower(std::string s) {
  for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return s;
  }

} // anonymous namespace

//====================================================================//
//  0.1  Initialisation                                                //
//====================================================================//
void RandomNoise::init() {
  // 1. Get DD4hep geometry
  m_dd4hepGeo = algorithms::GeoSvc::instance().detector();
  if (!m_dd4hepGeo) {
    error("RandomNoise: no DD4hep geometry service found");
    return;
  }

  // 2. Resolve readout
  m_readout = m_dd4hepGeo->readout(m_cfg.readout_name);
  if (!m_readout.isValid()) {
    error("RandomNoise: Invalid Readout name: {}", m_cfg.readout_name);
    return;
  }

  // 3. Find and store target detector elements
  m_targetDets.clear();
  m_idPathsCache.clear();
  m_boundsCache.clear();
  const auto& elements = m_dd4hepGeo->detectors();
  for (const auto& de : elements) {
    const dd4hep::DetElement& tdet = de.second;
    dd4hep::SensitiveDetector sd   = m_dd4hepGeo->sensitiveDetector(de.first);
    if (!sd)
      continue;
    auto readout = sd.readout();
    if (!readout)
      continue;
    if (readout.name() == m_cfg.readout_name) {
      m_targetDets.push_back(tdet);
    }
  }

  // 4. Precompute and cache idPaths and bounds for each target DetElement
  for (const auto& tdet : m_targetDets) {
    auto idPaths                = ScanDetectorElement(tdet);
    auto bounds                 = ScanComponent(tdet);
    m_idPathsCache[tdet.name()] = idPaths;
    m_boundsCache[tdet.name()]  = bounds;
  }
}

//====================================================================//
//  1.  Event processing                                               //
//====================================================================//
// Source-mode: this process() ignores the input collection and emits only synthetic noise.
void RandomNoise::process(const Input& in, const Output& out) const {
  auto [out_hits]      = out;
  const auto [headers] = in; // EventHeader collection (may be null)
  // Source-mode: no input collection is consumed. If disabled or no targets, emit empty.
  if (!m_cfg.addNoise) {
    info("RandomNoise: addNoise=false → emitting empty collection");
    return;
  }
  if (m_targetDets.empty()) {
    error("RandomNoise: no valid detector found for requested readout");
    return;
  }

  // ------------------------------------------------------------------
  // RNG seeded from EventHeader for reproducibility (#1934)
  // Prefer metadata input EventHeader; fall back to optional member pointer.
  // ------------------------------------------------------------------
  // Seed RNG exactly like SiliconTrackerDigi
  uint64_t seed = 0;
  if (headers) {
    seed = m_uid.getUniqueID(*headers, name());
  } else if (m_eventHeader) {
    // Optional fallback if someone set a single header via setter
    seed = (static_cast<uint64_t>(m_eventHeader->getRunNumber()) << 32) ^
           static_cast<uint64_t>(m_eventHeader->getEventNumber()) ^
           std::hash<std::string_view>{}(name());
  } else {
    info("RandomNoise: EventHeader not provided; seeding only from algorithm name (non-ideal)");
    seed = std::hash<std::string_view>{}(name());
  }
  std::mt19937_64 rng(seed);
  // Build map of noise-only hits (no input cloning)
  std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit> cell_hit_map;

  // Inject noise for each cached DetElement using precomputed data
  for (const auto& tdet : m_targetDets) {
    const auto& idPaths = m_idPathsCache.at(tdet.name());
    const auto& bounds  = m_boundsCache.at(tdet.name());
    add_noise_hits(cell_hit_map, tdet, idPaths, bounds, rng);
  }

  // Copy to output
  for (const auto& kv : cell_hit_map)
    out_hits->push_back(kv.second);
}

//====================================================================//
//  2.  Simple helper – check if ALL keys exist in a map               //
//====================================================================//
bool RandomNoise::hasAllKeys(const VolIDMap& m, const std::vector<std::string>& keys) {
  for (auto const& k : keys)
    if (m.find(k) == m.end())
      return false;
  return true;
}

//====================================================================//
//  3.  Recursively walk the Geo tree and collect every ID-path        //
//====================================================================//
void RandomNoise::PrintVolIDsRecursive(const dd4hep::PlacedVolume& pv, VolIDMapArray& result,
                                       VolIDMap& current, int depth) const {
  std::vector<std::string> added;
  for (auto const& id : pv.volIDs()) {
    current[id.first] = id.second;
    added.push_back(id.first);
  }

  TGeoNode* node = pv.ptr();
  if (!node || node->GetNdaughters() == 0) {
    result.push_back(current);
  } else {
    const int ndau = node->GetNdaughters();
    for (int i = 0; i < ndau; ++i)
      if (auto* d = node->GetDaughter(i))
        PrintVolIDsRecursive(dd4hep::PlacedVolume(d), result, current, depth + 1);
  }
  for (auto const& k : added)
    current.erase(k);
}

// Public wrapper
RandomNoise::VolIDMapArray RandomNoise::ScanDetectorElement(dd4hep::DetElement de,
                                                            bool keepDeepestOnly) const {
  VolIDMapArray paths;
  VolIDMap cur;
  PrintVolIDsRecursive(de.placement(), paths, cur, 0);

  if (keepDeepestOnly) {
    std::size_t maxKeys = 0;
    for (auto const& p : paths)
      maxKeys = std::max(maxKeys, p.size());

    VolIDMapArray filtered;
    for (auto const& p : paths)
      if (p.size() == maxKeys)
        filtered.push_back(p);
    paths.swap(filtered);
  }
  return paths;
}

//====================================================================//
//  4.  ScanComponent – determine min/max for local segmentation       //
//      (identical to the previously reviewed version, not shortened   //
//      here to keep the file self-contained)                          //
//====================================================================//
RandomNoise::ComponentBounds RandomNoise::ScanComponent(dd4hep::DetElement de,
                                                        std::string /*name*/) const {
  ComponentBounds bounds;

  /* -------- 1) sensitive leaf 찾기 ------------------------------ */
  struct Leaf {
    dd4hep::SensitiveDetector sd;
    dd4hep::PlacedVolume pv;
  };
  std::function<std::optional<Leaf>(dd4hep::DetElement)> findLeaf =
      [&](dd4hep::DetElement d) -> std::optional<Leaf> {
    if (!d.isValid())
      return {};
    auto pv = d.placement();
    if (!pv.isValid())
      return {};
    auto sd = pv.volume().sensitiveDetector();
    if (sd.isValid()) {
      bool child = false;
      for (auto const& [_, c] : d.children())
        if (c.placement().isValid() && c.placement().volume().sensitiveDetector().isValid()) {
          child = true;
          break;
        }
      if (!child)
        return Leaf{sd, pv};
    }
    for (auto const& [_, c] : d.children())
      if (auto r = findLeaf(c))
        return r;
    return {};
  };

  auto leaf = findLeaf(de);
  if (!leaf)
    return bounds;

  const auto* box = dynamic_cast<const TGeoBBox*>(leaf->pv.volume().solid().ptr());
  if (!box)
    return bounds;

  dd4hep::Segmentation segH = leaf->sd.readout().segmentation();
  auto seg                  = segH; // handle
  const auto* decoder       = seg.decoder();

  /* -------- 2) probe volume ------------------------------------ */
  std::set<dd4hep::CellID> unique;

  auto* ptr = seg.ptr();
  if (auto* cart = dynamic_cast<const dd4hep::DDSegmentation::CartesianGrid*>(ptr)) {
    scanCartesian(*cart, box, unique);
  } else if (auto* cyl =
                 dynamic_cast<const dd4hep::DDSegmentation::CylindricalSegmentation*>(ptr)) {
    scanCylindrical(*cyl, box, unique);
  } else if (auto* pol2 = dynamic_cast<const dd4hep::DDSegmentation::PolarGridRPhi2*>(ptr)) {
    scanPolarRPhi(*pol2, unique); // ← 새 함수
  } else if (auto* pol1 = dynamic_cast<const dd4hep::DDSegmentation::PolarGridRPhi*>(ptr)) {
    scanPolarUniform(*pol1, box, unique);
  } else {
    scanGeneric(seg, box, unique);
  }

  /* -------- 3) min / max 수집 ---------------------------------- */
  for (auto cid : unique) {
    for (auto const& f : decoder->fields()) {
      std::string n = f.name();
      long v        = decoder->get(cid, n);
      auto& rng     = bounds[n];
      if (rng.first == 0 && rng.second == 0)
        rng = {v, v};
      else {
        rng.first  = std::min(rng.first, v);
        rng.second = std::max(rng.second, v);
      }
    }
  }
  return bounds;
}

//====================================================================//
//  5.  Add noise hits for ONE DetElement                             //
//====================================================================//
// New overload: add_noise_hits with precomputed idPaths and bounds and RNG
void RandomNoise::add_noise_hits(
    std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& cell_hit_map,
    const dd4hep::DetElement& det, const VolIDMapArray& idPaths, const ComponentBounds& bounds,
    std::mt19937_64& rng) const {
  if (!m_cfg.addNoise || !m_dd4hepGeo)
    return;
  inject_noise_hits(cell_hit_map, det, idPaths, bounds, rng);
}

// Optionally, keep the old signature for compatibility
void RandomNoise::add_noise_hits(
    std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& cell_hit_map,
    const dd4hep::DetElement& det) const {
  // Use cached data if available, otherwise fallback to scanning
  auto it_id = m_idPathsCache.find(det.name());
  auto it_b  = m_boundsCache.find(det.name());

  // Fallback deterministic seed when EventHeader not available here
  uint64_t fallbackSeed = std::hash<std::string_view>{}(name()) ^ 0x9e3779b97f4a7c15ULL;
  std::mt19937_64 rng(fallbackSeed);

  if (it_id != m_idPathsCache.end() && it_b != m_boundsCache.end()) {
    add_noise_hits(cell_hit_map, det, it_id->second, it_b->second, rng);
  } else {
    auto idPaths = ScanDetectorElement(det);
    auto bounds  = ScanComponent(det);
    inject_noise_hits(cell_hit_map, det, idPaths, bounds, rng);
  }
}

//--------------------------------------------------------------------------
//  Inject N synthetic noise hits into ONE detector component
//--------------------------------------------------------------------------
void RandomNoise::inject_noise_hits(
    std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& hitMap,
    const dd4hep::DetElement& det, const VolIDMapArray& idPaths, const ComponentBounds& bounds,
    std::mt19937_64& rng) const {
    if (idPaths.empty() || bounds.empty())
    return;


  dd4hep::SensitiveDetector sd = m_dd4hepGeo->sensitiveDetector(det.name());
  if (!sd.isValid()) {
    error("inject_noise_hits: no SensitiveDetector for '{}'", det.name());
    return;
  }

  dd4hep::Readout ro = sd.readout();
  if (!ro.isValid()) {
    error("inject_noise_hits: no Readout for '{}'", det.name());
    return;
  }
  dd4hep::Segmentation segH = ro.segmentation();
  if (!segH.isValid()) {
    error("inject_noise_hits: no Segmentation for '{}'", det.name());
    return;
  }
  const auto* decoder = segH.decoder();
  if (!decoder) {
    error("inject_noise_hits: no BitFieldCoder for '{}'", det.name());
    return;
  }

  // Detect layer field name (prefer "layer", else anything containing "lay")
  auto detectLayerField = [&]() -> std::string {
    for (auto const& [n, _] : bounds) {
      if (lower(n) == "layer") return n;
    }
    for (auto const& [n, _] : bounds) {
      auto ln = lower(n);
      if (ln.find("lay") != std::string::npos) return n;
    }
    return std::string{};
  };
  const std::string layerField = detectLayerField();

  // Build distributions for non-fixed* fields (we will keep fixed volID keys as-is)
  struct Dist { std::uniform_int_distribution<long> uni; };
  std::unordered_map<std::string, Dist> fieldDists;
  for (auto const& [name, r] : bounds)
    fieldDists.emplace(name, Dist{std::uniform_int_distribution<long>(r.first, r.second)});

  // Utility to compute total "channels" count given a subset of idPaths
  auto computeChannels = [&](const std::vector<const VolIDMap*>& sensors) -> std::size_t {
    std::size_t perSensor = 1;
    for (auto const& [fname, rng] : bounds) {
      // If field is already set in a base sensor (e.g. layer, module), it is not multiplied
      // here since it is fixed by that sensor's id path; only missing fields contribute.
      bool fixedInAll = true;
      for (auto* base : sensors) {
        if (base->find(fname) == base->end()) { fixedInAll = false; break; }
      }
      if (!fixedInAll) {
        perSensor *= static_cast<std::size_t>(std::max<long>(1, rng.second - rng.first + 1));
      }
    }
    return perSensor * sensors.size();
  };

  // Function that draws 'nNoise' unique hits using a given sensor subset
  auto drawHits = [&](std::size_t nNoise, const std::vector<const VolIDMap*>& sensors) {
    if (nNoise == 0 || sensors.empty()) return;

    std::uniform_int_distribution<std::size_t> pickSensor(0, sensors.size() - 1);

    std::size_t created = 0;
    while (created < nNoise) {
      const VolIDMap& base = *sensors[pickSensor(rng)];
      dd4hep::CellID cid   = 0;

      // 3.1 set base fields first (fixed part)
      for (auto const& kv : base)
        decoder->set(cid, kv.first, kv.second);

      // 3.2 randomise only missing fields
      for (auto& kv : fieldDists) {
        if (base.find(kv.first) != base.end())
          continue;
        decoder->set(cid, kv.first, kv.second.uni(rng));
      }

      // 3.3 uniqueness / validity checks
      if (hitMap.find(cid) != hitMap.end())
        continue;
        
      bool inside = true;
      try { segH.position(cid); }
      catch (...) { inside = false; }
      if (!inside) continue;
      
      // 3.4 store hit (placeholder charge/time)
      edm4eic::MutableRawTrackerHit h;
      h.setCellID(cid);
      h.setCharge(1.0e6);
      h.setTimeStamp(0.0);

      hitMap.emplace(cid, h);
      ++created;
    }
  };

  // BVTX: per-layer noise, else is system-wide as of now
  // We use per-layer only if: layer ids AND per-layer rates are configured AND a layer field exists in the ID/segmentation
  const bool hasLayerConfig =
      (!m_cfg.layer_id.empty() && !m_cfg.n_noise_hits_per_layer.empty() &&
       m_cfg.layer_id.size() == m_cfg.n_noise_hits_per_layer.size());
  const bool canDoLayerWise = (!layerField.empty());

  if (hasLayerConfig && canDoLayerWise) {
    // Group sensors by layer value
    std::unordered_map<int, std::vector<const VolIDMap*>> sensorsByLayer;
    sensorsByLayer.reserve(m_cfg.layer_id.size());

    for (auto const& base : idPaths) {
      auto it = base.find(layerField);
      if (it != base.end()) {
        sensorsByLayer[it->second].push_back(&base);
      }
    }

    // For each configured layer, generate hits using that layer's mean
    for (std::size_t i = 0; i < m_cfg.layer_id.size(); ++i) {

      if (!m_cfg.detector_names.empty()) {
      if (i < m_cfg.detector_names.size()) {
      const auto& want = m_cfg.detector_names[i];
      if (!want.empty() && want != det.name()) {
        continue; // skip this (layer,mean) for other detectors
        }
      } 
      }

      int L = m_cfg.layer_id[i];
      auto it = sensorsByLayer.find(L);
      if (it == sensorsByLayer.end() || it->second.empty()) {
        info("inject_noise_hits '{}': layer {} has no sensors (skipping)", det.name(), L);
        continue;
      }
      const std::size_t nChannels = computeChannels(it->second);
      std::poisson_distribution<std::size_t> pois(m_cfg.n_noise_hits_per_layer[i]);
      std::size_t nNoise = std::min<std::size_t>(pois(rng), nChannels);

      info("inject_noise_hits '{}': layer {} → {} channels → {} noise hits",
           det.name(), L, nChannels, nNoise);

      drawHits(nNoise, it->second);
    }
    return; // layer path done -> so far only works for BVTX, but trivial to implement for BTRK/ECTRK.
  }

  // system-wide poisson using all(!) sensors (BTRK/ECTRK or no layer field RN)
  std::vector<const VolIDMap*> allSensors;
  allSensors.reserve(idPaths.size());
  for (auto const& base : idPaths)
    allSensors.push_back(&base);

  const std::size_t nChannels  = computeChannels(allSensors);
  std::poisson_distribution<std::size_t> pois(m_cfg.n_noise_hits_per_system);
  std::size_t nNoise = std::min<std::size_t>(pois(rng), nChannels);

  info("inject_noise_hits '{}': {} channels (system-wide) → {} noise hits",
       det.name(), nChannels, nNoise);

  drawHits(nNoise, allSensors);
  }
} // namespace eicrecon
