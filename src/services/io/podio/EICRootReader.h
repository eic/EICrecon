#ifndef _EICRootReader_h_
#define _EICRootReader_h_

#include <string>
#include <map>

#include <TFile.h>
#include <TTree.h>

#include <podio/CollectionIDTable.h>
#include <podio/podioVersion.h>
#include <podio/GenericParameters.h>

#include <EICEventStore.h>

class EICRootReader{

public:

    EICRootReader()=default;
    ~EICRootReader();

    void OpenFile( const std::string &filename );
    podio::version::Version GetPodioVersion() const { return *m_fileVersion;}
    size_t GetNumEvents() const;
    std::vector<const EICEventStore::DataVector*> GetDataVectors(void) const;
    EICEventStore* GetEvent( size_t entry_number );
    const podio::CollectionIDTable* GetCollectionIDTable() const { return m_collectionid_table; }
    void CloseFile(){ m_file.reset(); }

    class BranchData{
    public:
        BranchData(TBranch *b):branch(b),name(b->GetName()),className(b->GetClassName()){}

        std::string name;
        std::string className;
        TBranch *branch;
        virtual void SetBranchAddress( void ) = 0;
        virtual void Swap(void *v)=0;
    };
    template <typename T>
    class BranchDataT:public BranchData{
    public:
        BranchDataT(TBranch *b):BranchData(b){ SetBranchAddress(); }
        T vec;
        T* vecptr{&vec};
        virtual void SetBranchAddress( void ) {branch->SetAddress( &vecptr );}
        virtual void Swap( void* v ){ vec.swap( *((T*)v)); }
    };



protected:

    std::shared_ptr<TFile> m_file;
    podio::CollectionIDTable* m_collectionid_table{nullptr};
    podio::version::Version* m_fileVersion{nullptr};
    std::map<int,podio::GenericParameters>* m_run_metadata{nullptr};
    std::map<int,podio::GenericParameters>* m_col_metadata{nullptr};

    TTree *m_events_tree{nullptr};
    TTree *m_metadata_tree{nullptr};
    TTree *m_run_metadata_tree{nullptr};
    TTree *m_evt_metadata_tree{nullptr};
    TTree *m_col_metadata_tree{nullptr};

    std::vector<EICEventStore::DataVector*> m_datavectors;   // pod data objects
    std::vector<EICEventStore::DataVector*> m_objidvectors;  // keep podio::ObjectID objects in separate list
};

#endif // _EICRootReader_h_

