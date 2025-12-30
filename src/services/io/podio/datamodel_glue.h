// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck
//
// Modern datamodel glue for podio >= 1.3
// This file uses podio's built-in TypeList support instead of code generation

#pragma once

#include <stdexcept>
#include <string_view>
#include <map>
#include <type_traits>
#include <fmt/format.h>
#include <podio/CollectionBase.h>
#include <podio/utilities/TypeHelpers.h>

// Use umbrella headers if available (podio >= 1.3)
#include "services/io/podio/datamodel_includes.h"

// PodioTypeMap provides type traits for podio types
// This mirrors the structure written by the legacy python generator,
// and puts the types in the format expected by JANA2.
template <typename T> struct PodioTypeMap {
  using collection_t = typename T::collection_type;
  using mutable_t    = typename T::mutable_type;
};

// CollectionVisitorMap builds a dispatch table from podio collection type names
// to type-safe visitor functions for a given Visitor type.
//
// The Visitor template parameter is expected to be a callable type (e.g. a
// functor, lambda, or class with operator()) that can be invoked for each
// supported concrete collection type:
//
//   void operator()(const ConcreteCollectionType&);
//
// CollectionVisitorMap is instantiated once per Visitor (see VisitPodioCollection
// below). At construction time it registers all EDM4hep and EDM4eic data and
// link collection types by mapping their static typeName to a function that
// will:
//   1. downcast the generic podio::CollectionBase reference to the concrete
//      collection type, and
//   2. call the Visitor with that concrete collection.
//
// This map is then used by VisitPodioCollection to look up the correct
// handler at runtime based on collection.getTypeName(), providing a
// type-safe implementation of the visitor pattern over podio collections.
template <typename Visitor> class CollectionVisitorMap {
public:
  using FunctionType = void (*)(Visitor&, const podio::CollectionBase&);

private:
  std::map<std::string_view, FunctionType> m_map;

  template <typename CollectionT>
  static void visitCollection(Visitor& visitor, const podio::CollectionBase& collection) {
    static_assert(std::is_base_of_v<podio::CollectionBase, CollectionT>,
                  "CollectionT must be derived from podio::CollectionBase");
    visitor(*static_cast<const CollectionT*>(&collection));
  }

  template <typename CollectionT> void addToMap() {
    m_map[CollectionT::typeName] = &visitCollection<CollectionT>;
  }

  template <typename DataT> void addDataTypeToMap() { addToMap<typename DataT::collection_type>(); }

  template <typename... DataTypes>
  void addAllDataCollections(podio::utils::TypeList<DataTypes...>) {
    (addDataTypeToMap<DataTypes>(), ...);
  }

  template <typename... LinkTypes>
  void addAllLinkCollections(podio::utils::TypeList<LinkTypes...>) {
    (addDataTypeToMap<LinkTypes>(), ...);
  }

public:
  CollectionVisitorMap() {
    // Add all EDM4hep and EDM4eic data collection types
    addAllDataCollections(typename edm4hep::edm4hepDataTypes{});
    addAllDataCollections(typename edm4eic::edm4eicDataTypes{});

    // Add all EDM4hep and EDM4eic link collections
    addAllLinkCollections(typename edm4hep::edm4hepLinkTypes{});
    addAllLinkCollections(typename edm4eic::edm4eicLinkTypes{});
  }

  const auto& getMap() const { return m_map; }
};

// VisitPodioCollection is a visitor adaptor for type-safe processing of podio collections.
//
// This template implements a runtime-dispatch visitor pattern for
// podio::CollectionBase instances. It uses the collection's
// runtime type name together with CollectionVisitorMap to look up the
// concrete collection type and call the provided @p Visitor on it.
//
// template parameter Visitor
//   A type that models a callable taking a const reference to each
//   supported concrete podio collection type. In practice, this means
//   that for every collection type CollectionT registered in
//   CollectionVisitorMap, the following expression must be well-formed:
//
//     visitor(const CollectionT&);
//
//   This can be satisfied either by giving Visitor a suitable
//   operator() overload, or by providing free / member functions
//   that are invocable with const CollectionT&.
//
// Usage:
//   - Construct or obtain a Visitor instance.
//   - Call VisitPodioCollection<Visitor>::operator() with the visitor
//     and a podio::CollectionBase reference. The adaptor will:
//       1. Look up the collection's concrete type by getTypeName().
//       2. Cast the podio::CollectionBase to the matching concrete
//          collection type.
//       3. Invoke the visitor with a const CollectionT& argument.
//
// If the collection type name is not known to the registered EDM4hep or
// EDM4eic datamodels, operator() throws std::runtime_error with a
// descriptive error message.
template <typename Visitor> struct VisitPodioCollection {
  void operator()(Visitor& visitor, const podio::CollectionBase& collection) {
    // Build the map once (static initialization)
    static const CollectionVisitorMap<Visitor> visitorMap;

    auto typeName = collection.getTypeName();
    auto it       = visitorMap.getMap().find(typeName);

    if (it != visitorMap.getMap().end()) {
      it->second(visitor, collection);
    } else {
      throw std::runtime_error(fmt::format(
          "Unrecognized podio typename: {}.\n"
          "This type was not found in the supported datamodels (EDM4hep, EDM4eic).\n"
          "Possible causes:\n"
          "  - The type is not defined in EDM4hep or EDM4eic datamodels.\n"
          "  - You may be using an incompatible or outdated version of the datamodels.\n"
          "  - There may be a typo in the type name.\n"
          "Please check your datamodel installation and ensure you are using compatible versions.",
          typeName));
    }
  }
};
