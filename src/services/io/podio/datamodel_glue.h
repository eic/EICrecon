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
#include <podio/CollectionBase.h>
#include <podio/utilities/TypeHelpers.h>

// Use umbrella headers if available (podio >= 1.3)
#include "services/io/podio/datamodel_includes.h"

// PodioTypeMap provides type traits for podio types
template <typename T> struct PodioTypeMap {
  using collection_t = typename T::collection_type;
  using mutable_t    = typename T::mutable_type;
};

// Helper to build a map of typename -> visitor function
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

// VisitPodioCollection: visitor pattern for type-safe collection processing
template <typename Visitor> struct VisitPodioCollection {
  void operator()(Visitor& visitor, const podio::CollectionBase& collection) {
    // Build the map once (static initialization)
    static const CollectionVisitorMap<Visitor> visitorMap;

    auto typeName = collection.getTypeName();
    auto it       = visitorMap.getMap().find(typeName);

    if (it != visitorMap.getMap().end()) {
      it->second(visitor, collection);
    } else {
      throw std::runtime_error(
        std::string("Unrecognized podio typename: ") + std::string(typeName) +
        ".\nThis type was not found in the supported datamodels (EDM4hep, EDM4eic).\n"
        "Possible causes:\n"
        "  - The type is not defined in EDM4hep or EDM4eic datamodels.\n"
        "  - You may be using an incompatible or outdated version of the datamodels.\n"
        "  - There may be a typo in the type name.\n"
        "Please check your datamodel installation and ensure you are using compatible versions."
      );
    }
  }
};
