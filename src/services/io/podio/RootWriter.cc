
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

    void ROOTWriter::writeEvent(eic::EventStore* store)
    {
        std::vector<StoreCollection> collections;
        collections.reserve(m_collectionsToWrite.size());
        for (const auto &name : m_collectionsToWrite)
        {
            // (see long comment below)
            if( unwritable_collections.count(name) != 0 ) continue;

            const podio::CollectionBase *coll;
            store->get(name, coll);
            collections.emplace_back(name, const_cast<podio::CollectionBase *>(coll));
            try {
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
                collections.back().second->prepareForWrite();
            }catch( std::exception &e) {
                // podio threw an exception (see comments above). In this case, just
                // don't write out the collection to the file. Warn user on first occurance
                // for each type.
                collections.pop_back();
                if( unwritable_collections.count(name) ==0 ){
                    m_log->error( fmt::format("Unable to write collection {} to output file. Skipping.", name) );
                    unwritable_collections.insert( name );
                }
            }
        }

        if (m_firstEvent)
        {
            createBranches(collections);
            m_firstEvent = false;
        }
        else
        {
            setBranches(collections);
        }

        m_datatree->Fill();
        m_evtMDtree->Fill();
    }

    void ROOTWriter::createBranches(const std::vector<StoreCollection> &collections)
    {
        int iCollection = 0;
        for (auto &[name, coll] : collections)
        {
            podio::root_utils::CollectionBranches branches;
            const auto collBuffers = coll->getBuffers();
            if (collBuffers.data)
            {
                // only create the data buffer branch if necessary

                auto collClassName = "vector<" + coll->getDataTypeName() + ">";

                branches.data = m_datatree->Branch(name.c_str(), collClassName.c_str(), collBuffers.data);
            }

            // reference collections
            if (auto refColls = collBuffers.references)
            {
                int i = 0;
                for (auto &c : (*refColls))
                {
                    const auto brName = podio::root_utils::refBranch(name, i);
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
                    const auto brName = podio::root_utils::vecBranch(name, i);
                    branches.vecs.push_back(m_datatree->Branch(brName.c_str(), typeName.c_str(), vec));
                    ++i;
                }
            }
            m_collectionBranches.push_back(branches);
        }
    }

    void ROOTWriter::setBranches(const std::vector<StoreCollection> &collections)
    {
        size_t iCollection = 0;
        for (auto &coll : collections)
        {
            const auto &branches = m_collectionBranches[iCollection];
            podio::root_utils::setCollectionAddresses(coll.second, branches);

            iCollection++;
        }
    }

    void ROOTWriter::finish()
    {
        // now we want to safe the metadata. This includes info about the
        // collections
        const auto collIDTable = m_store->getCollectionIDTable();
        m_metadatatree->Branch("CollectionIDs", collIDTable);

        // collectionID, collection type, subset collection
        std::vector<podio::root_utils::CollectionInfoT> collectionInfo;
        collectionInfo.reserve(m_collectionsToWrite.size());
        for (const auto &name : m_collectionsToWrite)
        {
            const auto collID = collIDTable->collectionID(name);
            const podio::CollectionBase *coll{nullptr};
            // No check necessary, only registered collections possible
            m_store->get(name, coll);
            const auto collType = coll->getTypeName();
            collectionInfo.emplace_back(collID, std::move(collType), coll->isSubsetCollection());
        }

        m_metadatatree->Branch("CollectionTypeInfo", &collectionInfo);

        podio::version::Version podioVersion = podio::version::build_version;
        m_metadatatree->Branch("PodioVersion", &podioVersion);

        m_metadatatree->Fill();

        m_colMDtree->Branch("colMD", "std::map<int,podio::GenericParameters>", m_store->getColMetaDataMap());
        m_colMDtree->Fill();
        m_runMDtree->Branch("runMD", "std::map<int,podio::GenericParameters>", m_store->getRunMetaDataMap());
        m_runMDtree->Fill();

        m_file->Write();
        m_file->Close();
    }

    bool ROOTWriter::registerForWrite(const std::string &name)
    {
        const podio::CollectionBase *tmp_coll(nullptr);
        if (!m_store->get(name, tmp_coll))
        {
            std::cerr << "RootWriter: Omitting bad collection name: " << name << std::endl;
            return false;
        }

        m_collectionsToWrite.push_back(name);
        return true;
    }

} // namespace podio
