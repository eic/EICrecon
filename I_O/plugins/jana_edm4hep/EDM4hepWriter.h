// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#ifndef _EDM4hepWriter_h_
#define _EDM4hepWriter_h_

// podio specific includes
#include <podio/EventStore.h>
#include <podio/podioVersion.h>


#include <TFile.h>
#include <TTree.h>

#include <JANA/JEventProcessor.h>

class EDM4hepWriter : public JEventProcessor {

    // Shared state (e.g. histograms, TTrees, TFiles) live
    std::mutex m_mutex;
    
public:

    EDM4hepWriter();
    virtual ~EDM4hepWriter() = default;

    void Init() override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void Finish() override;

protected:

    std::string m_OUTPUT_FILE = "podio_output.root";
    std::string m_collection_names_str;
    std::set<std::string> m_OUTPUT_COLLECTIONS;
    std::set<std::string> m_registered_collections;
    podio::EventStore store;

//===================================================================================================================
// Code in this section copied from the example ROOTWriter class in podio.
//
// TODO: Copy the changes back into the podio::ROOTWriter class and submit a PR so these are fixed upstream.
public:
    struct CollectionBranches {
        TBranch* data{nullptr};
        std::vector<TBranch*> refs{};
        std::vector<TBranch*> vecs{};
    };

    class ROOTWriter {

    public:
        ROOTWriter(const std::string& filename, podio::EventStore* store);
        ~ROOTWriter();

        // non-copyable
        ROOTWriter(const ROOTWriter&) = delete;
        ROOTWriter& operator=(const ROOTWriter&) = delete;

        bool registerForWrite(const std::string& name);
        void writeEvent();
        void finish();

    private:

        using StoreCollection = std::pair<const std::string&, podio::CollectionBase*>;
        void createBranches(const std::vector<StoreCollection>& collections);
        void setBranches(const std::vector<StoreCollection>& collections);

        // members
        std::string m_filename;
        podio::EventStore* m_store;
        TFile* m_file;
        TTree* m_datatree;
        TTree* m_metadatatree;
        TTree* m_runMDtree;
        TTree* m_evtMDtree;
        TTree* m_colMDtree;
        std::vector<std::string> m_collectionsToWrite{};
        // In order to avoid having to look up the branches from the datatree for
        // every event, we cache them in this vector, that is populated the first
        // time we write an event. Since the collections and their order do not
        // change between events, the assocation between the collections to write
        // and their branches is simply index based
        std::vector<EDM4hepWriter::CollectionBranches> m_collectionBranches{};

        bool m_firstEvent{true};
    };

//===================================================================================================================

    std::unique_ptr<EDM4hepWriter::ROOTWriter> writer;
};


#endif // _EDM4hepWriter_h_

