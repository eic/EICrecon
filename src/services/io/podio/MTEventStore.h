#ifndef EIC_PODIO_EVENTSTORE_H
#define EIC_PODIO_EVENTSTORE_H

#include <iostream>
#include <memory>
#include <set>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// podio specific includes
#include <podio/GenericParameters.h>
#include <podio/ICollectionProvider.h>
#include <podio/IMetaDataProvider.h>
#include <sstream>

#include <spdlog/spdlog.h>

/**
The event store holds the object collections.
**/

namespace podio {
class CollectionBase;
}

namespace eic {


class MTEventStore : public podio::ICollectionProvider, public podio::IMetaDataProvider {
public:
    struct CollectionInfo {
        int id; // This might be redundant with what is on the collection itself, but I don't trust PODIO to do the right thing here
        std::string name;
        podio::CollectionBase *collection;
    };

    MTEventStore(std::shared_ptr<spdlog::logger>& logger);
    ~MTEventStore();
    MTEventStore(const MTEventStore &) = delete;
    MTEventStore &operator=(const MTEventStore &) = delete;

    podio::CollectionBase* get_untyped(const std::string& name);
    const std::vector<CollectionInfo>& get_all() { return m_collections; }

    // Needed for ICollectionProvider so that it can call recursively so that we can set references
    bool get(int id, podio::CollectionBase*& collection) const override;

    /// access a collection by name
    template<typename T>
    T* get(const std::string& name);

    void put(const std::string name, podio::CollectionBase* collection);
    void clear();

private:
    int m_next_collection_id = 1;
    std::vector<CollectionInfo> m_collections;

    std::shared_ptr<spdlog::logger> m_log;

    typedef std::map<int, podio::GenericParameters> RunMDMap;
    typedef std::map<int, podio::GenericParameters> ColMDMap;
    RunMDMap m_runMDMap{};
    ColMDMap m_colMDMap{};
    podio::GenericParameters m_evtMD{};

public:

    podio::GenericParameters& getEventMetaData() override;
    podio::GenericParameters& getRunMetaData(int runID) override;
    podio::GenericParameters& getCollectionMetaData(int colID) override;

    RunMDMap *getRunMetaDataMap() { return &m_runMDMap; }
    ColMDMap *getColMetaDataMap() { return &m_colMDMap; }

    void setEventMetaData(podio::GenericParameters emd) { m_evtMD = emd;}
    void setRunMetaDataMap(RunMDMap&& map) { m_runMDMap = map; }
    void setColMetaDataMap(ColMDMap&& map) { m_colMDMap = map; }
};

template<typename T>
T* MTEventStore::get(const std::string& name) {
    auto result = get_untyped(name);
    if (result == nullptr) {
        std::ostringstream oss;
        oss << "Collection '" << name << "' not present in event store!";
        throw std::runtime_error(oss.str());
    }
    return static_cast<T*>(result);
}

} // namespace podio
#endif
