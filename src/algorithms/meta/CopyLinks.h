// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <spdlog/spdlog.h>
#include <algorithms/algorithm.h>
#include <podio/LinkNavigator.h>
#include <string>
#include <string_view>
#include <type_traits>

#include "algorithms/interfaces/WithPodConfig.h"
#include "services/log/Log_service.h"

namespace eicrecon {

template <class SourceT, class SourceLinkT, class TargetT = SourceT,
          class TargetLinkT = SourceLinkT>
using CopyLinksAlgorithm =
    algorithms::Algorithm<typename algorithms::Input<const typename SourceT::collection_type,
                                                     const typename TargetT::collection_type,
                                                     const typename SourceLinkT::collection_type>,
                          typename algorithms::Output<typename TargetLinkT::collection_type>>;

/*! Copy links from one collection to links from
 *  another collection.
 *
 *  Schematically, this operation looks like:
 *
 *      source --------> target
 *        |                |
 *      source link      target link
 *        |                |
 *        v                v
 *      link   --------> link
 *
 *  Here, "source" is the input collection you're copying from,
 *  "target" is the the collection you're copying to, and "link"
 *  is what you're linking to.
 *
 *  Algorithm is templated on 2 - 4 types:
 *    - `SourceT`, type of source;
 *    - `SourceLinkT`, type of source link;
 *    - `TargetT`, type of target (defaults to `SourceT`); and
 *    - `TargetLinkT`, type of target link.
 *
 *  For example, if source and target are the same type one
 *  would write:
 *
 *      CopyLinks<Cluster, TrackClusterLink> algo;
 *
 *  And if they differed, one would write:
 *
 *      CopyLinks<ProtoCluster, TrackProtoClusterLink,
 *                Cluster, TrackClusterLink> algo;
 *
 *  \note Source and target collections are assumed to be
 *    1-to-1, i.e. that the Nth object in the source
 *    collection corresponds to the Nth object in the target
 *    collection.
 *
 *  \note Algorithm assumes that links are a different
 *    type than source or target.
 */
template <typename SourceT, typename SourceLinkT, typename TargetT = SourceT,
          typename TargetLinkT = SourceLinkT>
class CopyLinks : public CopyLinksAlgorithm<SourceT, SourceLinkT, TargetT, TargetLinkT>,
                  public WithPodConfig<NoConfig> {

public:
  CopyLinks(std::string_view name)
      : CopyLinksAlgorithm<SourceT, SourceLinkT, TargetT, TargetLinkT>{
            name,
            {"inputSourceCollection", "inputTargetCollection", "inputSourceLinks"},
            {"outputTargetLinks"},
            "Copies links from source collection onto links from target collection"} {}

  void process(
      const typename CopyLinksAlgorithm<SourceT, SourceLinkT, TargetT, TargetLinkT>::Input& input,
      const typename CopyLinksAlgorithm<SourceT, SourceLinkT, TargetT, TargetLinkT>::Output& output)
      const final {

    const auto [in_sources, in_targets, in_source_links] = input;
    auto [out_target_links]                              = output;

    // exit if no links in input collection
    if (in_source_links->size() == 0) {
      this->debug("No links in input link collection.");
      return;
    }

    // exit if source/target collection sizes
    // are different
    //   --> 1-to-1 ordering can't be assumed!
    if (in_sources->size() != in_targets->size()) {
      this->error("Size of source collection ({}) not the same as size of target collection ({})",
                  in_sources->size(), in_targets->size());
      return;
    }

    const auto navigator = podio::LinkNavigator(*in_source_links);
    for (std::size_t idx = 0; const auto& source : *in_sources) {
      const auto target         = in_targets->at(idx);
      const auto source_linkees = navigator.getLinked(source);
      for (const auto& [source_linkee, source_weight] : source_linkees) {
        auto target_link = out_target_links->create();
        target_link.template set<TargetT>(target);
        target_link.template set<std::remove_cvref_t<decltype(source_linkee)>>(source_linkee);
        target_link.setWeight(source_weight);
      }
      ++idx;
    }
  }
};

} // namespace eicrecon
