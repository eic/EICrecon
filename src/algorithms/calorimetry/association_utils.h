// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck
//
// Temporary fallback header for association_utils
// This allows EICrecon CI to work before edm4eic updates are available
// TODO: Remove this file once edm4eic with association_utils is in the standard stack

#ifndef EICRECON_ALGORITHMS_ASSOCIATION_UTILS_FALLBACK_HH
#define EICRECON_ALGORITHMS_ASSOCIATION_UTILS_FALLBACK_HH

// Check if edm4eic has the association_utils header
#if __has_include(<edm4eic/association_utils.h>)
// Use the official edm4eic version
#include <edm4eic/association_utils.h>
#else
// Fallback: provide local implementation for CI compatibility
// Note: Using local fallback - edm4eic version not found in include path

#include <podio/ObjectID.h>
#include <unordered_map>
#include <vector>
#include <ranges>
#include <type_traits>

// Define necessary hash functions
namespace std {

#if defined(podio_VERSION_MAJOR) && defined(podio_VERSION_MINOR)
#if podio_VERSION <= PODIO_VERSION(1, 2, 0)
// Hash for podio::ObjectID
template <> struct hash<podio::ObjectID> {
  size_t operator()(const podio::ObjectID& id) const noexcept {
    size_t h1 = std::hash<uint32_t>{}(id.collectionID);
    size_t h2 = std::hash<int>{}(id.index);
    return h1 ^ (h2 << 1);
  }
};
#endif // podio version check
#endif // defined(podio_VERSION_MAJOR) && defined(podio_VERSION_MINOR)

} // namespace std

namespace edm4eic {

/**
   * @brief Helper class to efficiently lookup associations between objects.
   *
   * This is a TEMPORARY fallback implementation for CI compatibility.
   * The canonical version lives in edm4eic/utils/include/edm4eic/association_utils.h
   *
   * Provides O(1) lookup time for associations by building hash maps from association
   * collections. Handles both one-to-one and one-to-many relationships.
   *
   * @tparam AssociationCollection Type of the association collection
   * @tparam GetFromObjectFunc Function to extract 'from' object from association
   * @tparam GetToObjectFunc Function to extract 'to' object from association
   */
template <typename AssociationCollection, typename GetFromObjectFunc, typename GetToObjectFunc>
class association_lookup {
public:
  using from_object_t = std::decay_t<decltype(std::declval<GetFromObjectFunc>()(
      *std::declval<const AssociationCollection*>()->begin()))>;
  using to_object_t   = std::decay_t<decltype(std::declval<GetToObjectFunc>()(
      *std::declval<const AssociationCollection*>()->begin()))>;

  using from_to_map_t = std::unordered_map<podio::ObjectID, std::vector<to_object_t>>;
  using to_from_map_t = std::unordered_map<podio::ObjectID, std::vector<from_object_t>>;

  static constexpr bool different_types = !std::is_same_v<from_object_t, to_object_t>;

  association_lookup(const AssociationCollection* associations, GetFromObjectFunc get_from_object,
                     GetToObjectFunc get_to_object)
      : m_get_from_object(get_from_object), m_get_to_object(get_to_object) {
    if (associations) {
      build_maps(associations);
    }
  }

  const std::vector<to_object_t>& operator[](const from_object_t& from_obj) const
    requires different_types
  {
    return lookup_from_to(from_obj);
  }

  const std::vector<to_object_t>& lookup_from_to(const from_object_t& from_obj) const {
    auto it = m_from_to_map.find(from_obj.getObjectID());
    if (it != m_from_to_map.end()) {
      return it->second;
    }
    static const std::vector<to_object_t> empty;
    return empty;
  }

  const std::vector<from_object_t>& lookup_to_from(const to_object_t& to_obj) const {
    auto it = m_to_from_map.find(to_obj.getObjectID());
    if (it != m_to_from_map.end()) {
      return it->second;
    }
    static const std::vector<from_object_t> empty;
    return empty;
  }

  bool has_from_associations(const from_object_t& from_obj) const {
    return m_from_to_map.find(from_obj.getObjectID()) != m_from_to_map.end();
  }

  bool has_to_associations(const to_object_t& to_obj) const {
    return m_to_from_map.find(to_obj.getObjectID()) != m_to_from_map.end();
  }

  const from_to_map_t& from_to_view() const { return m_from_to_map; }
  const to_from_map_t& to_from_view() const { return m_to_from_map; }

  size_t size() const { return m_total_associations; }
  bool empty() const { return m_total_associations == 0; }

  auto begin() const { return m_from_to_map.begin(); }
  auto end() const { return m_from_to_map.end(); }
  auto cbegin() const { return m_from_to_map.cbegin(); }
  auto cend() const { return m_from_to_map.cend(); }

private:
  void build_maps(const AssociationCollection* associations) {
    for (const auto& assoc : *associations) {
      auto from_obj = m_get_from_object(assoc);
      auto to_obj   = m_get_to_object(assoc);

      m_from_to_map[from_obj.getObjectID()].push_back(to_obj);
      m_to_from_map[to_obj.getObjectID()].push_back(from_obj);
      ++m_total_associations;
    }
  }

  GetFromObjectFunc m_get_from_object;
  GetToObjectFunc m_get_to_object;

  from_to_map_t m_from_to_map;
  to_from_map_t m_to_from_map;
  size_t m_total_associations = 0;
};

template <typename AssociationCollection, typename GetFromObjectFunc, typename GetToObjectFunc>
association_lookup<AssociationCollection, GetFromObjectFunc, GetToObjectFunc>
make_association_lookup(const AssociationCollection* associations,
                        GetFromObjectFunc get_from_object, GetToObjectFunc get_to_object) {
  return association_lookup<AssociationCollection, GetFromObjectFunc, GetToObjectFunc>(
      associations, get_from_object, get_to_object);
}

} // namespace edm4eic

#endif // __has_include check

#endif // EICRECON_ALGORITHMS_ASSOCIATION_UTILS_FALLBACK_HH
