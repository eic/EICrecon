
// Copyright 2022, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.


#ifndef EICRECON_H_JPODIOUTILS_H
#define EICRECON_H_JPODIOUTILS_H

#include <vector>
#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>

#include <services/io/podio/MTEventStore.h>
#include <services/log/Log_service.h>

// TODO: we could define a type dictionary to automatically figure out PodioCollectionT given PodioT

template <typename PodioT, typename PodioCollectionT>
void StorePodioData(std::vector<PodioT*> data, JFactoryT<PodioT>* factory, const std::shared_ptr<const JEvent>& event) {
    LOG << "StorePodioData: Starting save of " << data.size() << " items of type " << JTypeInfo::demangle<PodioT>() << " to collection " << factory->GetTag() << LOG_END;
    factory->Set(data);
    factory->SetFactoryFlag(JFactory::NOT_OBJECT_OWNER);

    auto* store = const_cast<eic::MTEventStore*>(event->GetSingle<eic::MTEventStore>());
    if (store == nullptr) {
        auto log = event->GetJApplication()->GetService<Log_service>()->logger("MTEventStore");
        store = new eic::MTEventStore(log);
        event->Insert(store);
    }
    auto collection_name = factory->GetTag();
    if (store->get_untyped(collection_name) != nullptr) {
        throw JException("StorePodioData: MTEventStore is not empty! %s", collection_name);
    }
    auto* collection = new PodioCollectionT;
    store->put(collection_name, collection); // TODO: Maybe put() should except if collection is already present

    for (auto* datum: data) {
        LOG << "Adding datum with object id (" << datum->getObjectID().collectionID << ", " << datum->getObjectID().index << ") and ptr " << datum << LOG_END;
        collection->push_back(*datum);
        LOG << "Added datum, object id is now (" << datum->getObjectID().collectionID << ", " << datum->getObjectID().index << ") and ptr " << datum << LOG_END;
        // TODO: Need to verify that the original datum pointers stay valid after the podio object is added to the collection.
        //       Knowing podio, they probably aren't. Maybe it makes more sense to put the podio data into a Collection first,
        //       and copy the pointers into the JFactory afterwards.
    }
    LOG << "StorePodioData: Success saving " << data.size() << " items of type " << JTypeInfo::demangle<PodioT>() << " to collection " << factory->GetTag() << LOG_END;
}



template <typename PodioT, typename PodioCollectionT>
void StorePodioData(PodioCollectionT* collection, JFactory* factory, const std::shared_ptr<const JEvent>& event) {

    std::vector<PodioT*> items;
    for (auto& item : collection) {
        items.push_back(&PodioT(item));
        // Pretty confident this doesn't do a deep copy. We need to do this because the podio objects self-delete
    }
    factory->Set(items);
    factory->SetFactoryFlag(JFactory::NOT_OBJECT_OWNER);

    auto* store = event->GetSingle<eic::MTEventStore>();
    if (store == nullptr) {
        auto log = event->GetJApplication()->GetService<Log_service>()->logger("MTEventStore");
        store = new eic::MTEventStore(log);
        event->Insert(store);
    }
    store->put(factory->GetTag(), collection);
    LOG << "StorePodioData: Saved " << items.size() << " items of type " << JTypeInfo::demangle<PodioT>() << " to collection " << factory->GetTag() << LOG_END;
}


#endif //EICRECON_H_JPODIOUTILS_H
