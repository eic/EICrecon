// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// #include "extensions/jana/JOmniFactoryGeneratorT.h"
#include <JANA/Components/JOmniFactory.h>
// #include <JANA/Utils/JEventKey.h>

#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>

#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/TrackerHitCollection.h>

#include "services/io/podio/datamodel_glue.h"

struct timeAlignmentFactory : public JOmniFactory<timeAlignmentFactory> {
  JEventLevel m_factory_level;

  std::vector<std::string> m_trackerhit_collection_names = {"MPGDBarrelRecHits_TK"};
  std::vector<std::string> m_trackerhit_collection_names_aligned = {"MPGDBarrelRecHits_TK_aligned"};

  // std::vector<std::string> m_trackerhit_collection_names = {
  //     "B0TrackerRecHits_TK",       "BackwardMPGDEndcapRecHits_TK", "DIRCBarRecHits_TK",
  //     "DRICHRecHits_TK",           "ForwardMPGDEndcapRecHits_TK",  "ForwardOffMTrackerRecHits_TK",
  //     "ForwardRomanPotRecHits_TK", "LumiSpecTrackerRecHits_TK",    "MPGDBarrelRecHits_TK",
  //     "OuterMPGDBarrelRecHits_TK", "RICHEndcapNRecHits_TK",        "SiBarrelTrackerRecHits_TK",
  //     "TOFBarrelRecHits_TK",       "TOFEndcapRecHits_TK",          "TaggerTrackerRecHits_TK",
  //     "SiEndcapTrackerRecHits_TK", "SiBarrelVertexRecHits_TK"};

  // std::vector<std::string> m_trackerhit_collection_names_aligned = {
  //     "B0TrackerRecHits_TK_aligned",         "BackwardMPGDEndcapRecHits_TK_aligned",
  //     "DIRCBarRecHits_TK_aligned",           "DRICHRecHits_TK_aligned",
  //     "ForwardMPGDEndcapRecHits_TK_aligned", "ForwardOffMTrackerRecHits_TK_aligned",
  //     "ForwardRomanPotRecHits_TK_aligned",   "LumiSpecTrackerRecHits_TK_aligned",
  //     "MPGDBarrelRecHits_TK_aligned",        "OuterMPGDBarrelRecHits_TK_aligned",
  //     "RICHEndcapNRecHits_TK_aligned",       "SiBarrelTrackerRecHits_TK_aligned",
  //     "TOFBarrelRecHits_TK_aligned",         "TOFEndcapRecHits_TK_aligned",
  //     "TaggerTrackerRecHits_TK_aligned",     "SiEndcapTrackerRecHits_TK_aligned",
  //     "SiBarrelVertexRecHits_TK_aligned"};

  VariadicPodioInput<edm4eic::TrackerHit> m_trackerhits_in{
      this, {.names = m_trackerhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4eic::TrackerHit> m_trackerhits_out{this,
                                                             m_trackerhit_collection_names_aligned};

  Double_t m_time_offset = 0.0; // Time offset to apply to hits

  void Configure() {}

  void ChangeRun(int32_t /*run_nr*/) {}

  void Execute(int64_t run_number, uint64_t event_number) {
    Int_t a = 1;
    // for (size_t coll_index = 0; coll_index < m_trackerhits_in().size(); ++coll_index) {
    //   const auto* coll_in = m_trackerhits_in().at(coll_index);
    //   auto& coll_out      = m_trackerhits_out().at(coll_index);
    //   if (coll_in != nullptr) {
    //     // std::vector<edm4hep::Mutabletrackerhit> sorted_hits; // for edm4hep (G4Hit level)
    //     std::vector<edm4eic::MutableTrackerHit> sorted_hits; // for edm4eic (After digitization)
    //     for (const auto& hit : *coll_in) {
    //       // edm4hep::Mutabletrackerhit copiedHit = hit.clone(); // for edm4hep (G4Hit level)
    //       edm4eic::MutableTrackerHit copiedHit = hit.clone(); // for edm4eic (After digitization)
    //       copiedHit.setTime(hit.getTime() - m_time_offset);
    //       sorted_hits.push_back(copiedHit);
    //     }

    //     std::sort(sorted_hits.begin(), sorted_hits.end(),
    //               [](const auto& a, const auto& b) { return a.getTime() < b.getTime(); });

    //     for (auto hit : sorted_hits) {
    //       auto hitTime = hit.getTime();
    //       coll_out->push_back(hit);
    //     }
    //   }
    // }
  }
};
