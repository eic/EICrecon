// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <podio/LinkNavigator.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

template <class TSource, class TSourceLink, class TTarget = TSource, class TTargetLink = TSourceLink>
using CopyLinksAlgorithm =
    algorithms::Algorithm<typename algorithms::Input<const typename TSource::collection_type,
                                                     const typename TTarget::collection_type,
                                                     const typename TSourceLink::collection_type>,
                          typename algorithms::Output<const typename TTargetLink::collection_type>>;

/*! Copy links from one collection to links
 *  from another collection.
 *
 *  TODO add some more words here
 *    - Naming convention:
 *        - source: what you're copying from
 *        - target: what you're copying to
 *    - Algo works for subset-to-set (cloner)
 *    - And for set-to-set (e.g. protocluster
 *      to cluster)
 *
 *  \note Input and Output collections are
 *    assumed to be 1-to-1, i.e. that the
 *    Nth object in the output corresponds
 *    to the Nth object in the input.
 *
 *  \note TODO mention templating convention
 */
template <typename TSource, typename TSourceLink, typename TTarget = TSource,
          typename TTargetLink = TSourceLink>
class CopyLinks : public CopyLinksAlgorithm<TSource, TSourceLink, TTarget, TTargetLink>,
                  public WithPodConfig<NoConfig> {

public:
  CopyLinks(std::string_view name)
      : CopyLinksAlgorithm<TSource, TSourceLink, TTarget, TTargetLink>{
            name,
            {"inputSourceCollection", "inputTargetCollection", "inputSourceLinks"},
            {"outputTargetLinks"},
            "Copies links from source collection onto links from target collection"} {}

  void process(
      const typename CopyLinksAlgorithm<TSource, TSourceLink, TTarget, TTargetLink>::Input& input,
      const typename CopyLinksAlgorithm<TSource, TSourceLink, TTarget, TTargetLink>::Output& output)
      const final {

    const auto [in_sources, in_targets, in_source_links] = input;
    auto [out_target_links]                              = output;

    // exit if no links in input collection
    if (in_source_links->size() == 0) {
      debug("No links in input link collection.");
      return;
    }

    // exit if source/target collection sizes
    // are different
    //   --> 1-to-1 ordering can't be assumed!
    if (in_sources->size() != in_targets->size()) {
      error("Size of source collection ({}) not the same as size of target collection ({})",
            in_sources->size(), in_targets->size());
      return;
    }

    const auto navigator = podio::LinkNavigator(in_source_links);
    for (std::size_t idx = 0; const auto& source : in_sources) {
      const auto target       = in_targets[idx];
      const auto source_links = navigator.getLinked(source);
      for (const auto source_link : source_links) {
        auto target_link = out_target_links->create();
        target_link.set<TTarget>(target);
        target_link.set<TTargetLink>(source_link);
      }
      ++idx;
    }
  }
};

} // namespace eicrecon
