// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#pragma once

#include <edm4hep/CaloHitContribution.h>
#include <edm4hep/MCParticle.h>
#include <podio/LinkNavigator.h>
#include <optional>
#include <utility>

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
inline void addWeightedRelation(const RecT& rec, const SimT& sim, float weight, LinkCollT* links,
                                AssocCollT* assocs) {
  auto link = links->create();
  link.setFrom(rec);
  link.setTo(sim);
  link.setWeight(weight);

  auto assoc = assocs->create();
  assoc.setRec(rec);
  assoc.setSim(sim);
  assoc.setWeight(weight);
}

inline edm4hep::MCParticle primaryFrom(const edm4hep::CaloHitContribution& contrib) {
  edm4hep::MCParticle primary = contrib.getParticle();
  while (primary.parents_size() > 0) {
    if (primary.getGeneratorStatus() != 0) {
      break;
    }
    primary = primary.getParents(0);
  }
  return primary;
}

} // namespace eicrecon::truth
