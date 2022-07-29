

#include "EICRootReader.h"


void EICRootReader::OpenFile(const std::string &filename) {
    m_file = std::make_shared<TFile>( filename.c_str() );

    m_chain = new TChain("events");
    for (const auto& filename : filenames) {
        m_chain->Add(filename.c_str());
    }

    // read the meta data and build the collectionBranches cache
    // NOTE: This is a small pessimization, if we do not read all collections
    // afterwards, but it makes the handling much easier in general
    auto metadatatree = static_cast<TTree*>(m_chain->GetFile()->Get("metadata"));
    m_table = new CollectionIDTable();
    metadatatree->SetBranchAddress("CollectionIDs", &m_table);

    podio::version::Version* versionPtr{nullptr};
    if (auto* versionBranch = root_utils::getBranch(metadatatree, "PodioVersion")) {
        versionBranch->SetAddress(&versionPtr);
    }

    // Check if the CollectionTypeInfo branch is there and assume that the file
    // has been written with with podio pre #197 (<0.13.1) if that is not the case
    if (auto* collInfoBranch = root_utils::getBranch(metadatatree, "CollectionTypeInfo")) {
        auto collectionInfo = new std::vector<root_utils::CollectionInfoT>;
        collInfoBranch->SetAddress(&collectionInfo);
        metadatatree->GetEntry(0);
        createCollectionBranches(*collectionInfo);
        delete collectionInfo;
    } else {
        std::cout << "PODIO: Reconstructing CollectionTypeInfo branch from other sources in file: \'"
                  << m_chain->GetFile()->GetName() << "\'" << std::endl;
        metadatatree->GetEntry(0);
        const auto collectionInfo = root_utils::reconstructCollectionInfo(m_chain, *m_table);
        createCollectionBranches(collectionInfo);
    }

    m_fileVersion = versionPtr ? *versionPtr : podio::version::Version{0, 0, 0};
    delete versionPtr;
}
