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
    std::string m_OUTPUT_FILE_COPY_DIR = "";
    std::string m_include_collections_str;
    std::string m_exclude_collections_str;
    std::set<std::string> m_OUTPUT_INCLUDE_COLLECTIONS;
    std::set<std::string> m_OUTPUT_EXCLUDE_COLLECTIONS;
    static thread_local podio::EventStore m_store;

    std::unique_ptr<TFile> m_file;
    TTree* m_datatree;
    TTree* m_metadatatree;
    TTree* m_runMDtree;
    TTree* m_evtMDtree;
    TTree* m_colMDtree;

    std::set<std::string> m_collection_branches; // names of collections that have branches made
    std::vector<std::tuple<int, std::string, bool>> m_collectionInfo;

    void createBranch(const std::string& collName, podio::CollectionBase* collBase);
    //void createBranches(const std::map<std::string, podio::CollectionBase*>& collections);
    void resetBranches(const std::map<std::string, podio::CollectionBase*>& collections);

    // The following were defined in the podio src/rootUtils.h file. Oddly, this does not
    // seem to be in the podio install directory (??) Thus, they are copied here.
    inline std::string refBranch(const std::string& name, size_t index) {
        return name + "#" + std::to_string(index);
    }
    inline std::string vecBranch(const std::string& name, size_t index) {
        return name + "_" + std::to_string(index);
    }
};


#endif // _EDM4hepWriter_h_

