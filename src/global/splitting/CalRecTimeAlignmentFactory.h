// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <extensions/jana/JOmniFactory.h>
#include <edm4eic/CalorimeterHitCollection.h>

#include "CalRecTimeAlignment.h"

struct CalRecTimeAlignmentFactory : public JOmniFactory<CalRecTimeAlignmentFactory> {
  using AlgoT = eicrecon::CalRecTimeAlignment;

  JEventLevel m_factory_level;

  std::unique_ptr<AlgoT> m_algo;

  std::vector<std::string> m_simcalorechit_collection_names = {"B0ECalRecHits",
                                                               "EcalBarrelImagingRecHits",
                                                               "EcalBarrelScFiRecHits",
                                                               "EcalEndcapNRecHits",
                                                               "EcalEndcapPRecHits",
                                                               "EcalFarForwardZDCRecHits",
                                                               "EcalLumiSpecRecHits",
                                                               "HcalBarrelRecHits",
                                                               "HcalEndcapNRecHits",
                                                               "HcalEndcapPInsertRecHits",
                                                               "HcalFarForwardZDCRecHits",
                                                               "LFHCALRecHits"};

  std::vector<std::string> m_simcalorechit_collection_names_aligned = {
      "B0ECalRecHits_aligned",
      "EcalBarrelImagingRecHits_aligned",
      "EcalBarrelScFiRecHits_aligned",
      "EcalEndcapNRecHits_aligned",
      "EcalEndcapPRecHits_aligned",
      "EcalFarForwardZDCRecHits_aligned",
      "EcalLumiSpecRecHits_aligned",
      "HcalBarrelRecHits_aligned",
      "HcalEndcapNRecHits_aligned",
      "HcalEndcapPInsertRecHits_aligned",
      "HcalFarForwardZDCRecHits_aligned",
      "LFHCALRecHits_aligned"};

  VariadicPodioInput<edm4eic::CalorimeterHit, true> m_calorechit_in{
      this, m_simcalorechit_collection_names};

  VariadicPodioOutput<edm4eic::CalorimeterHit> m_calorechit_out{
      this, m_simcalorechit_collection_names_aligned};

  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->init();
  }

  void ChangeRun(int32_t /*run_nr*/) override {}

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) override {

    for (size_t coll_index = 0; coll_index < m_calorechit_in().size(); ++coll_index) {
      const auto* coll_in = m_calorechit_in().at(coll_index);
      auto& coll_out      = m_calorechit_out().at(coll_index);

      if (coll_in != nullptr) {
        m_algo->process({coll_in}, {coll_out.get()});
      }
    }
  }
};
