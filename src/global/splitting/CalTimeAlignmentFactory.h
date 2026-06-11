// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <extensions/jana/JOmniFactory.h>

#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>

struct CalTimeAlignmentFactory : public JOmniFactory<CalTimeAlignmentFactory> {
  JEventLevel m_factory_level;


  std::vector<std::string> m_simcalocluster_collection_names = {
      "B0ECalClusters_TK",
      "EcalBarrelClusters_TK",
      "EcalEndcapNClusters_TK",
      "EcalEndcapPClusters_TK"
    };
    // "HcalBarrelClusters_TK",
    // "HcalEndcapNClusters_TK",
    // "HcalEndcapPInsertClusters_TK",
    // "EcalFarForwardZDCClusters_TK",
    // "HcalFarForwardZDCClusters_TK",
    // "EcalFarForwardZDCImagingClusters_TK",
    // "EcalLumiSpecImagingClusters_TK"
    // "LFHCALClusters_TK",
    // "EcalLumiSpecClusters_TK",
    // "EcalBarrelImagingClusters_TK",
    // "EcalBarrelScFiClusters_TK",
    // "EcalEndcapNImagingClusters_TK",
    // "EcalEndcapPImagingClusters_TK",

    std::vector<std::string> m_simcalocluster_collection_names_aligned = {
      "B0ECalClusters_TK_aligned",
      "EcalBarrelClusters_TK_aligned",
      "EcalEndcapNClusters_TK_aligned",
      "EcalEndcapPClusters_TK_aligned"
    };
    // "LFHCALClusterAssociations_TK_aligned",
    // "HcalFarForwardZDCClusterAssociations_TK_aligned",
    // "EcalBarrelScFiClusterAssociations_TK_aligned"
    // "EcalBarrelImagingClusterAssociations_TK_aligned",
    // "HcalBarrelClusterAssociations_TK_aligned",
    // "HcalEndcapNClusterAssociations_TK_aligned",
    // "HcalEndcapPInsertClusterAssociations_TK_aligned",


  VariadicPodioInput<edm4eic::Cluster, true> m_calocluster_in{this,
                                                                 m_simcalocluster_collection_names};

  VariadicPodioOutput<edm4eic::Cluster> m_calocluster_out{this,
                                                             m_simcalocluster_collection_names_aligned};

  Double_t m_time_offset = 0.0; // Time offset to apply to hits

  void Configure() {}

  void ChangeRun(int32_t /*run_nr*/) {}

  void Process(int64_t run_number, uint64_t event_number) {
    std::cout << "<<<<<<<<<<<<Time Alignment Factory: Event " << event_number << std::endl;

    for (size_t coll_index = 0; coll_index < m_calocluster_in().size(); ++coll_index) {
      const auto* coll_in = m_calocluster_in().at(coll_index);
      auto& coll_out      = m_calocluster_out().at(coll_index);
      if (coll_in != nullptr) {
        std::vector<edm4eic::MutableCluster> sorted_hits; // for edm4eic (After digitization)
        for (const auto& hit : *coll_in) {
          edm4eic::MutableCluster copiedHit = hit.clone(); // for edm4eic (After digitization)

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
