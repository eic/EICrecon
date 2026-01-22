// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <TDirectory.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/Vector3d.h>
#include <cmath>
#include <cstddef>
#include <exception>
#include <map>
#include <memory>

#include "TrackingOccupancyAnalysis.h"

void TrackingOccupancyAnalysis::init(JApplication* /* app */, TDirectory* plugin_tdir) {
  auto* dir = plugin_tdir->mkdir("SimOccupancies"); // TODO create directory for this analysis

  auto z_limit_min = -2000;
  auto z_limit_max = 2000;
  auto r_limit_min = 0;
  auto r_limit_max = 1200;

  m_total_occup_th2 = new TH2F("total_occup", "Occupancy plot for all readouts", 200, z_limit_min,
                               +z_limit_max, 100, r_limit_min, r_limit_max);
  m_total_occup_th2->SetDirectory(dir);

  for (auto& name : m_data_names) {
    auto count_hist = std::make_shared<TH1F>(("count_" + name).c_str(),
                                             ("Count hits for " + name).c_str(), 100, 0, 30);
    count_hist->SetDirectory(dir);
    m_hits_count_hists.push_back(count_hist);

    auto occup_hist =
        std::make_shared<TH2F>(("occup_" + name).c_str(), ("Occupancy plot for" + name).c_str(),
                               100, z_limit_min, z_limit_max, 200, r_limit_min, r_limit_max);
    occup_hist->SetDirectory(dir);
    m_hits_occup_hists.push_back(occup_hist);
  }
}

void TrackingOccupancyAnalysis::process(const std::shared_ptr<const JEvent>& event) {

  for (std::size_t name_index = 0; name_index < m_data_names.size(); name_index++) {
    std::string data_name = m_data_names[name_index];
    auto& count_hist      = m_hits_count_hists[name_index];
    auto& occup_hist      = m_hits_occup_hists[name_index];

    try {
      auto* hits = event->GetCollection<edm4hep::SimTrackerHit>(data_name);
      count_hist->Fill(hits->size());
      for (const auto& hit : *hits) {
        float x = hit.getPosition().x;
        float y = hit.getPosition().y;
        float z = hit.getPosition().z;
        float r = sqrt(x * x + y * y);
        occup_hist->Fill(z, r);
        m_total_occup_th2->Fill(z, r);
      }
    } catch (std::exception& e) {
      // silently skip missing collections
    }
  }
}
