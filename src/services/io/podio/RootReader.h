// Copied from https://github.com/AIDASoft/podio/blob/master/include/podio/ROOTReader.h

#ifndef EICRECON_ROOT_READER_H
#define EICRECON_ROOT_READER_H

#include "podio/CollectionBranches.h"
#include "podio/ICollectionProvider.h"
#include "podio/IReader.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <memory>
#include <spdlog/spdlog.h>

// forward declarations
class TClass;
class TFile;
class TTree;
class TChain;

namespace podio {
class CollectionBase;
class Registry;
class CollectionIDTable;
class GenericParameters;
}

namespace eic {
class EventStore;

/**
This class has the function to read available data from disk
and to prepare collections and buffers.
**/
class ROOTReader {
    friend EventStore;

public:
    ROOTReader();
    // todo: see https://github.com/AIDASoft/podio/issues/290
    ~ROOTReader(); // NOLINT(modernize-use-equals-default)

    // non-copyable
    ROOTReader(const ROOTReader&) = delete;
    ROOTReader& operator=(const ROOTReader&) = delete;

    void openFile(const std::string& filename);
    void openFiles(const std::vector<std::string>& filenames);
    void closeFile();
    void closeFiles();

    /// Read all collections requested and _insert_ into provided event store
    void readEvent(eic::EventStore* store, uint64_t event_nr);

    /// Read CollectionIDTable from ROOT file
    podio::CollectionIDTable* getCollectionIDTable() {
        return m_table;
    }

    /// Returns number of entries in the TTree
    unsigned getEntries() const;

    podio::version::Version currentFileVersion() const {
        return m_fileVersion;
    }

    /// Check if TFile is valid
    bool isValid() const;

    inline void setLogger(std::shared_ptr<spdlog::logger> log) { m_log = log; }

private:
    /// Implementation for collection reading
    podio::CollectionBase* readCollection(const std::string& name);

    /// read event meta data for current event
    podio::GenericParameters* readEventMetaData();

    /// read the collection meta data
    std::map<int, podio::GenericParameters>* readCollectionMetaData();

    /// read the run meta data
    std::map<int, podio::GenericParameters>* readRunMetaData();

    /// logging
    std::shared_ptr<spdlog::logger> m_log;

private:
    void createCollectionBranches(const std::vector<std::tuple<int, std::string, bool>>& collInfo);

    std::pair<TTree*, unsigned> getLocalTreeAndEntry(const std::string& treename);
    // Information about the data vector as wall as the collection class type
    // and the index in the collection branches cache vector
    using CollectionInfo = std::tuple<const TClass*, const TClass*, size_t>;

    podio::CollectionBase* getCollection(const std::pair<std::string, CollectionInfo>& collInfo);
    podio::CollectionBase* readCollectionData(const podio::root_utils::CollectionBranches& branches, podio::CollectionBase* collection,
                                       Long64_t entry, const std::string& name);

    // cache the necessary information to more quickly construct and read each
    // collection after it has been read the very first time
    std::map<std::string, CollectionInfo> m_storedClasses{};

    podio::CollectionIDTable* m_table{nullptr};
    TChain* m_chain{nullptr};
    unsigned m_eventNumber{0};

    // Similar to writing we cache the branches that belong to each collection
    // in order to not having to look them up every event. However, for the
    // reader we cannot guarantee a fixed order of collections as they are read
    // on demand. Hence, we give each collection an index the first time it is
    // read and we start caching the branches.
    size_t m_collectionIndex = 0;
    std::vector<podio::root_utils::CollectionBranches> m_collectionBranches{};

    podio::version::Version m_fileVersion{0, 0, 0};
};

} // namespace podio

#endif //EICRECON_ROOT_READER_H
