// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <string>

namespace eicrecon {

/**
 * Clone collection elements to create a new standalone collection.
 *
 * This algorithm takes an input collection and creates an output collection
 * containing cloned copies of those elements. This is primarily useful for
 * subset collections (where elements point to objects in another collection)
 * to create an owning (non-subset) collection.
 *
 * \note Any relations carried by the elements are cloned as-is; if you write
 * the output without also writing the referenced collections, you may create
 * dangling references in the output file.
 *
 * \note While this works on any collection, cloning non-subset collections
 * creates unnecessary duplicates and is generally not recommended.
 *
 * Template parameter T is the PODIO object type (e.g., edm4eic::Cluster)
 */
template <typename T>
class Cloner : public algorithms::Algorithm<algorithms::Input<typename T::collection_type>,
                                            algorithms::Output<typename T::collection_type>> {

public:
  Cloner(std::string name)
      : algorithms::Algorithm<algorithms::Input<typename T::collection_type>,
                              algorithms::Output<typename T::collection_type>>(
            std::move(name), {"inputCollection"}, {"outputCollection"},
            "Clone collection elements to create standalone collection") {}

  void process(const typename Cloner::Input& input,
               const typename Cloner::Output& output) const final {
    const auto [in_coll] = input;
    auto [out_coll]      = output;

    // Clone each element from input to output
    for (const auto& obj : *in_coll) {
      out_coll->push_back(obj.clone());
    }
  }
};

} // namespace eicrecon
