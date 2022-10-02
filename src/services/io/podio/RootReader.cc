// Copied from https://github.com/AIDASoft/podio/blob/master/src/ROOTReader.cc

#include "ROOTReader.h"

#include "rootUtils.h"

// podio specific includes
#include "podio/CollectionBase.h"
#include "podio/CollectionIDTable.h"
#include "podio/GenericParameters.h"

// ROOT specific includes
#include "TChain.h"
#include "TClass.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeCache.h"
#include <memory>
#include <podio/CollectionBase.h>
#include <podio/CollectionIDTable.h>
#include <services/io/podio/EventStore.h>

namespace eic {

ROOTReader::ROOTReader() {
    m_log = spdlog::default_logger();
    m_log->set_level(spdlog::level::trace);
}

// todo: see https://github.com/AIDASoft/podio/issues/290
ROOTReader::~ROOTReader() { // NOLINT(modernize-use-equals-default)
}

/// Multithreaded API
void ROOTReader::readEvent(eic::EventStore* store, uint64_t event_nr) {
    m_log->debug("ROOTReader: Reading event {}", event_nr);
    m_eventNumber = event_nr;
    m_chain->GetEntry(m_eventNumber);

    for (auto collection_name : m_table->names()) {
        m_log->debug("ROOTReader: Reading collection {}", collection_name);
        auto collection = readCollection(collection_name);
        store->put(collection_name, collection);
    }
    for (auto& c : store->get_all()) {
        c.collection->prepareAfterRead();
        c.collection->setReferences(store);
    }
}

std::pair<TTree*, unsigned> ROOTReader::getLocalTreeAndEntry(const std::string& treename) {
    auto localEntry = m_chain->LoadTree(m_eventNumber);
    auto* tree = static_cast<TTree*>(m_chain->GetFile()->Get(treename.c_str()));
    return {tree, localEntry};
}

podio::GenericParameters* ROOTReader::readEventMetaData() {
    auto* emd = new podio::GenericParameters();
    auto [evt_metadatatree, entry] = getLocalTreeAndEntry("evt_metadata");
    auto* branch = podio::root_utils::getBranch(evt_metadatatree, "evtMD");
    branch->SetAddress(&emd);
    evt_metadatatree->GetEntry(entry);
    return emd;
}
std::map<int, podio::GenericParameters>* ROOTReader::readCollectionMetaData() {
    auto* emd = new std::map<int, podio::GenericParameters>;
    auto* col_metadatatree = getLocalTreeAndEntry("col_metadata").first;
    auto* branch = podio::root_utils::getBranch(col_metadatatree, "colMD");
    branch->SetAddress(&emd);
    col_metadatatree->GetEntry(0);
    return emd;
}
std::map<int, podio::GenericParameters>* ROOTReader::readRunMetaData() {
    auto* emd = new std::map<int, podio::GenericParameters>;
    auto* run_metadatatree = getLocalTreeAndEntry("run_metadata").first;
    auto* branch = podio::root_utils::getBranch(run_metadatatree, "runMD");
    branch->SetAddress(&emd);
    run_metadatatree->GetEntry(0);
    return emd;
}

podio::CollectionBase* ROOTReader::readCollection(const std::string& name) {
    // Do we know about this collection? If so, read it
    if (const auto& info = m_storedClasses.find(name); info != m_storedClasses.end()) {
        return getCollection(*info);
    }

    // At this point this collection is definitely not in this file, because we
    // have no information on how to construct it in the first place
    return nullptr;
}

podio::CollectionBase* ROOTReader::getCollection(const std::pair<std::string, CollectionInfo>& collInfo) {
    const auto& name = collInfo.first;
    const auto& [theClass, collectionClass, index] = collInfo.second;
    auto& branches = m_collectionBranches[index];

    auto* collection = static_cast<podio::CollectionBase*>(collectionClass->New());
    auto collBuffers = collection->getBuffers();
    // If we have a valid data buffer class we know that have to read data,
    // otherwise we are handling a subset collection
    if (theClass) {
        collBuffers.data = theClass->New();
    } else {
        collection->setSubsetCollection();
    }

    const auto localEntry = m_chain->LoadTree(m_eventNumber);
    // After switching trees in the chain, branch pointers get invalidated so
    // they need to be reassigned.
    // NOTE: root 6.22/06 requires that we get completely new branches here,
    // with 6.20/04 we could just re-set them
    if (localEntry == 0) {
        branches.data = podio::root_utils::getBranch(m_chain, name.c_str());

        // reference collections
        if (auto* refCollections = collBuffers.references) {
            for (size_t i = 0; i < refCollections->size(); ++i) {
                const auto brName = podio::root_utils::refBranch(name, i);
                branches.refs[i] = podio::root_utils::getBranch(m_chain, brName.c_str());
            }
        }

        // vector members
        if (auto* vecMembers = collBuffers.vectorMembers) {
            for (size_t i = 0; i < vecMembers->size(); ++i) {
                const auto brName = podio::root_utils::vecBranch(name, i);
                branches.vecs[i] = podio::root_utils::getBranch(m_chain, brName.c_str());
            }
        }
    }

    // set the addresses
    podio::root_utils::setCollectionAddresses(collection, branches);

    return readCollectionData(branches, collection, localEntry, name);
}

podio::CollectionBase* ROOTReader::readCollectionData(const podio::root_utils::CollectionBranches& branches,
                                               podio::CollectionBase* collection, Long64_t entry, const std::string& name) {
    // Read all data
    if (branches.data) {
        branches.data->GetEntry(entry);
    }
    for (auto* br : branches.refs) {
        br->GetEntry(entry);
    }
    for (auto* br : branches.vecs) {
        br->GetEntry(entry);
    }

    // do the unpacking
    const auto id = m_table->collectionID(name);
    collection->setID(id);
    collection->prepareAfterRead();
    return collection;
}

void ROOTReader::openFile(const std::string& filename) {
    openFiles({filename});
}

void ROOTReader::openFiles(const std::vector<std::string>& filenames) {
    m_chain = new TChain("events");
    for (const auto& filename : filenames) {
        m_chain->Add(filename.c_str());
    }

    // read the meta data and build the collectionBranches cache
    // NOTE: This is a small pessimization, if we do not read all collections
    // afterwards, but it makes the handling much easier in general
    auto metadatatree = static_cast<TTree*>(m_chain->GetFile()->Get("metadata"));
    m_table = new podio::CollectionIDTable();
    metadatatree->SetBranchAddress("CollectionIDs", &m_table);

    podio::version::Version* versionPtr{nullptr};
    if (auto* versionBranch = podio::root_utils::getBranch(metadatatree, "PodioVersion")) {
        versionBranch->SetAddress(&versionPtr);
    }

    // Check if the CollectionTypeInfo branch is there and assume that the file
    // has been written with with podio pre #197 (<0.13.1) if that is not the case
    if (auto* collInfoBranch = podio::root_utils::getBranch(metadatatree, "CollectionTypeInfo")) {
        auto collectionInfo = new std::vector<podio::root_utils::CollectionInfoT>;
        collInfoBranch->SetAddress(&collectionInfo);
        metadatatree->GetEntry(0);
        createCollectionBranches(*collectionInfo);
        delete collectionInfo;
    } else {
        std::cout << "PODIO: Reconstructing CollectionTypeInfo branch from other sources in file: \'"
                  << m_chain->GetFile()->GetName() << "\'" << std::endl;
        metadatatree->GetEntry(0);
        const auto collectionInfo = podio::root_utils::reconstructCollectionInfo(m_chain, *m_table);
        createCollectionBranches(collectionInfo);
    }

    m_fileVersion = versionPtr ? *versionPtr : podio::version::Version{0, 0, 0};
    delete versionPtr;
}

void ROOTReader::closeFile() {
    closeFiles();
}

void ROOTReader::closeFiles() {
    delete m_chain;
}

bool ROOTReader::isValid() const {
    return m_chain->GetFile()->IsOpen() && !m_chain->GetFile()->IsZombie();
}

unsigned ROOTReader::getEntries() const {
    return m_chain->GetEntries();
}

void ROOTReader::createCollectionBranches(const std::vector<podio::root_utils::CollectionInfoT>& collInfo) {
    size_t collectionIndex{0};

    for (const auto& [collID, collType, isSubsetColl] : collInfo) {
        // We only write collections that are in the collectionIDTable, so no need
        // to check here
        const auto name = m_table->name(collID);

        podio::root_utils::CollectionBranches branches{};
        const auto collectionClass = TClass::GetClass(collType.c_str());

        // Make sure that ROOT actually knows about this datatype before running
        // into a potentially cryptic segmentation fault by accessing the nullptr
        if (!collectionClass) {
            std::cerr << "PODIO: Cannot create the collection type \'" << collType << "\' stored in branch \'" << name
                      << "\'. Contents of this branch cannot be read." << std::endl;
            continue;
        }
        // Need the collection here to setup all the branches. Have to manage the
        // temporary collection ourselves
        auto collection =
                std::unique_ptr<podio::CollectionBase>(static_cast<podio::CollectionBase*>(collectionClass->New()));
        collection->setSubsetCollection(isSubsetColl);

        if (!isSubsetColl) {
            // This branch is guaranteed to exist since only collections that are
            // also written to file are in the info metadata that we work with here
            branches.data = podio::root_utils::getBranch(m_chain, name.c_str());
        }

        const auto buffers = collection->getBuffers();
        for (size_t i = 0; i < buffers.references->size(); ++i) {
            const auto brName = podio::root_utils::refBranch(name, i);
            branches.refs.push_back(podio::root_utils::getBranch(m_chain, brName.c_str()));
        }

        for (size_t i = 0; i < buffers.vectorMembers->size(); ++i) {
            const auto brName = podio::root_utils::vecBranch(name, i);
            branches.vecs.push_back(podio::root_utils::getBranch(m_chain, brName.c_str()));
        }

        const std::string bufferClassName = "std::vector<" + collection->getDataTypeName() + ">";
        const auto bufferClass = isSubsetColl ? nullptr : TClass::GetClass(bufferClassName.c_str());

        m_storedClasses.emplace(name, std::make_tuple(bufferClass, collectionClass, collectionIndex++));
        m_collectionBranches.push_back(branches);
    }
}

} // namespace eic
