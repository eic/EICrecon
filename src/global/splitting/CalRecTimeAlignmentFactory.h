// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <extensions/jana/JOmniFactory.h>

#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4eic/CalorimeterHitCollection.h>

struct CalRecTimeAlignmentFactory : public JOmniFactory<CalRecTimeAlignmentFactory> {
  JEventLevel m_factory_level;


  std::vector<std::string> m_simcalorechit_collection_names = {
      "B0ECalRecHits",
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
      "LFHCALRecHits"
    };

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
      "LFHCALRecHits_aligned"
    };


  VariadicPodioInput<edm4eic::CalorimeterHit, true> m_calorechit_in{this,
                                                                 m_simcalorechit_collection_names};

  VariadicPodioOutput<edm4eic::CalorimeterHit> m_calorechit_out{this,
                                                             m_simcalorechit_collection_names_aligned};

  Double_t m_time_offset = 0.0; // Time offset to apply to hits

  void Configure() {}

  void ChangeRun(int32_t /*run_nr*/) {}

  void Process(int64_t run_number, uint64_t event_number) {

    for (size_t coll_index = 0; coll_index < m_calorechit_in().size(); ++coll_index) {
      const auto* coll_in = m_calorechit_in().at(coll_index);
      auto& coll_out      = m_calorechit_out().at(coll_index);

// std::cerr << "[KUMA_DEBUG] timeAlignment input collection = " << m_simcalorechit_collection_names.at(coll_index) << ", pointer = " << coll_in;
// if (coll_in != nullptr) std::cerr << ", size = " << coll_in->size();
// std::cerr << std::endl;

      if (coll_in != nullptr) {
        std::vector<edm4eic::MutableCalorimeterHit> sorted_hits; // for edm4eic (After digitization)
        for (const auto& hit : *coll_in) {
          edm4eic::MutableCalorimeterHit copiedHit = hit.clone(); // for edm4eic (After digitization)

          Double_t hitR      = std::sqrt(hit.getPosition()[0] * hit.getPosition()[0] +
                                         hit.getPosition()[1] * hit.getPosition()[1] +
                                         hit.getPosition()[2] * hit.getPosition()[2]);
          Double_t calibTime = hitR * 0.0034;
          copiedHit.setTime(hit.getTime() - calibTime);
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
