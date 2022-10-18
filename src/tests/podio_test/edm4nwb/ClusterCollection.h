// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#ifndef EDM4NWB_ClusterCollection_H
#define EDM4NWB_ClusterCollection_H

// datamodel specific includes
#include "edm4nwb/ClusterData.h"
#include "edm4nwb/Cluster.h"
#include "edm4nwb/MutableCluster.h"
#include "edm4nwb/ClusterObj.h"
#include "edm4nwb/ClusterCollectionData.h"

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



class ClusterCollectionIterator {
public:
  ClusterCollectionIterator(size_t index, const ClusterObjPointerContainer* collection) : m_index(index), m_object(nullptr), m_collection(collection) {}

  ClusterCollectionIterator(const ClusterCollectionIterator&) = delete;
  ClusterCollectionIterator& operator=(const ClusterCollectionIterator&) = delete;

  bool operator!=(const ClusterCollectionIterator& x) const {
    return m_index != x.m_index; // TODO: may not be complete
  }

  Cluster operator*();
  Cluster* operator->();
  ClusterCollectionIterator& operator++();

private:
  size_t m_index;
  Cluster m_object;
  const ClusterObjPointerContainer* m_collection;
};


class ClusterMutableCollectionIterator {
public:
  ClusterMutableCollectionIterator(size_t index, const ClusterObjPointerContainer* collection) : m_index(index), m_object(nullptr), m_collection(collection) {}

  ClusterMutableCollectionIterator(const ClusterMutableCollectionIterator&) = delete;
  ClusterMutableCollectionIterator& operator=(const ClusterMutableCollectionIterator&) = delete;

  bool operator!=(const ClusterMutableCollectionIterator& x) const {
    return m_index != x.m_index; // TODO: may not be complete
  }

  MutableCluster operator*();
  MutableCluster* operator->();
  ClusterMutableCollectionIterator& operator++();

private:
  size_t m_index;
  MutableCluster m_object;
  const ClusterObjPointerContainer* m_collection;
};


/**
A Collection is identified by an ID.
*/
class ClusterCollection : public podio::CollectionBase {
public:
  using const_iterator = ClusterCollectionIterator;
  using iterator = ClusterMutableCollectionIterator;

  ClusterCollection();
  // This is a move-only type
  ClusterCollection(const ClusterCollection& ) = delete;
  ClusterCollection& operator=(const ClusterCollection& ) = delete;
  ClusterCollection(ClusterCollection&&) = default;
  ClusterCollection& operator=(ClusterCollection&&) = default;

//  ClusterCollection(ClusterVector* data, int collectionID);
  ~ClusterCollection();

  void clear() final;

  /// operator to allow pointer like calling of members a la LCIO
  ClusterCollection* operator->() { return (ClusterCollection*) this; }

  /// Append a new object to the collection, and return this object.
  MutableCluster create();

  /// Append a new object to the collection, and return this object.
  /// Initialized with the parameters given
  template<typename... Args>
  MutableCluster create(Args&&... args);

  /// number of elements in the collection
  size_t size() const final;

  /// fully qualified type name
  std::string getTypeName() const final { return std::string("edm4nwb::ClusterCollection"); }
  /// fully qualified type name of elements - with namespace
  std::string getValueTypeName() const final { return std::string("edm4nwb::Cluster"); }
  /// fully qualified type name of stored POD elements - with namespace
  std::string getDataTypeName() const final { return std::string("edm4nwb::ClusterData"); }

  bool isSubsetCollection() const final {
    return m_isSubsetColl;
  }

  void setSubsetCollection(bool setSubset=true) final;

  /// Returns the const object of given index
  Cluster operator[](unsigned int index) const;
  /// Returns the object of a given index
  MutableCluster operator[](unsigned int index);
  /// Returns the const object of given index
  Cluster at(unsigned int index) const;
  /// Returns the object of given index
  MutableCluster at(unsigned int index);


  /// Append object to the collection
  void push_back(Cluster object);

  void prepareForWrite() const final;
  void prepareAfterRead() final;
  bool setReferences(const podio::ICollectionProvider* collectionProvider) final;

  /// Get the collection buffers for this collection
  podio::CollectionBuffers getBuffers() final;

  void setID(unsigned ID) final {
    m_collectionID = ID;
    if (!m_isSubsetColl) {
      std::for_each(m_storage.entries.begin(), m_storage.entries.end(),
                  [ID] (ClusterObj* obj) { obj->id = {obj->id.index, static_cast<int>(ID)}; }
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
  const std::array<std::int32_t, arraysize> type() const;
  template<size_t arraysize>
  const std::array<float, arraysize> energy() const;
  template<size_t arraysize>
  const std::array<float, arraysize> energyError() const;
  template<size_t arraysize>
  const std::array<float, arraysize> time() const;
  template<size_t arraysize>
  const std::array<float, arraysize> timeError() const;
  template<size_t arraysize>
  const std::array<std::uint32_t, arraysize> nhits() const;
  template<size_t arraysize>
  const std::array<edm4hep::Vector3f, arraysize> position() const;
  template<size_t arraysize>
  const std::array<edm4eic::Cov3f, arraysize> positionError() const;
  template<size_t arraysize>
  const std::array<float, arraysize> intrinsicTheta() const;
  template<size_t arraysize>
  const std::array<float, arraysize> intrinsicPhi() const;
  template<size_t arraysize>
  const std::array<edm4eic::Cov2f, arraysize> intrinsicDirectionError() const;

private:
  // For setReferences, we need to give our own CollectionData access to our
  // private entries. Otherwise we would need to expose a public member function
  // that gives access to the Obj* which is definitely not what we want
  friend class ClusterCollectionData;

  bool m_isValid{false};
  mutable bool m_isPrepared{false};
  bool m_isSubsetColl{false};
  int m_collectionID{0};
  mutable std::unique_ptr<std::mutex> m_storageMtx{std::make_unique<std::mutex>()};
  mutable ClusterCollectionData m_storage{};
};

std::ostream& operator<<(std::ostream& o, const ClusterCollection& v);

template<typename... Args>
MutableCluster ClusterCollection::create(Args&&... args) {
  if (m_isSubsetColl) {
    throw std::logic_error("Cannot create new elements on a subset collection");
  }
  const int size = m_storage.entries.size();
  auto obj = new ClusterObj({size, m_collectionID}, {std::forward<Args>(args)...});
  m_storage.entries.push_back(obj);

  // Need to initialize the relation vectors manually for the {ObjectID, ClusterData} constructor
  obj->m_clusters = new std::vector<edm4nwb::Cluster>();
  obj->m_hits = new std::vector<edm4nwb::CalorimeterHit>();
  obj->m_particleIDs = new std::vector<edm4hep::ParticleID>();
  obj->m_shapeParameters = new std::vector<float>();
  obj->m_hitContributions = new std::vector<float>();
  obj->m_subdetectorEnergies = new std::vector<float>();
  m_storage.createRelations(obj);
  return MutableCluster(obj);
}

template<size_t arraysize>
const std::array<std::int32_t, arraysize> ClusterCollection::type() const {
  std::array<std::int32_t, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.type;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<float, arraysize> ClusterCollection::energy() const {
  std::array<float, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.energy;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<float, arraysize> ClusterCollection::energyError() const {
  std::array<float, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.energyError;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<float, arraysize> ClusterCollection::time() const {
  std::array<float, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.time;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<float, arraysize> ClusterCollection::timeError() const {
  std::array<float, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.timeError;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<std::uint32_t, arraysize> ClusterCollection::nhits() const {
  std::array<std::uint32_t, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.nhits;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<edm4hep::Vector3f, arraysize> ClusterCollection::position() const {
  std::array<edm4hep::Vector3f, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.position;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<edm4eic::Cov3f, arraysize> ClusterCollection::positionError() const {
  std::array<edm4eic::Cov3f, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.positionError;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<float, arraysize> ClusterCollection::intrinsicTheta() const {
  std::array<float, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.intrinsicTheta;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<float, arraysize> ClusterCollection::intrinsicPhi() const {
  std::array<float, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.intrinsicPhi;
  }
  return tmp;
}

template<size_t arraysize>
const std::array<edm4eic::Cov2f, arraysize> ClusterCollection::intrinsicDirectionError() const {
  std::array<edm4eic::Cov2f, arraysize> tmp{};
  const auto valid_size = std::min(arraysize, m_storage.entries.size());
  for (unsigned i = 0; i < valid_size; ++i) {
    tmp[i] = m_storage.entries[i]->data.intrinsicDirectionError;
  }
  return tmp;
}


#ifdef PODIO_JSON_OUTPUT
void to_json(nlohmann::json& j, const ClusterCollection& collection);
#endif

} // namespace edm4nwb


#endif
