// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 EIC-FT
//
//  RandomNoise.h
//  ------------------------------------------------------------------
//  Injects synthetic electronic noise into a RawTrackerHit collection
//  during the digitisation stage.  The class
//
//        • scans the DD4hep geometry to discover the index structure
//          of every sensitive detector that belongs to the input
//          Readout,
//        • derives practical min / max bounds for the local
//          segmentation fields,
//        • generates random hits inside these bounds and appends them
//          to the event.
//
//  All geometry interaction is performed at run-time; no detector-
//  specific hard-coding is required.
//

#pragma once

/* ROOT / DD4hep --------------------------------------------------------- */
#include <TRandom3.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Volumes.h>
#include <DDSegmentation/Segmentation.h>

/* Framework helpers ---------------------------------------------------- */
#include "RandomNoiseConfig.h"
#include "algorithms/algorithm.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

/* EDM4eic -------------------------------------------------------------- */
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeader.h>

/* STL ------------------------------------------------------------------ */
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>

class TGeoNode; // forward declaration (ROOT geometry node)

namespace eicrecon {

//-------------------------------------------------------------------------
//  Convenience type aliases
//-------------------------------------------------------------------------
using PlacementPath = std::vector<TGeoNode*>;

/* One bit-field of the read-out and the discrete values seen in a scan. */
struct FieldInfo {
  std::string name;        ///< Bit-field name as in ID encoding
  std::vector<int> values; ///< Values found in this component
};

/* Quick handle for the EICrecon algorithm template. */
using RandomNoiseAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::EventHeaderCollection>,
                          algorithms::Output<edm4eic::RawTrackerHitCollection>>;

//-------------------------------------------------------------------------
//  Main class
//-------------------------------------------------------------------------
class RandomNoise : public RandomNoiseAlgorithm, public WithPodConfig<RandomNoiseConfig> {
public:
  /* Map  field-name → (min,max)  returned by ScanComponent. */
  using ComponentBounds = std::map<std::string, std::pair<long, long>>;

  /* One complete volID path (field-name → value) and a list thereof. */
  using VolIDMap      = std::unordered_map<std::string, int>;
  using VolIDMapArray = std::vector<VolIDMap>;

  explicit RandomNoise(std::string_view name)
      : RandomNoiseAlgorithm{name,
                             {"EventHeader"},            // metadata-only input for RNG seeding
                             {"outputRawHitCollection"}, // default output tag
                             "Generates standalone noise RawTrackerHits for a given readout."} {}

  /* Called once – stores pointer to DD4hep geometry. */
  void init();

  /* Framework entry point – executed for every event. */
  void process(const Input&, const Output&) const final;

  // Set EventHeader pointer for per-event reproducible RNG seeding (#1934)
  void setEventHeader(const edm4hep::EventHeader* evtHeader) { m_eventHeader = evtHeader; }

  //-------------------------------------------------------------------------
  //  Geometry helpers (public for unit tests, otherwise used internally)
  //-------------------------------------------------------------------------
  /* Return all ID-paths from ‘de’ to its *sensitive* leaves.
       If keepDeepestOnly==true (default) only the longest paths are kept. */
  VolIDMapArray ScanDetectorElement(dd4hep::DetElement de, bool keepDeepestOnly = true) const;

  /* Determine min / max of every local segmentation field that appears
       inside ‘de’.  Parameter ‘name’ is reserved for future filters. */
  ComponentBounds ScanComponent(dd4hep::DetElement de, std::string name = "") const;

  //=====================================================================
  //  Noise generation helpers
  //=====================================================================

  /* Add noise hits belonging to one DetElement to the global hit-map. */
  void add_noise_hits(std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& hitMap,
                      const dd4hep::DetElement& det) const;

  void add_noise_hits(std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& hitMap,
                      const dd4hep::DetElement& det,
                      const VolIDMapArray& idPaths,
                      const ComponentBounds& bounds,
                      std::mt19937_64& rng) const;

  /* Core routine that creates the actual RawTrackerHits. */
  void inject_noise_hits(std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& map,
                         const dd4hep::DetElement& det, const VolIDMapArray& idPaths,
                         const ComponentBounds& bounds,
                         std::mt19937_64& rng) const;

  //=====================================================================
  //  Recursive helper used by ScanDetectorElement
  //=====================================================================
  void PrintVolIDsRecursive(const dd4hep::PlacedVolume& pv, VolIDMapArray& result,
                            VolIDMap& current, int depth) const;

  /* True if every key in ‘keys’ exists in map ‘m’. */
  static bool hasAllKeys(const VolIDMap& m, const std::vector<std::string>& keys);

  //-------------------------------------------------------------------------
  //  Data members
  //-------------------------------------------------------------------------
  const edm4hep::EventHeader* m_eventHeader = nullptr; // set each event for reproducible RNG
  dd4hep::Readout m_readout;
  std::vector<dd4hep::DetElement> m_targetDets;
  std::unordered_map<std::string, VolIDMapArray> m_idPathsCache;
  std::unordered_map<std::string, ComponentBounds> m_boundsCache;
  mutable TRandom3 m_random{0};                  ///< ROOT RNG (not used atm)
  const dd4hep::Detector* m_dd4hepGeo = nullptr; ///< DD4hep geometry handle
  std::string m_input_collection_name;           ///< Input tag override
  const algorithms::UniqueIDGenSvc& m_uid = algorithms::UniqueIDGenSvc::instance();
};

} // namespace eicrecon
