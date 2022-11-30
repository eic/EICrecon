// This file is part of the Acts project.
//
// Copyright (C) 2017-2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef H_JUG_GEOMETRY_CONTAINERS_H
#define H_JUG_GEOMETRY_CONTAINERS_H

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

#include "Utilities/GroupBy.hpp"
#include "Utilities/Range.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>


namespace eicrecon {
namespace detail {
// extract the geometry identifier from a variety of types
struct GeometryIdGetter {
  // explicit geometry identifier are just forwarded
  constexpr Acts::GeometryIdentifier operator()(Acts::GeometryIdentifier geometryId) const {
    return geometryId;
  }
  // encoded geometry ids are converted back to geometry identifiers.
  constexpr Acts::GeometryIdentifier operator()(Acts::GeometryIdentifier::Value encoded) const {
    return Acts::GeometryIdentifier(encoded);
  }
  // support elements in map-like structures.
  template <typename T>
  constexpr Acts::GeometryIdentifier operator()(
      const std::pair<Acts::GeometryIdentifier, T>& mapItem) const {
    return mapItem.first;
  }
  // support elements that implement `.geometryId()`.
  template <typename T>
  inline auto operator()(const T& thing) const
      -> decltype(thing.geometryId(), Acts::GeometryIdentifier()) {
    return thing.geometryId();
  }
  // support reference_wrappers around such types as well
  template <typename T>
  inline auto operator()(std::reference_wrapper<T> thing) const
      -> decltype(thing.get().geometryId(), Acts::GeometryIdentifier()) {
    return thing.get().geometryId();
  }
};

struct CompareGeometryId {
  // indicate that comparisons between keys and full objects are allowed.
  using is_transparent = void;
  // compare two elements using the automatic key extraction.
  template <typename Left, typename Right>
  constexpr bool operator()(Left&& lhs, Right&& rhs) const {
    return GeometryIdGetter()(lhs) < GeometryIdGetter()(rhs);
  }
};
}  // namespace detail

/// Store elements that know their detector geometry id, e.g. simulation hits.
///
/// @tparam T type to be stored, must be compatible with `CompareGeometryId`
///
/// The container stores an arbitrary number of elements for any geometry
/// id. Elements can be retrieved via the geometry id; elements can be selected
/// for a specific geometry id or for a larger range, e.g. a volume or a layer
/// within the geometry hierachy using the helper functions below. Elements can
/// also be accessed by index that uniquely identifies each element regardless
/// of geometry id.
template <typename T>
using GeometryIdMultiset =
    boost::container::flat_multiset<T, detail::CompareGeometryId>;

/// Store elements indexed by an geometry id.
///
/// @tparam T type to be stored
///
/// The behaviour is the same as for the `GeometryIdMultiset` except that the
/// stored elements do not know their geometry id themself. When iterating
/// the iterator elements behave as for the `std::map`, i.e.
///
///     for (const auto& entry: elements) {
///         auto id = entry.first; // geometry id
///         const auto& el = entry.second; // stored element
///     }
///
template <typename T>
using GeometryIdMultimap = GeometryIdMultiset<std::pair<Acts::GeometryIdentifier, T>>;

/// Select all elements within the given volume.
template <typename T>
inline Range<typename GeometryIdMultiset<T>::const_iterator> selectVolume(
    const GeometryIdMultiset<T>& container, Acts::GeometryIdentifier::Value volume) {
  auto cmp = Acts::GeometryIdentifier().setVolume(volume);
  auto beg = std::lower_bound(container.begin(), container.end(), cmp,
                              detail::CompareGeometryId{});
  // WARNING overflows to volume==0 if the input volume is the last one
  cmp = Acts::GeometryIdentifier().setVolume(volume + 1u);
  // optimize search by using the lower bound as start point. also handles
  // volume overflows since the geo id would be located before the start of
  // the upper edge search window.
  auto end =
      std::lower_bound(beg, container.end(), cmp, detail::CompareGeometryId{});
  return makeRange(beg, end);
}
template <typename T>
inline auto selectVolume(const GeometryIdMultiset<T>& container,
                         Acts::GeometryIdentifier id) {
  return selectVolume(container, id.volume());
}

/// Select all elements within the given layer.
template <typename T>
inline Range<typename GeometryIdMultiset<T>::const_iterator> selectLayer(
    const GeometryIdMultiset<T>& container, Acts::GeometryIdentifier::Value volume,
    Acts::GeometryIdentifier::Value layer) {
  auto cmp = Acts::GeometryIdentifier().setVolume(volume).setLayer(layer);
  auto beg = std::lower_bound(container.begin(), container.end(), cmp,
                              detail::CompareGeometryId{});
  // WARNING resets to layer==0 if the input layer is the last one
  cmp = Acts::GeometryIdentifier().setVolume(volume).setLayer(layer + 1u);
  // optimize search by using the lower bound as start point. also handles
  // volume overflows since the geo id would be located before the start of
  // the upper edge search window.
  auto end =
      std::lower_bound(beg, container.end(), cmp, detail::CompareGeometryId{});
  return makeRange(beg, end);
}
template <typename T>
inline auto selectLayer(const GeometryIdMultiset<T>& container,
                        Acts::GeometryIdentifier id) {
  return selectLayer(container, id.volume(), id.layer());
}

/// Select all elements for the given module / sensitive surface.
template <typename T>
inline Range<typename GeometryIdMultiset<T>::const_iterator> selectModule(
    const GeometryIdMultiset<T>& container, Acts::GeometryIdentifier geoId) {
  // module is the lowest level and defines a single geometry id value
  return makeRange(container.equal_range(geoId));
}
template <typename T>
inline auto selectModule(const GeometryIdMultiset<T>& container,
                         Acts::GeometryIdentifier::Value volume,
                         Acts::GeometryIdentifier::Value layer,
                         Acts::GeometryIdentifier::Value module) {
  return selectModule(
      container,
      Acts::GeometryIdentifier().setVolume(volume).setLayer(layer).setSensitive(
          module));
}

/// Iterate over groups of elements belonging to each module/ sensitive surface.
template <typename T>
inline GroupBy<typename GeometryIdMultiset<T>::const_iterator,
               detail::GeometryIdGetter>
groupByModule(const GeometryIdMultiset<T>& container) {
  return makeGroupBy(container, detail::GeometryIdGetter());
}

/// The accessor for the GeometryIdMultiset container
///
/// It wraps up a few lookup methods to be used in the Combinatorial Kalman
/// Filter
template <typename T>
struct GeometryIdMultisetAccessor {
  using Container = GeometryIdMultiset<T>;
  using Key = Acts::GeometryIdentifier;
  using Value = typename GeometryIdMultiset<T>::value_type;
  using Iterator = typename GeometryIdMultiset<T>::const_iterator;

  // pointer to the container
  const Container* container = nullptr;

  // get the range of elements with requested geoId
  std::pair<Iterator, Iterator> range(const Acts::Surface& surface) const {
    assert(container != nullptr);
    return container->equal_range(surface.geometryId());
  }
};

}  // namespace FW

#endif // H_JUG_GEOMETRY_CONTAINERS_H