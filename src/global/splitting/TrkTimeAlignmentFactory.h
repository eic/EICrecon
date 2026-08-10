// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <extensions/jana/JOmniFactory.h>
#include <edm4eic/TrackerHitCollection.h>

#include "TrkTimeAlignment.h"

struct TrkTimeAlignmentFactory : public JOmniFactory<TrkTimeAlignmentFactory> {
  using AlgoT = eicrecon::TrkTimeAlignment;

  JEventLevel m_factory_level;

  std::unique_ptr<AlgoT> m_algo;

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

  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->init();
  }

  void ChangeRun(int32_t /*run_nr*/) override {}

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) override {
    for (size_t coll_index = 0; coll_index < m_trackerhits_in().size(); ++coll_index) {
      const auto* coll_in = m_trackerhits_in().at(coll_index);
      auto& coll_out      = m_trackerhits_out().at(coll_index);

      if (coll_in != nullptr) {
        m_algo->process({coll_in}, {coll_out.get()});
      }
    }
  }
};
