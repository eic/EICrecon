// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <RtypesCore.h>

#include <extensions/jana/JOmniFactory.h>

#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/SimTrackerHit.h>

#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/TrackerHitCollection.h>

struct timeAlignmentFactory : public JOmniFactory<timeAlignmentFactory> {
  JEventLevel m_factory_level;

  std::vector<std::string> m_trackerhit_collection_names = {
      "TOFBarrelRecHits",         "TOFEndcapRecHits",          "MPGDBarrelRecHits",
      "OuterMPGDBarrelRecHits",   "BackwardMPGDEndcapRecHits", "ForwardMPGDEndcapRecHits",
      "SiBarrelVertexRecHits",    "SiBarrelTrackerRecHits",    "SiEndcapTrackerRecHits",
      "B0TrackerRecHits",         "TaggerTrackerRecHits",      "ForwardRomanPotRecHits",
      "ForwardOffMTrackerRecHits"};

  // "DRICHRecHits"
  // "DIRCBarRecHits",
  // "RICHEndcapNRecHits_TK" // PFRICH

  std::vector<std::string> m_trackerhit_collection_names_aligned = {
      "TOFBarrelRecHits_aligned",          "TOFEndcapRecHits_aligned",
      "MPGDBarrelRecHits_aligned",         "OuterMPGDBarrelRecHits_aligned",
      "BackwardMPGDEndcapRecHits_aligned", "ForwardMPGDEndcapRecHits_aligned",
      "SiBarrelVertexRecHits_aligned",     "SiBarrelTrackerRecHits_aligned",
      "SiEndcapTrackerRecHits_aligned",    "B0TrackerRecHits_aligned",
      "TaggerTrackerRecHits_aligned",      "ForwardRomanPotRecHits_aligned",
      "ForwardOffMTrackerRecHits_aligned"};

  // "DRICHRecHits_aligned"
  // "DIRCBarRecHits_aligned",
  // "RICHEndcapNRecHits_TK_aligned" // PFRICH

  VariadicPodioInput<edm4eic::TrackerHit, true> m_trackerhits_in{this,
                                                                 m_trackerhit_collection_names};

  VariadicPodioOutput<edm4eic::TrackerHit> m_trackerhits_out{this,
                                                             m_trackerhit_collection_names_aligned};

  Double_t m_time_offset = 0.0; // Time offset to apply to hits

  void Configure() {}

  void ChangeRun(int32_t /*run_nr*/) {}

  void Process(int64_t /*run_number*/, uint64_t /*event_number*/) {
    for (size_t coll_index = 0; coll_index < m_trackerhits_in().size(); ++coll_index) {
      const auto* coll_in = m_trackerhits_in().at(coll_index);
      auto& coll_out      = m_trackerhits_out().at(coll_index);

      if (coll_in != nullptr) {
        // std::vector<edm4hep::MutableSimTrackerHit> sorted_hits; // for edm4hep (G4Hit level)
        std::vector<edm4eic::MutableTrackerHit> sorted_hits; // for edm4eic (After digitization)
        for (const auto& hit : *coll_in) {
          // edm4hep::MutableSimTrackerHit copiedHit = hit.clone(); // for edm4hep (G4Hit level)
          edm4eic::MutableTrackerHit copiedHit = hit.clone(); // for edm4eic (After digitization)

          Double_t hitR      = std::sqrt(hit.getPosition()[0] * hit.getPosition()[0] +
                                         hit.getPosition()[1] * hit.getPosition()[1] +
                                         hit.getPosition()[2] * hit.getPosition()[2]);
          Double_t calibTime = hitR * 0.0034;
          copiedHit.setTime(hit.getTime() - calibTime);
          sorted_hits.push_back(copiedHit);

          // std::cout << ">>> Detector " << m_trackerhit_collection_names.at(coll_index)
          //           << " Original Time: " << hit.getTime()
          //           << " Calibrated Time: " << hit.getTime() - calibTime << std::endl;
        }

        std::sort(sorted_hits.begin(), sorted_hits.end(),
                  [](const auto& a, const auto& b) { return a.getTime() < b.getTime(); });

        for (const auto& hit : sorted_hits) {
          coll_out->push_back(hit);
        }
      }
    }
  }
};
