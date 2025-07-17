// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/Components/JOmniFactory.h>
// #include <JANA/Utils/JEventKey.h>

#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>

#include "services/io/podio/datamodel_glue.h"

struct timeAlignmentFactory : public JOmniFactory<timeAlignmentFactory> {
  JEventLevel m_factory_level;

  // PodioInput<edm4hep::SimTrackerHit> check_hits_in {this};
  // PodioOutput<edm4hep::SimTrackerHit> check_hits_out {this};

  std::vector<std::string> m_simtrackerhit_collection_names_aligned = {
      "B0TrackerHits_aligned",         "BackwardMPGDEndcapHits_aligned",
      "DIRCBarHits_aligned",           "DRICHHits_aligned",
      "ForwardMPGDEndcapHits_aligned", "ForwardOffMTrackerHits_aligned",
      "ForwardRomanPotHits_aligned",   "LumiSpecTrackerHits_aligned",
      "MPGDBarrelHits_aligned",        "OuterMPGDBarrelHits_aligned",
      "RICHEndcapNHits_aligned",       "SiBarrelHits_aligned",
      "TOFBarrelHits_aligned",         "TOFEndcapHits_aligned",
      "TaggerTrackerHits_aligned",     "TrackerEndcapHits_aligned",
      "VertexBarrelHits_aligned"};

  std::vector<std::string> m_simtrackerhit_collection_names = {
      "B0TrackerHits",       "BackwardMPGDEndcapHits", "DIRCBarHits",
      "DRICHHits",           "ForwardMPGDEndcapHits",  "ForwardOffMTrackerHits",
      "ForwardRomanPotHits", "LumiSpecTrackerHits",    "MPGDBarrelHits",
      "OuterMPGDBarrelHits", "RICHEndcapNHits",        "SiBarrelHits",
      "TOFBarrelHits",       "TOFEndcapHits",          "TaggerTrackerHits",
      "TrackerEndcapHits",   "VertexBarrelHits"};

  VariadicPodioInput<edm4hep::SimTrackerHit> m_simtrackerhits_in{
      this, {.names = m_simtrackerhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4hep::SimTrackerHit> m_simtrackerhits_out{
      this, m_simtrackerhit_collection_names_aligned};

  Double_t m_time_offset = 0.0; // Time offset to apply to hits

  void Configure() {}

  void ChangeRun(int32_t /*run_nr*/) {}

  void Execute(int64_t run_number, uint64_t event_number) {
    for (size_t coll_index = 0; coll_index < m_simtrackerhits_in().size(); ++coll_index) {
      const auto* coll_in = m_simtrackerhits_in().at(coll_index);
      auto& coll_out      = m_simtrackerhits_out().at(coll_index);
      if (coll_in != nullptr) {
        std::vector<edm4hep::MutableSimTrackerHit> sorted_hits;
        for (const auto& hit : *coll_in) {
          edm4hep::MutableSimTrackerHit copiedHit = hit.clone();
          copiedHit.setTime(hit.getTime() - m_time_offset);
          sorted_hits.push_back(copiedHit);
        }

        std::sort(sorted_hits.begin(), sorted_hits.end(),
                  [](const auto& a, const auto& b) { return a.getTime() < b.getTime(); });

        for (auto hit : sorted_hits) {
          auto hitTime = hit.getTime();
          coll_out->push_back(hit);
        }
      }
    }
  }
};
