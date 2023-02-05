// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#ifndef _EDM4hepWriter_h_
#define _EDM4hepWriter_h_

// podio specific includes
#include <podio/podioVersion.h>
#include <podio/CollectionIDTable.h>
#include <podio/GenericParameters.h>


#include <TFile.h>
#include <TTree.h>

#include <JANA/JEventProcessor.h>

#include <services/io/podio/EICEventStore.h>

/// This is a JEventProcessor class that is used for writing out podio/edm4hep root files.
class EICRootWriter : public JEventProcessor {

    // Shared state (e.g. histograms, TTrees, TFiles) live
    std::mutex m_mutex;

public:

    EICRootWriter();
    virtual ~EICRootWriter() = default;

    void Init() override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void Finish() override;

    const std::set<std::string>& GetIncludeCollections(){ return m_OUTPUT_INCLUDE_COLLECTIONS; }
    const std::set<std::string>& GetExcludeCollections(){ return m_OUTPUT_EXCLUDE_COLLECTIONS; }

protected:

    std::string m_OUTPUT_FILE = "podio_output.root";
    std::string m_OUTPUT_FILE_COPY_DIR = "";
    std::string m_include_collections_str;
    std::string m_exclude_collections_str;
    std::set<std::string> m_OUTPUT_INCLUDE_COLLECTIONS;
    std::set<std::string> m_OUTPUT_EXCLUDE_COLLECTIONS;

    std::unique_ptr<TFile> m_file;
    TTree* m_datatree;
    TTree* m_metadatatree;
    TTree* m_runMDtree;
    TTree* m_evtMDtree;
    TTree* m_colMDtree;
    podio::CollectionIDTable m_collectionIDtable;
    podio::GenericParameters m_evtMD;
    std::map<int,podio::GenericParameters> m_colMetaDataMap;
    std::map<int,podio::GenericParameters> m_runMetaDataMap;

    std::map<std::string, std::string> m_collection_branches; // names of collections that have branches made and name of vector type
    std::vector<std::tuple<int, std::string, bool>> m_collectionInfo;

    void CreateBranch(EICEventStore::DataVector *dv);
    void ResetBranches(EICEventStore &store);

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
