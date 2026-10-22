// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <spdlog/spdlog.h>
#include <algorithms/algorithm.h>
#include <functional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/meta/CopyAssociationsConfig.h"
#include "services/log/Log_service.h"

namespace eicrecon {

template <class SourceT, class SourceAssocT, class TargetT = SourceT, class TargetAssocT = SourceAssocT>
using CopyAssociationsAlgorithm = algorithms::Algorithm<
    typename algorithms::Input<const typename SourceT::collection_type,
                               const typename TargetT::collection_type,
                               const typename SourceAssocT::collection_type>,
    typename algorithms::Output<typename TargetAssocT::collection_type>>;

/*! Copy associations from one collection to
 *  associations from another collection.
 *
 *  TODO add some more words here
 *    - Naming convention:
 *        - source: what you're copying from
 *        - target: what you're copying to
 *        - linked: what you're associating with
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
template <typename SourceT, typename SourceAssocT, typename AssocT, typename TargetT = SourceT,
          typename TargetAssocT = SourceAssocT>
class CopyAssociations
    : public CopyAssociationsAlgorithm<SourceT, SourceAssocT, TargetT, TargetAssocT>,
      public WithPodConfig<
          CopyAssociationsConfig<SourceT, SourceAssocT, AssocT, TargetT, TargetAssocT>> {

public:
  CopyAssociations(std::string_view name)
      : CopyAssociationsAlgorithm<SourceT, SourceAssocT, TargetT, TargetAssocT>{
            name,
            {"inputSourceCollection", "inputTargetCollection", "inputSourceAssociations"},
            {"outputTargetAssociations"},
            "Copies associations from source collection onto associations from target collection"} {
  }

  void process(const typename CopyAssociationsAlgorithm<SourceT, SourceAssocT, TargetT,
                                                        TargetAssocT>::Input& input,
               const typename CopyAssociationsAlgorithm<SourceT, SourceAssocT, TargetT,
                                                        TargetAssocT>::Output& output) const final {

    const auto [in_sources, in_targets, in_source_assocs] = input;
    auto [out_target_assocs]                              = output;

    // exit if no associations in input collection
    if (in_source_assocs->size() == 0) {
      this->debug("No associations in input association collection.");
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

    for (std::size_t idx = 0; const auto& source : *in_sources) {
      for (const auto& source_assoc : *in_source_assocs) {
        const auto target = in_targets->at(idx);
        if (source == this->m_cfg.getSourceFrom(source_assoc)) {
          auto target_assoc = out_target_assocs->create();
          this->m_cfg.setTargetFrom(target, target_assoc);
          this->m_cfg.setTargetTo(this->m_cfg.getSourceTo(source_assoc), target_assoc);
          target_assoc.setWeight(source_assoc.getWeight());
        }
      }
      ++idx;
    }
  }
};

} // namespace eicrecon
