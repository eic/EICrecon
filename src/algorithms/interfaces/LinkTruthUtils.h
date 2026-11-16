// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#pragma once

#include <edm4hep/CaloHitContribution.h>
#include <edm4hep/MCParticle.h>
#include <gsl/pointers>
#include <podio/LinkNavigator.h>
#include <algorithm>
#include <cstdlib>
#include <optional>
#include <utility>
#include <vector>

namespace eicrecon::truth {

template <typename LinkCollectionT> class EventLinkNavigator {
public:
  explicit EventLinkNavigator(const LinkCollectionT* links)
      : m_enabled(links != nullptr && !links->empty()) {
    if (m_enabled) {
      m_nav.emplace(*links);
    }
  }

  bool enabled() const { return m_enabled; }
  template <typename SrcT> auto linked(const SrcT& src) const {
    using ReturnT = decltype(std::declval<podio::LinkNavigator<LinkCollectionT>>().getLinked(src));
    return m_enabled ? m_nav->getLinked(src) : ReturnT{};
  }

private:
  bool m_enabled = false;
  std::optional<podio::LinkNavigator<LinkCollectionT>> m_nav;
};

template <typename RecT, typename SimT, typename LinkCollT, typename AssocCollT>
inline void addWeightedRelation(const RecT& rec, const SimT& sim, float weight,
                                gsl::not_null<LinkCollT*> links,
                                gsl::not_null<AssocCollT*> assocs) {
  auto link = links->create();
  link.setFrom(rec);
  link.setTo(sim);
  link.setWeight(weight);

  auto assoc = assocs->create();
  assoc.setRec(rec);
  assoc.setSim(sim);
  assoc.setWeight(weight);
}

// Walk back through parents to find the primary particle responsible for a hit
// contribution, skipping over particles whose PDG code is in `promptDecayPDGs`
// (e.g. pi0) so that their stable/longer-lived descendants are reported instead.
inline edm4hep::MCParticle primaryFrom(const edm4hep::CaloHitContribution& contrib,
                                       const std::vector<int>& promptDecayPDGs) {
  edm4hep::MCParticle current = contrib.getParticle();
  if (!current.isAvailable()) {
    return current;
  }

  const edm4hep::MCParticle original = current;
  std::vector<edm4hep::MCParticle> chain{current};
  while (current.getGeneratorStatus() == 0 && current.parents_size() > 0) {
    const auto parent = current.getParents(0);
    if (!parent.isAvailable()) {
      break;
    }
    current = parent;
    chain.push_back(current);
  }

  const auto is_prompt_decay_particle = [&promptDecayPDGs](const edm4hep::MCParticle& particle) {
    return std::ranges::find(promptDecayPDGs, std::abs(particle.getPDG())) != promptDecayPDGs.end();
  };

  for (auto iterator = chain.rbegin(); iterator != chain.rend(); ++iterator) {
    if (!iterator->isAvailable()) {
      continue;
    }
    if (is_prompt_decay_particle(*iterator)) {
      continue;
    }
    return *iterator;
  }
  return original;
}

} // namespace eicrecon::truth
