
#include "RootWriter.h"
#include "rootUtils.h"

// podio specific includes
#include "podio/CollectionBase.h"
#include "podio/EventStore.h"
#include "podio/ROOTWriter.h"
#include "podio/podioVersion.h"

#include <services/io/podio/EventStore.h>

// ROOT specifc includes
#include "TFile.h"
#include "TTree.h"

namespace eic {

    ROOTWriter::ROOTWriter(const std::string &filename, std::shared_ptr<spdlog::logger> &logger) :
        m_filename(filename),
        m_file(new TFile(filename.c_str(), "RECREATE", "data file")),
        m_datatree(new TTree("events", "Events tree")),
        m_metadatatree(new TTree("metadata", "Metadata tree")),
        m_runMDtree(new TTree("run_metadata", "Run metadata tree")),
        m_evtMDtree(new TTree("evt_metadata", "Event metadata tree")),
        m_colMDtree(new TTree("col_metadata", "Collection metadata tree")),
        m_log( logger )
    {
        // m_evtMDtree->Branch("evtMD", "GenericParameters", m_store->eventMetaDataPtr());
        // NWB: TODO: Where should this go? What is this really keyed off of? I assume writeEvent?
    }

    ROOTWriter::~ROOTWriter()
    {
        delete m_file;
    }

    // Important: David's explanation about why prepareForWrite() sometimes excepts:
    // For objects that have one-to-many relations, podio will loop
    // over these and check that the objectID of each of the related
    // objects is being "tracked". If one is not, then an exception
    // is thrown. Presumably, this is to ensure the related objects
    // are also being written to the podio output file since podio
    // just writes indexes for each of these and needs them to be there
    // when it tries to read them back in. Currently, since we are not
    // fully in on the podio collection management, these objects
    // are often "untracked". For example, the EcalEndcapNIslandProtoClusters
    // have edm4eic::CalorimeterHit objects that are currently "untracked".
    // Thus, when prepareForWrite is called for the EcalEndcapNIslandProtoClusters
    // collection, an exception is thrown.
    // (See for example ProtoClusterCollectionData.cc)

    void ROOTWriter::writeEvent(eic::EventStore* store) {

        if (m_firstEvent) {
            // Populate collection infos
            for (const auto& c : store->get_all()) {
                auto collection_info = std::make_unique<CollectionInfo>();
                collection_info->name = c.name;
                collection_info->id = c.id;
                if (m_write_requests.count(c.name) != 0) collection_info->write_requested = true;
                collection_info->write_failed = false;
                m_collection_infos[c.name] = std::move(collection_info);
            }
            // TODO: Check whether everything in m_write_requests matched up to an entry in the collection ID table
        }

        for (auto& pair : m_collection_infos) {
            if (pair.second->write_requested && !pair.second->write_failed) {

                podio::CollectionBase* collection = store->get_untyped(pair.first);
                if (collection == nullptr) {
                    m_log->error("RootWriter: Unable to find collection '{}' in event store. Skipping.", pair.first);
                    pair.second->write_failed = true;
                    continue;
                }
                try {
                    collection->prepareForWrite();
                }
                catch (std::exception &e) {
                    m_log->error("Unable to write collection {} to output file. Skipping.", pair.first);
                    pair.second->write_failed = true;
                    continue;
                }
                m_log->trace("RootWriter: Writing collection '{}' with id={}, size={}", pair.first, pair.second->id, collection->size());
                if (m_firstEvent) {
                    m_log->trace("RootWriter: Creating branches for collection '{}' ({})", pair.first, pair.second->id);
                    pair.second->podtype = collection->getTypeName();
                    createBranches(*(pair.second), collection);
                }
                else {
                    m_log->trace("RootWriter: Moving branch pointer for collection '{}' ({})", pair.first, pair.second->id);
                    podio::root_utils::setCollectionAddresses(collection, pair.second->branches);
                }
            }
        }
        m_firstEvent = false;
        m_datatree->Fill();
        m_evtMDtree->Fill();
        m_log->debug("Finished writing event");
    }

    void ROOTWriter::createBranches(CollectionInfo& info, podio::CollectionBase* collection) {

        auto& branches = info.branches;

        const auto collBuffers = collection->getBuffers();
        if (collBuffers.data)
        {
            // only create the data buffer branch if necessary

            auto collClassName = "vector<" + collection->getDataTypeName() + ">";

            branches.data = m_datatree->Branch(info.name.c_str(), collClassName.c_str(), collBuffers.data);
        }

        // reference collections
        if (auto refColls = collBuffers.references)
        {
            int i = 0;
            for (auto &c : (*refColls))
            {
                const auto brName = podio::root_utils::refBranch(info.name, i);
                branches.refs.push_back(m_datatree->Branch(brName.c_str(), c.get()));
                ++i;
            }
        }

        // vector members
        if (auto vminfo = collBuffers.vectorMembers)
        {
            int i = 0;
            for (auto &[type, vec] : (*vminfo))
            {
                const auto typeName = "vector<" + type + ">";
                const auto brName = podio::root_utils::vecBranch(info.name, i);
                branches.vecs.push_back(m_datatree->Branch(brName.c_str(), typeName.c_str(), vec));
                ++i;
            }
        }
    }


    void ROOTWriter::finish()
    {
        m_log->debug("Calling ROOTWriter::finish");
        // Extract ([names],[ids])
        std::vector<int> ids;
        std::vector<std::string> names;
        for (const auto& item: m_collection_infos) {
            ids.push_back(item.second->id);
            names.push_back(item.second->name);
            m_log->trace("Writing collection metadata '{}' with id {}", item.second->name, item.second->id);
        }
        auto collection_id_table = podio::CollectionIDTable(std::move(ids), std::move(names));
        m_metadatatree->Branch("CollectionIDs", &collection_id_table);

        // Extract (collection_id, collection_type, is_subset)
        std::vector<podio::root_utils::CollectionInfoT> collectionTypeInfo;
        collectionTypeInfo.reserve(m_collection_infos.size());
        for (const auto &pair : m_collection_infos) {
            auto& collection_info = pair.second;
            collectionTypeInfo.emplace_back(collection_info->id, collection_info->podtype, collection_info->is_subset);
        }

        m_metadatatree->Branch("CollectionTypeInfo", &collectionTypeInfo);

        podio::version::Version podioVersion = podio::version::build_version;
        m_metadatatree->Branch("PodioVersion", &podioVersion);

        m_metadatatree->Fill();

        // TODO: Re-enable these when I understand them
        // m_colMDtree->Branch("colMD", "std::map<int,podio::GenericParameters>", m_store->getColMetaDataMap());
        // m_colMDtree->Fill();
        // m_runMDtree->Branch("runMD", "std::map<int,podio::GenericParameters>", m_store->getRunMetaDataMap());
        // m_runMDtree->Fill();

        m_file->Write();
        m_file->Close();
        m_log->debug("RootWriter::finish: Wrote and closed file");
    }

    bool ROOTWriter::registerForWrite(const std::string &name) {
        m_write_requests.insert(name);
        return true;
    }

} // namespace podio
