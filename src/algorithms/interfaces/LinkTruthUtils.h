// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#pragma once

#include <edm4hep/CaloHitContribution.h>
#include <edm4hep/MCParticle.h>
#include <gsl/pointers>
#include <podio/LinkNavigator.h>
#include <memory>
#include <utility>

namespace eicrecon::truth {

template <typename LinkCollectionT> class EventLinkNavigator {
public:
  explicit EventLinkNavigator(const LinkCollectionT* links) {
    if (links != nullptr && !links->empty()) {
      m_nav = std::make_unique<const podio::LinkNavigator<LinkCollectionT>>(*links);
    }
  }

  bool enabled() const { return m_nav != nullptr; }
  template <typename SrcT> auto linked(const SrcT& src) const {
    using ReturnT = decltype(std::declval<const podio::LinkNavigator<LinkCollectionT>>().getLinked(src));
    return m_nav ? m_nav->getLinked(src) : ReturnT{};
  }

private:
  std::unique_ptr<const podio::LinkNavigator<LinkCollectionT>> m_nav;
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
