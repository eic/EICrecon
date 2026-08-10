// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <algorithms/algorithm.h>

namespace eicrecon {

template <typename HitCollectionT>
using RecHitTimeAlignmentAlgorithm =
    algorithms::Algorithm<algorithms::Input<HitCollectionT>, algorithms::Output<HitCollectionT>>;

template <typename HitCollectionT, typename MutableHitT>
class RecHitTimeAlignment : public RecHitTimeAlignmentAlgorithm<HitCollectionT> {
public:
  using AlgorithmT = RecHitTimeAlignmentAlgorithm<HitCollectionT>;

  RecHitTimeAlignment(std::string_view name, std::string input_name, std::string output_name,
                      std::string description)
      : AlgorithmT{name, {std::move(input_name)}, {std::move(output_name)},
                   std::move(description)} {}

  void init() final {}

  void process(const typename AlgorithmT::Input& input,
               const typename AlgorithmT::Output& output) const final {
    const auto [hits_in] = input;
    auto [hits_out]      = output;

    std::vector<MutableHitT> sorted_hits;
    sorted_hits.reserve(hits_in->size());
    for (const auto& hit : *hits_in) {
      MutableHitT copied_hit = hit.clone();
      const auto position    = hit.getPosition();
      const auto hit_r       = std::sqrt(position[0] * position[0] +
                                   position[1] * position[1] + position[2] * position[2]);
      const auto calibrated_time = hit_r * m_refInverseVelocity;
      copied_hit.setTime(hit.getTime() - calibrated_time);
      sorted_hits.push_back(copied_hit);
    }

    std::sort(sorted_hits.begin(), sorted_hits.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.getTime() < rhs.getTime(); });

    for (const auto& hit : sorted_hits) {
      hits_out->push_back(hit);
    }
  }

  private:
    double m_refInverseVelocity = 0.0034; // ns/mm
  
};

} // namespace eicrecon
