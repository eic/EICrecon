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
#include <edm4hep/SimTrackerHit.h>

#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/TrackerHitCollection.h>

#include "services/io/podio/datamodel_glue.h"

struct timeAlignmentFactory : public JOmniFactory<timeAlignmentFactory> {
  JEventLevel m_factory_level;

  std::vector<std::string> m_trackerhit_collection_names = {
      "TOFBarrelRecHits_TK", "TOFEndcapRecHits_TK",
      "MPGDBarrelRecHits_TK", "OuterMPGDBarrelRecHits_TK",
      "BackwardMPGDEndcapRecHits_TK", "ForwardMPGDEndcapRecHits_TK",
      "SiBarrelVertexRecHits_TK", "SiBarrelTrackerRecHits_TK",
      "SiEndcapTrackerRecHits_TK", "TaggerTrackerRecHits_TK",
      "B0TrackerRecHits_TK", "DIRCBarRecHits_TK",
      "DRICHRecHits_TK", "ForwardOffMTrackerRecHits_TK",
      "ForwardRomanPotRecHits_TK", "LumiSpecTrackerRecHits_TK",
      "RICHEndcapNRecHits_TK"};

  std::vector<std::string> m_trackerhit_collection_names_aligned = {
      "TOFBarrelRecHits_TK_aligned", "TOFEndcapRecHits_TK_aligned",
      "MPGDBarrelRecHits_TK_aligned", "OuterMPGDBarrelRecHits_TK_aligned",
      "BackwardMPGDEndcapRecHits_TK_aligned", "ForwardMPGDEndcapRecHits_TK_aligned",
      "SiBarrelVertexRecHits_TK_aligned", "SiBarrelTrackerRecHits_TK_aligned",
      "SiEndcapTrackerRecHits_TK_aligned", "TaggerTrackerRecHits_TK_aligned",
      "B0TrackerRecHits_TK_aligned", "DIRCBarRecHits_TK_aligned",
      "DRICHRecHits_TK_aligned", "ForwardOffMTrackerRecHits_TK_aligned",
      "ForwardRomanPotRecHits_TK_aligned", "LumiSpecTrackerRecHits_TK_aligned",
      "RICHEndcapNRecHits_TK_aligned"};


  // VariadicPodioInput<edm4hep::SimTrackerHit> m_trackerhits_in{
  //     this, {.names = m_trackerhit_collection_names, .is_optional = true}};
  VariadicPodioInput<edm4eic::TrackerHit> m_trackerhits_in{
      this, {.names = m_trackerhit_collection_names, .is_optional = true}};
  VariadicPodioOutput<edm4eic::TrackerHit> m_trackerhits_out{this,
                                                             m_trackerhit_collection_names_aligned};

  Double_t m_time_offset = 0.0; // Time offset to apply to hits

  void Configure() {}

  void ChangeRun(int32_t /*run_nr*/) {}

  void Execute(int64_t run_number, uint64_t event_number) {
    std::cout << "<<<<<<<<<<<<Time Alignment Factory: Event " << event_number << std::endl;
    unsigned int nColls = 0;
    for (size_t coll_index = 0; coll_index < m_trackerhits_in().size(); ++coll_index) {
      const auto* coll_in = m_trackerhits_in().at(coll_index);
      auto& coll_out      = m_trackerhits_out().at(coll_index);
      if (coll_in != nullptr) {
        // std::vector<edm4hep::MutableSimTrackerHit> sorted_hits; // for edm4hep (G4Hit level)
        std::vector<edm4eic::MutableTrackerHit> sorted_hits; // for edm4eic (After digitization)
        for (const auto& hit : *coll_in) {
          // edm4hep::MutableSimTrackerHit copiedHit = hit.clone(); // for edm4hep (G4Hit level)
          edm4eic::MutableTrackerHit copiedHit = hit.clone(); // for edm4eic (After digitization)

          Double_t hitR = std::sqrt(hit.getPosition()[0] * hit.getPosition()[0] +
                                    hit.getPosition()[1] * hit.getPosition()[1] +
                                    hit.getPosition()[2] * hit.getPosition()[2]);
          Double_t calibTime = (hitR - 91.7)/279;
          copiedHit.setTime(hit.getTime() - calibTime);
          sorted_hits.push_back(copiedHit);

          // std::cout << ">>> Detector " << m_trackerhit_collection_names.at(coll_index)
          //           << " Original Time: " << hit.getTime()
          //           << " Calibrated Time: " << hit.getTime() - calibTime << std::endl;
        }

        std::sort(sorted_hits.begin(), sorted_hits.end(),
                  [](const auto& a, const auto& b) { return a.getTime() < b.getTime(); });

        for (auto hit : sorted_hits) {
          auto hitTime = hit.getTime();
          coll_out->push_back(hit);
        }
      }
      nColls++;
    }
  }
};
