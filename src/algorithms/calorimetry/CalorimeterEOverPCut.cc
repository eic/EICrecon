// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#include "CalorimeterEOverPCut.h"

#include <edm4hep/utils/vector_utils.h>

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>

#include <cmath>
#include <vector>

namespace eicrecon {

void CalorimeterEOverPCut::init() {
  if (m_cfg.readout.empty() || m_cfg.layerField.empty()) {
    m_id_dec    = nullptr;
    m_layer_idx = -1;
    return;
  }

  try {
    auto& det = dd4hep::Detector::getInstance();
    m_id_spec = det.readout(m_cfg.readout).idSpec();

    m_id_dec = m_id_spec.decoder();
    if (m_id_dec == nullptr) {
      warning("Failed to load ID decoder for readout {}", m_cfg.readout);
      m_layer_idx = -1;
      return;
    }

    m_layer_idx = m_id_dec->index(m_cfg.layerField);
  } catch (...) {
    warning("Failed to initialize cellID decoder for readout {} (field={})", m_cfg.readout,
            m_cfg.layerField);
    m_id_dec    = nullptr;
    m_layer_idx = -1;
  }
}

void CalorimeterEOverPCut::process(const Input& input, const Output& output) const {
  const auto& [clusters_notnull, matches_notnull, hits_notnull] = input;
  auto const& clusters                                          = *clusters_notnull;
  auto const& matches                                           = *matches_notnull;
  (void)hits_notnull;

  auto& [out_clusters_notnull, out_matches_notnull, out_pids_notnull] = output;
  auto& out_clusters                                                  = *out_clusters_notnull;
  auto& out_matches                                                   = *out_matches_notnull;
  auto& out_pids                                                      = *out_pids_notnull;

  for (auto const& in_cl : clusters) {

    edm4eic::MutableCluster out_cl = in_cl.clone();
    out_clusters.push_back(out_cl);

    bool found_match                      = false;
    edm4eic::TrackClusterMatch best_match = edm4eic::TrackClusterMatch::makeEmpty();

    for (auto const& m : matches) {
      if (m.getCluster() != in_cl) {
        continue;
      }

      auto out_m = m.clone();
      out_m.setCluster(out_cl);
      out_matches.push_back(out_m);

      if (!found_match || m.getWeight() > best_match.getWeight()) {
        best_match  = m;
        found_match = true;
      }
    }

    if (!found_match) {
      debug("No TrackClusterMatch for this cluster; skipping PID.");
      continue;
    }

    const double ptrack = edm4hep::utils::magnitude(best_match.getTrack().getMomentum());
    if (!(ptrack > 0.0)) {
      continue;
    }

    double energyInDepth = 0.0;

    if (m_cfg.maxLayer <= 0) {
      energyInDepth = in_cl.getEnergy();
    } else if (m_id_dec != nullptr && m_layer_idx >= 0) {
      for (auto const& hit : in_cl.getHits()) {
        const auto cellID = hit.getCellID();
        const int layer   = static_cast<int>(m_id_dec->get(cellID, m_layer_idx));
        if (layer <= m_cfg.maxLayer) {
          energyInDepth += hit.getEnergy();
        }
      }
    } else {
      energyInDepth = in_cl.getEnergy();
    }

    const double ep = energyInDepth / ptrack;

    if (ep > m_cfg.eOverPCut) {
      auto pid = out_pids.create(
          /* type= */ 0,
          /* PDG=  */ 11,
          /* algo= */ 0,
          /* like= */ 1.0f);

      pid.addToParameters(static_cast<float>(ep));
      out_cl.addToParticleIDs(pid);
    }
  }
}

} // namespace eicrecon
