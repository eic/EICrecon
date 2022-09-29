#ifndef EIC_PODIO_EVENTSTORE_H
#define EIC_PODIO_EVENTSTORE_H

#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

// podio specific includes
#include <podio/CollectionIDTable.h>
#include <podio/GenericParameters.h>
#include <podio/ICollectionProvider.h>
#include <podio/IMetaDataProvider.h>

/**
This is an *example* event store

The event store holds the object collections.

It is used to create new collections, and to access existing ones.
When accessing a collection that is not yet in the event store,
the event store makes use of a Reader to read the collection.

**/

namespace podio {
class CollectionBase;
class IReader;
}

namespace eic {

typedef std::map<int, podio::GenericParameters> RunMDMap;
typedef std::map<int, podio::GenericParameters> ColMDMap;

class EventStore : public podio::ICollectionProvider, public podio::IMetaDataProvider {
public:
  /// Make non-copyable
  EventStore(const EventStore&) = delete;
  EventStore& operator=(const EventStore&) = delete;

  /// Collection entry. Each collection is identified by a name
  typedef std::pair<std::string, podio::CollectionBase*> CollPair;
  typedef std::vector<CollPair> CollContainer;

  EventStore();
  ~EventStore();

  /// create a new collection
  template <typename T>
  T& create(const std::string& name);

  /// register an existing collection
  void registerCollection(const std::string& name, podio::CollectionBase* coll);

  /// access a collection by name. returns true if successful
  template <typename T>
  bool get(const std::string& name, T*& collection);

  /// fast access to cached collections
  podio::CollectionBase* getFast(int id) const {
    return (m_cachedCollections.size() > (unsigned)id ? m_cachedCollections[id] : nullptr);
  }

  /// access a collection by ID. returns true if successful
  bool get(int id, podio::CollectionBase*& coll) const final;

  /// access a collection by name
  /// returns a collection w/ setting isValid to true if successful
  template <typename T>
  T& get(const std::string& name);

  /// empties collections.
  void clearCollections();

  /// clears itself; deletes collections (use at end of event processing)
  void clear();

  /// Clears only the cache containers (use at end of event if ownership of read objects is transferred)
  void clearCaches();

  /// set the reader
  void setReader(podio::IReader* reader);

  podio::CollectionIDTable* getCollectionIDTable() const {
    return m_table.get();
  }

  virtual bool isValid() const final;

  /// return the event meta data for the current event
  podio::GenericParameters& getEventMetaData() override;

  /// return the run meta data for the given runID
  podio::GenericParameters& getRunMetaData(int runID) override;

  /// return the collection meta data for the given colID
  podio::GenericParameters& getCollectionMetaData(int colID) override;

  RunMDMap* getRunMetaDataMap() {
    return &m_runMDMap;
  }
  ColMDMap* getColMetaDataMap() {
    return &m_colMDMap;
  }
  podio::GenericParameters* eventMetaDataPtr() {
    return &m_evtMD;
  }

private:
  /// get the collection of given name; returns true if successfull
  bool doGet(const std::string& name, podio::CollectionBase*& collection, bool setReferences = true) const;
  /// check if a collection of given name already exists
  bool collectionRegistered(const std::string& name) const;
  void setCollectionIDTable(podio::CollectionIDTable* table) {
    m_table.reset(table);
  }

  // members
  mutable std::set<int> m_retrievedIDs{};
  mutable CollContainer m_collections{};
  mutable std::vector<podio::CollectionBase*> m_cachedCollections{};
  podio::IReader* m_reader{nullptr};
  std::unique_ptr<podio::CollectionIDTable> m_table;

  podio::GenericParameters m_evtMD{};
  RunMDMap m_runMDMap{};
  ColMDMap m_colMDMap{};
};

template <typename T>
T& EventStore::create(const std::string& name) {
  static_assert(std::is_base_of<podio::CollectionBase, T>::value,
                "DataStore only accepts types inheriting from CollectionBase");
  // TODO: add check for existence
  T* coll = new T();
  registerCollection(name, coll);
  return *coll;
}

template <typename T>
bool EventStore::get(const std::string& name, T*& collection) {
  //  static_assert(std::is_base_of<podio::CollectionBase,T>::value,
  //              "DataStore only contains types inheriting from CollectionBase");
  podio::CollectionBase* tmp{nullptr};
  doGet(name, tmp);
  collection = static_cast<T*>(tmp);
  if (collection != nullptr) {
    return true;
  }
  return false;
}

template <typename T>
T& EventStore::get(const std::string& name) {
  T* tmp(0);
  auto success = this->get(name, tmp);
  if (!success) {
    throw std::runtime_error("No collection \'" + name + "\' is present in the EventStore");
  }
  return *tmp;
}

} // namespace podio
#endif
