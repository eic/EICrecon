
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
    factory->Set(data);
    factory->SetFactoryFlag(JFactory::NOT_OBJECT_OWNER);

    auto* collection = new PodioCollectionT;
    for (auto* datum: data) {
        collection->push_back(*datum);
        // TODO: Need to verify that the original datum pointers stay valid after the podio object is added to the collection.
        //       Knowing podio, they probably aren't. Maybe it makes more sense to put the podio data into a Collection first,
        //       and copy the pointers into the JFactory afterwards.
    }
    auto* store = const_cast<eic::MTEventStore*>(event->GetSingle<eic::MTEventStore>());
    if (store == nullptr) {
        auto log = event->GetJApplication()->GetService<Log_service>()->logger("MTEventStore");
        store = new eic::MTEventStore(log);
        event->Insert(store);
    }
    store->put(factory->GetTag(), collection);
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
}


#endif //EICRECON_H_JPODIOUTILS_H
