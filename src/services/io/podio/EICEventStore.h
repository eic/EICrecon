// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _EICEventStore_h_
#define _EICEventStore_h_

#include <podio/ObjBase.h>

/// This class is used to keep a list of collections belonging to the event.
/// This serves a similar role as the podio::EventStore class except it is
/// designed to be multi-thread capable.
///
/// Note that this does not necessarily hold all objects for the event, only
/// those defined in the PODIO (EDM4hep) Data Model. Any transient objects
/// created by JANA algorithms that are not inheriting from one of the
/// edm4hep classes will not be kept here.
///
/// Note that this class is defined completely in this header file. The
/// internal utility classes DataVector and DataVectorT store vectors of
/// the POD data that can be used for reading/writing from/to a TBranch.
class EICEventStore{
public:

    EICEventStore()= default;
    ~EICEventStore(){

//        std::cout << "EICEventStore destructor report ------------" << std::endl;
//        std::cout << "Number of   m_podio_objs: " << m_podio_objs.size() << "  m_datavectors: " << m_datavectors.size() << std::endl;
//        int Nfails = 0;
//        for( auto dv : m_datavectors ) Nfails += dv->GetVectorAddress() != *(dv->GetVectorAddressPtr());
//        std::cout << "Number of datavectors with inconsistent addresses: " << Nfails << std::endl;
//        std::cout << "--------------------------------------------" << std::endl;

        Clear();
    }

    class DataVector{
    public:
        DataVector(const std::string name, const std::string className, int collectionID=-1):name(name),className(className),collectionID(collectionID){}
        virtual ~DataVector() = default;
        std::string name;       // e.g. EventHeaders
        std::string className;  // e.g. vector<edm4hep::EventHeader>
        int collectionID;
        virtual void* GetVectorAddress()=0;
        virtual void** GetVectorAddressPtr()=0;
        virtual size_t GetVectorSize()=0;
        virtual void Swap(DataVector *dv)=0;
        virtual void SwapUnsafe(void *addr)=0; // addr should be  std::vector<OutputType>
    };

    template <typename T> // e.g. OutputType=edm4hep::EventHeaderData
    class DataVectorT:public DataVector{
    public:
        DataVectorT(const std::string name, const std::string className, int collectionID=-1):DataVector(name, className, collectionID),vecptr(&vec){}
        std::vector<T> vec;
        void  *vecptr;
        void*  GetVectorAddress(){ return &vec; }
        void** GetVectorAddressPtr(){ return &vecptr; } // ROOT TBranch wants a pointer to a variable which points to actual data object
        size_t GetVectorSize(){ return vec.size(); }

        /// Swap contents of the std::vector<> with the given DataVector. This is an efficient
        /// way to move the data contents from a vector used by TBranch without having to copy
        /// the POD data or reset the branch address.
        void   Swap(DataVector *dv){
            auto *vec_other = static_cast<std::vector<T>*>( dv->GetVectorAddress() );
            vec_other->swap(vec);
        }

        /// Same as Swap() only slightly less safe since this takes the address as a void*
        void   SwapUnsafe(void *addr){
            auto *vec_other = static_cast<std::vector<T>*>( addr );
            vec_other->swap(vec);
        }
    };

    /// Swap contents of our members with the given EICEventStore.
    /// n.b. this does NOT swap the contents of the DataVectorT objects,
    /// just the vectors holding pointers to them.
    void Swap(EICEventStore *es){
        m_datavectors.swap(es->m_datavectors);
        m_objidvectors.swap(es->m_objidvectors);
        m_podio_objs.swap(es->m_podio_objs);
    }

    /// Free all data objects. This is called from JEventSourcePODIO::FinishEvent
    /// just to free this memory up a little earlier than when the destructor of
    /// this class gets called which may not happen until later.
    void Clear(void){
        for( auto obj : m_datavectors  ) delete obj;
        for( auto obj : m_objidvectors ) delete obj;
        for( auto obj : m_podio_objs   ) delete obj;
        m_datavectors.clear();
        m_objidvectors.clear();
        m_podio_objs.clear();
    }

    std::vector<DataVector*> m_datavectors;    // pod data objects
    std::vector<DataVector*> m_objidvectors;   // keep podio::ObjectID objects in separate list
    std::vector<podio::ObjBase*> m_podio_objs; // these are
};

#endif // _EICEventStore_h_