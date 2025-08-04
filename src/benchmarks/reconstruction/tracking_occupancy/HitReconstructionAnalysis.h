// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JEvent.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <memory>
#include <string>
#include <vector>

class HitReconstructionAnalysis {
public:
  void init(JApplication* app, TDirectory* plugin_tdir);

  void process(const std::shared_ptr<const JEvent>& event);

private:
  /// This is edm4hep::SimTrackerHits names of different detector readouts
  std::vector<std::string> m_data_names = {
      "SiBarrelTrackerRecHits", // Barrel Tracker
      "SiBarrelVertexRecHits",  // Vertex
      "SiEndcapTrackerRecHits", // End Cap tracker
                                // MPGD
      "MPGDBarrelRecHits",
      "OuterMPGDBarrelRecHits",
      "ForwardMPGDEndcapRecHits",
      "BackwardMPGDEndcapRecHits",
      // TOF
      "TOFEndcapRecHits",
      "TOFBarrelRecHit",
  };

  /// Hits count histogram for each hits readout name
  std::vector<std::shared_ptr<TH1F>> m_hits_count_hists;

  /// Hits occupancy histogram for each hits readout name
  std::vector<std::shared_ptr<TH2F>> m_hits_occup_hists;

  /// Total occupancy of all m_data_names
  TH2F* m_total_occup_th2; /// MC Particles px,py
};
