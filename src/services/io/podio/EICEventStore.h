#ifndef _EICEventStore_h_
#define _EICEventStore_h_

#include <podio/ObjBase.h>

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

        FreeEventObjects();
        for( auto obj : m_datavectors ) delete obj;
    }

    class DataVector{
    public:
        DataVector(const std::string name, const std::string className):name(name),className(className){}
        virtual ~DataVector() = default;
        std::string name;
        std::string className;
        virtual void* GetVectorAddress()=0;
        virtual void** GetVectorAddressPtr()=0;
        virtual size_t GetVectorSize()=0;
        virtual void Swap(DataVector *dv)=0;
    };

    template <typename T> // e.g. T=edm4hep::EventHeaderData
    class DataVectorT:public DataVector{
    public:
        DataVectorT(const std::string name, const std::string className):DataVector(name, className),vecptr(&vec){}
        std::vector<T> vec;
        void *vecptr;
        void* GetVectorAddress(){ return &vec; }
        void** GetVectorAddressPtr(){ return &vecptr; } // ROOT TBranch wants a pointer to a variable which points to actual data object
        size_t GetVectorSize(){ return vec.size(); }
        void Swap(DataVector *dv){
            auto *vec_other = static_cast<std::vector<T>*>( dv->GetVectorAddress() );
            vec_other->swap(vec);
        }
    };

    // Free all of the underlying podio "Obj" objects allocated in CopyToJEventT
    // in JEventSourcePODIO.cc. This is called from JEventSourcePODIO::FinishEvent()
    void FreeEventObjects(void){
        for( auto obj : m_podio_objs ) delete obj;
        m_podio_objs.clear();
    }

    std::vector<DataVector*> m_datavectors;
    std::vector<podio::ObjBase*> m_podio_objs;
};

#endif // _EICEventStore_h_