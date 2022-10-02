
// podio specific includes
#include "EventStore.h"
#include <podio/CollectionBase.h>
#include <podio/IReader.h>

namespace eic {

EventStore::EventStore() {
    m_log = spdlog::default_logger();
    m_log->set_level(spdlog::level::trace);
}

EventStore::~EventStore() {
    clear();
}

void EventStore::put(const std::string name, podio::CollectionBase* collection) {

    // Check if we've already seen this
    for (auto& collection_info : m_collections) {
        if (collection_info.name == name) {
            // We have a match!
            if (collection_info.collection == nullptr) {
                collection_info.collection = collection;
                collection->setID(collection_info.id);
                // If the user creates a collection and doesn't set the collection id, we still
                // want it to go to the correct place.
                // TODO: Overriding the specified collection id like this is ugly.
                //       Probably a better option is to take control of collection creation completely.
                return;
            }
            else {
                std::ostringstream oss;
                oss << "EventStore::put: Already have a collection with name '" << collection_info.name << "'";
                throw std::runtime_error(oss.str());
            }
        }
    }

    // If control flow reaches this point, this is the first time this EventStore has seen a collection with this name.
    // Now we need to assign it a non-conflicting id. We try to preserve the existing id whenever possible in case
    // we ever need to read and write different collections to different files while keeping the ids consistent.
    // Honestly we'd be better off manually assigning the ids manually instead.

    auto id = collection->getID();
    if (id == 0) {
        collection->setID(m_next_collection_id);
        m_next_collection_id++;
    }
    else if (id >= m_next_collection_id) {
        m_next_collection_id = id + 1;
    }
    else if (id < 0) {
        std::ostringstream oss;
        oss << "EventStore::put: Negative collection id '" << id << "'";
        throw std::runtime_error(oss.str());
    }
    else {
        // id is somewhere in (0, m_next_collection_id)
        // Check for id collisions
        for (const auto& collection_info : m_collections) {
            if (collection_info.id == collection->getID()) {
                std::ostringstream oss;
                oss << "EventStore::put: Collision on id '" << collection_info.id << "'";
                throw std::runtime_error(oss.str());
            }
        }
    }
    CollectionInfo info;
    info.id = collection->getID();
    info.name = name;
    info.collection = collection;
    m_collections.push_back(info);
    m_log->debug("eic::EventStore: Added collection '{}' with id {}", name, info.id);
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
    for (auto& c : m_collections) {
        delete c.collection;
        c.collection = nullptr;
    }
    m_evtMD.clear();
}

} // namespace podio