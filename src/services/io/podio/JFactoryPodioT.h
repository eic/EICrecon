

// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>
#include <podio/Frame.h>
#include "datamodel_glue.h"


namespace eicrecon {

template <typename T>
class JFactoryPodioT : public JFactoryT<T>, public JFactoryPodio {
public:
    using CollectionT = typename PodioTypeMap<T>::collection_t;
private:
    // mCollection is owned by the frame.
    // mFrame is owned by the JFactoryT<podio::Frame>.
    // mData holds lightweight value objects which hold a pointer into mCollection.
    // This factory owns these value objects.

public:
    explicit JFactoryPodioT();
    ~JFactoryPodioT() override;

    void Init() override {}
    void BeginRun(const std::shared_ptr<const JEvent>&) override {}
    void ChangeRun(const std::shared_ptr<const JEvent>&) override {}
    void Process(const std::shared_ptr<const JEvent>&) override {}
    void EndRun() override {}
    void Finish() override {}

    void Create(const std::shared_ptr<const JEvent>& event) final;
    std::type_index GetObjectType() const final { return std::type_index(typeid(T)); }
    std::size_t GetNumObjects() const final { return mCollection->size(); }
    void ClearData() final;

    void SetCollection(CollectionT&& collection);
    void SetCollection(std::unique_ptr<CollectionT> collection);
    void Set(const std::vector<T*>& aData) final;
    void Set(std::vector<T*>&& aData) final;
    void Insert(T* aDatum) final;



private:
    // This is meant to be called by JEvent::Insert
    friend class JEvent;
    void SetCollectionAlreadyInFrame(const CollectionT* collection);

};


template <typename T>
JFactoryPodioT<T>::JFactoryPodioT() = default;

template <typename T>
JFactoryPodioT<T>::~JFactoryPodioT() {
    // Ownership of mData, mCollection, and mFrame is complicated, so we always handle it via ClearData()
    ClearData();
}

template <typename T>
void JFactoryPodioT<T>::SetCollection(typename PodioTypeMap<T>::collection_t&& collection) {
    /// Provide a PODIO collection. Note that PODIO assumes ownership of this collection, and the
    /// collection pointer should be assumed to be invalid after this call

    if (this->mFrame == nullptr) {
        throw JException("JFactoryPodioT: Unable to add collection to frame as frame is missing!");
    }
    const auto& moved = this->mFrame->put(std::move(collection), this->GetTag());
    this->mCollection = &moved;

    for (const T& item : moved) {
        T* clone = new T(item);
        this->mData.push_back(clone);
    }
    this->mStatus = JFactory::Status::Inserted;
    this->mCreationStatus = JFactory::CreationStatus::Inserted;
}


template <typename T>
void JFactoryPodioT<T>::SetCollection(std::unique_ptr<typename PodioTypeMap<T>::collection_t> collection) {
    /// Provide a PODIO collection. Note that PODIO assumes ownership of this collection, and the
    /// collection pointer should be assumed to be invalid after this call

    if (this->mFrame == nullptr) {
        throw JException("JFactoryPodioT: Unable to add collection to frame as frame is missing!");
    }
    this->mFrame->put(std::move(collection), this->GetTag());
    const auto* moved = &this->mFrame->template get<typename PodioTypeMap<T>::collection_t>(this->GetTag());
    this->mCollection = moved;

    for (const T& item : *moved) {
        T* clone = new T(item);
        this->mData.push_back(clone);
    }
    this->mStatus = JFactory::Status::Inserted;
    this->mCreationStatus = JFactory::CreationStatus::Inserted;
}


template <typename T>
void JFactoryPodioT<T>::ClearData() {
    for (auto p : this->mData) delete p;
    this->mData.clear();
    this->mCollection = nullptr;  // Collection is owned by the Frame, so we ignore here
    this->mFrame = nullptr;  // Frame is owned by the JEvent, so we ignore here
    if (this->mStatus != JFactory::Status::Uninitialized) {
        this->mStatus = JFactory::Status::Unprocessed;
    }
    this->mCreationStatus = JFactory::CreationStatus::NotCreatedYet;

}

template <typename T>
void JFactoryPodioT<T>::SetCollectionAlreadyInFrame(const CollectionT* collection) {
    for (const T& item : *collection) {
        T* clone = new T(item);
        this->mData.push_back(clone);
    }
    this->mCollection = collection;
    this->mStatus = JFactory::Status::Inserted;
    this->mCreationStatus = JFactory::CreationStatus::Inserted;
}

template <typename T>
void JFactoryPodioT<T>::Create(const std::shared_ptr<const JEvent>& event) {
    mFrame = GetOrCreateFrame(event);
    if (this->mApp == nullptr) this->mApp = event->GetJApplication();
    auto run_number = event->GetRunNumber();

    if (this->mStatus == JFactory::Status::Uninitialized) {
        try {
            std::call_once(this->mInitFlag, &JFactory::Init, this);
            this->mStatus = JFactory::Status::Unprocessed;
        }
        catch (JException& ex) {
            ex.plugin_name = this->mPluginName;
            ex.component_name = this->mFactoryName;
            throw ex;
        }
        catch (...) {
            auto ex = JException("Unknown exception in JFactoryT::Init()");
            ex.nested_exception = std::current_exception();
            ex.plugin_name = this->mPluginName;
            ex.component_name = this->mFactoryName;
            throw ex;
        }
    }
    if (JFactory::mStatus == JFactory::Status::Unprocessed) {
        if (this->mPreviousRunNumber == -1) {
            // This is the very first run
            ChangeRun(event);
            BeginRun(event);
            this->mPreviousRunNumber = run_number;
        }
        else if (this->mPreviousRunNumber != run_number) {
            // This is a later run, and it has changed
            EndRun();
            ChangeRun(event);
            BeginRun(event);
            this->mPreviousRunNumber = run_number;
        }
        try {
            Process(event);
        }
        catch(...) {
            if (mCollection == nullptr) {
                SetCollection(typename PodioTypeMap<T>::collection_t());
            }
            throw;
        }
        if (mCollection == nullptr) {
            SetCollection(typename PodioTypeMap<T>::collection_t());
            // If calling Process() didn't result in a call to Set() or SetCollection(), we create an empty collection
            // so that podio::ROOTFrameWriter doesn't segfault on the null mCollection pointer
        }
        this->mStatus = JFactory::Status::Processed;
        this->mCreationStatus = JFactory::CreationStatus::Created;
    }
}

template <typename T>
void JFactoryPodioT<T>::Set(const std::vector<T*>& aData) {
    typename PodioTypeMap<T>::collection_t collection;
    if (mIsSubsetCollection) collection.setSubsetCollection(true);
    for (T* item : aData) {
        collection.push_back(*item);
    }
    SetCollection(std::move(collection));
}

template <typename T>
void JFactoryPodioT<T>::Set(std::vector<T*>&& aData) {
    typename PodioTypeMap<T>::collection_t collection;
    if (mIsSubsetCollection) collection.setSubsetCollection(true);
    for (T* item : aData) {
        collection.push_back(*item);
    }
    SetCollection(std::move(collection));
}

template <typename T>
void JFactoryPodioT<T>::Insert(T* aDatum) {
    typename PodioTypeMap<T>::collection_t collection;
    if (mIsSubsetCollection) collection->setSubsetCollection(true);
    collection->push_back(*aDatum);
    SetCollection(std::move(collection));
}

} // namespace eicrecon
