// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#ifndef EDM4NWB_CalorimeterHitCollection_H
#define EDM4NWB_CalorimeterHitCollection_H

// datamodel specific includes
#include "edm4nwb/CalorimeterHitData.h"
#include "edm4nwb/CalorimeterHit.h"
#include "edm4nwb/MutableCalorimeterHit.h"
#include "edm4nwb/CalorimeterHitObj.h"
#include "edm4nwb/CalorimeterHitCollectionData.h"

// podio specific includes
#include "podio/ICollectionProvider.h"
#include "podio/CollectionBase.h"
#include "podio/CollectionIDTable.h"

#ifdef PODIO_JSON_OUTPUT
#include "nlohmann/json.hpp"
#endif

#include <string>
#include <vector>
#include <deque>
#include <array>
#include <algorithm>
#include <ostream>
#include <mutex>
#include <memory>

namespace edm4nwb {



class CalorimeterHitCollectionIterator {
public:
  CalorimeterHitCollectionIterator(size_t index, const CalorimeterHitObjPointerContainer* collection) : m_index(index), m_object(nullptr), m_collection(collection) {}

  CalorimeterHitCollectionIterator(const CalorimeterHitCollectionIterator&) = delete;
  CalorimeterHitCollectionIterator& operator=(const CalorimeterHitCollectionIterator&) = delete;

  bool operator!=(const CalorimeterHitCollectionIterator& x) const {
    return m_index != x.m_index; // TODO: may not be complete
  }

  CalorimeterHit operator*();
  CalorimeterHit* operator->();
  CalorimeterHitCollectionIterator& operator++();

private:
  size_t m_index;
  CalorimeterHit m_object;
  const CalorimeterHitObjPointerContainer* m_collection;
};


class CalorimeterHitMutableCollectionIterator {
public:
  CalorimeterHitMutableCollectionIterator(size_t index, const CalorimeterHitObjPointerContainer* collection) : m_index(index), m_object(nullptr), m_collection(collection) {}

  CalorimeterHitMutableCollectionIterator(const CalorimeterHitMutableCollectionIterator&) = delete;
  CalorimeterHitMutableCollectionIterator& operator=(const CalorimeterHitMutableCollectionIterator&) = delete;

  bool operator!=(const CalorimeterHitMutableCollectionIterator& x) const {
    return m_index != x.m_index; // TODO: may not be complete
  }

  MutableCalorimeterHit operator*();
  MutableCalorimeterHit* operator->();
  CalorimeterHitMutableCollectionIterator& operator++();

private:
  size_t m_index;
  MutableCalorimeterHit m_object;
  const CalorimeterHitObjPointerContainer* m_collection;
};


/**
A Collection is identified by an ID.
*/
class CalorimeterHitCollection : public podio::CollectionBase {
public:
  using const_iterator = CalorimeterHitCollectionIterator;
  using iterator = CalorimeterHitMutableCollectionIterator;

  CalorimeterHitCollection();
  // This is a move-only type
  CalorimeterHitCollection(const CalorimeterHitCollection& ) = delete;
  CalorimeterHitCollection& operator=(const CalorimeterHitCollection& ) = delete;
  CalorimeterHitCollection(CalorimeterHitCollection&&) = default;
  CalorimeterHitCollection& operator=(CalorimeterHitCollection&&) = default;

//  CalorimeterHitCollection(CalorimeterHitVector* data, int collectionID);
  ~CalorimeterHitCollection();

  void clear() final;

  /// operator to allow pointer like calling of members a la LCIO
  CalorimeterHitCollection* operator->() { return (CalorimeterHitCollection*) this; }

  /// Append a new object to the collection, and return this object.
  MutableCalorimeterHit create();

  /// Append a new object to the collection, and return this object.
  /// Initialized with the parameters given
  template<typename... Args>
  MutableCalorimeterHit create(Args&&... args);

  /// number of elements in the collection
  size_t size() const final;

  /// fully qualified type name
  std::string getTypeName() const final { return std::string("edm4nwb::CalorimeterHitCollection"); }
  /// fully qualified type name of elements - with namespace
  std::string getValueTypeName() const final { return std::string("edm4nwb::CalorimeterHit"); }
  /// fully qualified type name of stored POD elements - with namespace
  std::string getDataTypeName() const final { return std::string("edm4nwb::CalorimeterHitData"); }

  bool isSubsetCollection() const final {
    return m_isSubsetColl;
  }

  void setSubsetCollection(bool setSubset=true) final;

  /// Returns the const object of given index
  CalorimeterHit operator[](unsigned int index) const;
  /// Returns the object of a given index
  MutableCalorimeterHit operator[](unsigned int index);
  /// Returns the const object of given index
  CalorimeterHit at(unsigned int index) const;
  /// Returns the object of given index
  MutableCalorimeterHit at(unsigned int index);


  /// Append object to the collection
  void push_back(CalorimeterHit object);

  void prepareForWrite() const final;
  void prepareAfterRead() final;
  bool setReferences(const podio::ICollectionProvider* collectionProvider) final;

  /// Get the collection buffers for this collection
  podio::CollectionBuffers getBuffers() final;

  void setID(unsigned ID) final {
    m_collectionID = ID;
    if (!m_isSubsetColl) {
      std::for_each(m_storage.entries.begin(), m_storage.entries.end(),
                  [ID] (CalorimeterHitObj* obj) { obj->id = {obj->id.index, static_cast<int>(ID)}; }
      );
    }
    m_isValid = true;
  };

  unsigned getID() const final {
    return m_collectionID;
  }

  bool isValid() const final {
    return m_isValid;
  }

  // support for the iterator protocol
  iterator begin() {
    return iterator(0, &m_storage.entries);
  }
  const_iterator begin() const {
    return const_iterator(0, &m_storage.entries);
  }
  iterator end() {
    return iterator(m_storage.entries.size(), &m_storage.entries);
  }
  const_iterator end() const {
    return const_iterator(m_storage.entries.size(), &m_storage.entries);
  }

  template<size_t arraysize>
  const std::array<std::uint64_t, arraysize> cellID() const;
  template<size_t arraysize>
  const std::array<float, arraysize> energy() const;
  template<size_t arraysize>
  const std::array<float, arraysize> energyError() const;
  template<size_t arraysize>
  const std::array<float, arraysize> time() const;
  template<size_t arraysize>
  const std::array<float, arraysize> timeError() const;
  template<size_t arraysize>
  const std::array<edm4hep::Vector3f, arraysize> position() const;
  template<size_t arraysize>
  const std::array<edm4hep::Vector3f, arraysize> dimension() const;
  template<size_t arraysize>
  const std::array<std::int32_t, arraysize> sector() const;
  template<size_t arraysize>
  const std::array<std::int32_t, arraysize> layer() const;
  template<size_t arraysize>
  const std::array<edm4hep::Vector3f, arraysize> local() const;

private:
  // For setReferences, we need to give our own CollectionData access to our
  // private entries. Otherwise we would need to expose a public member function
  // that gives access to the Obj* which is definitely not what we want
  friend class CalorimeterHitCollectionData;

  bool m_isValid{false};
  mutable bool m_isPrepared{false};
  bool m_isSubsetColl{false};
  int m_collectionID{0};
  mutable std::unique_ptr<std::mutex> m_storageMtx{std::make_unique<std::mutex>()};
  mutable CalorimeterHitCollectionData m_storage{};
};

std::ostream& operator<<(std::ostream& o, const CalorimeterHitCollection& v);

template<typename... Args>
MutableCalorimeterHit CalorimeterHitCollection::create(Args&&... args) {
  if (m_isSubsetColl) {
    throw std::logic_error("Cannot create new elements on a subset collection");
  }
  const int size = m_storage.entries.size();
  auto obj = new CalorimeterHitObj({size, m_collectionID}, {std::forward<Args>(args)...});
  m_storage.entries.push_back(obj);

  return MutableCalorimeterHit(obj);
}

template<size_t arraysize>
const std::array<std::uint64_t, arraysize> CalorimeterHitCollection::cellID() const {
  std::array<std::uint64_t, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.cellID;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<float, arraysize> CalorimeterHitCollection::energy() const {
  std::array<float, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.energy;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<float, arraysize> CalorimeterHitCollection::energyError() const {
  std::array<float, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.energyError;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<float, arraysize> CalorimeterHitCollection::time() const {
  std::array<float, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.time;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<float, arraysize> CalorimeterHitCollection::timeError() const {
  std::array<float, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.timeError;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<edm4hep::Vector3f, arraysize> CalorimeterHitCollection::position() const {
  std::array<edm4hep::Vector3f, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.position;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<edm4hep::Vector3f, arraysize> CalorimeterHitCollection::dimension() const {
  std::array<edm4hep::Vector3f, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.dimension;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<std::int32_t, arraysize> CalorimeterHitCollection::sector() const {
  std::array<std::int32_t, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.sector;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<std::int32_t, arraysize> CalorimeterHitCollection::layer() const {
  std::array<std::int32_t, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.layer;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<edm4hep::Vector3f, arraysize> CalorimeterHitCollection::local() const {
  std::array<edm4hep::Vector3f, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.local;
  }
  return tmp;
}


#ifdef PODIO_JSON_OUTPUT
void to_json(nlohmann::json& j, const CalorimeterHitCollection& collection);
#endif

} // namespace edm4nwb


#endif
