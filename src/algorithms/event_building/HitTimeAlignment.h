// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#pragma once

#include <algorithm>
#include <cmath>
#include <string_view>
#include <vector>

#include <algorithms/algorithm.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/event_building/HitTimeAlignmentConfig.h"

namespace eicrecon {

template <typename HitT>
using HitTimeAlignmentAlgorithm =
    algorithms::Algorithm<algorithms::Input<const typename HitT::collection_type>,
                          algorithms::Output<typename HitT::collection_type>>;

template <typename HitT>
class HitTimeAlignment : public HitTimeAlignmentAlgorithm<HitT>,
                         public WithPodConfig<HitTimeAlignmentConfig> {
public:
  using AlgorithmT = HitTimeAlignmentAlgorithm<HitT>;

  explicit HitTimeAlignment(std::string_view name)
      : AlgorithmT{name,
                   {"inputHits"},
                   {"outputHits"},
                   "Correct reconstructed hit times for propagation and sort by time."} {}

  void init() final {}

  void process(const typename AlgorithmT::Input& input,
               const typename AlgorithmT::Output& output) const final {
    const auto [hits_in] = input;
    auto [hits_out]      = output;

    std::vector<typename HitT::mutable_type> sorted_hits;
    sorted_hits.reserve(hits_in->size());
    for (const auto& hit : *hits_in) {
      auto copied_hit     = hit.clone();
      const auto position = hit.getPosition();
      const auto radius   = std::sqrt(position[0] * position[0] + position[1] * position[1] +
                                      position[2] * position[2]);
      const auto average_time_of_flight = radius * this->m_cfg.reference_inverse_velocity;
      copied_hit.setTime(hit.getTime() - average_time_of_flight);
      sorted_hits.push_back(copied_hit);
    }

    std::stable_sort(sorted_hits.begin(), sorted_hits.end(), [](const auto& lhs, const auto& rhs) {
      return lhs.getTime() < rhs.getTime();
    });

    for (const auto& hit : sorted_hits) {
      hits_out->push_back(hit);
    }
  }
};

} // namespace eicrecon
