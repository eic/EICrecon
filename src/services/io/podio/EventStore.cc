
// podio specific includes
#include "EventStore.h"
#include <podio/CollectionBase.h>
#include <podio/IReader.h>

namespace eic {

void EventStore::put(const std::string name, podio::CollectionBase* collection) {
    for (const auto& collection_info : m_collections) {
        if (collection_info.id == collection->getID()) {
            std::ostringstream oss;
            oss << "EventStore::put: Collision on id '" << collection_info.id << "'";
            throw std::runtime_error(oss.str());
        }
        if (collection_info.name == name) {
            std::ostringstream oss;
            oss << "EventStore::put: Collision on name '" << name << "'";
        }
    }
    CollectionInfo info;
    info.id = collection->getID();
    info.name = name;
    info.collection = collection;
    m_collections.push_back(info);
}

podio::CollectionBase* EventStore::get_untyped(const std::string& name) {
    for (const auto& c : m_collections) {
        if (c.name == name) {
            return c.collection;
        }
    }
    return nullptr;
}

bool EventStore::get(int id, podio::CollectionBase*& collection) const {
    for (const auto &c: m_collections) {
        if (c.id == id) {
            collection = c.collection;
            return true;
        }
    }
    return false;
}

podio::GenericParameters& EventStore::getEventMetaData() {
  return m_evtMD;
}

podio::GenericParameters& EventStore::getRunMetaData(int runID) {
  return m_runMDMap[runID];
}

podio::GenericParameters& EventStore::getCollectionMetaData(int colID) {
    return m_colMDMap[colID];
}


void EventStore::clear() {
    m_collections.clear();
    m_evtMD.clear();
}

} // namespace podio