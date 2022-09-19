// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _JEventSourcePODIOsimple_h_
#define  _JEventSourcePODIOsimple_h_

#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>

#include <services/io/podio/ROOTReader.h>
#include <services/io/podio/EventStore.h>

class JEventSourcePODIOsimple : public JEventSource {

public:
    JEventSourcePODIOsimple(std::string resource_name, JApplication* app);

    virtual ~JEventSourcePODIOsimple();

    void Open() override;

    void GetEvent(std::shared_ptr<JEvent>) override;
    void FinishEvent(JEvent&) override ;

    static std::string GetDescription();

    void PrintCollectionTypeTable(void);

protected:
    eic::ROOTReader reader;
    eic::EventStore store;
    size_t Nevents_in_file = 0;
    size_t Nevents_read = 0;

    // User may optionally specify a file of background events to overlay
    std::vector<std::tuple<eic::ROOTReader*, eic::EventStore*, uint64_t>> readers_background; // uint64_t is current event read

    std::string m_include_collections_str;
    std::string m_exclude_collections_str;
    std::set<std::string> m_INPUT_INCLUDE_COLLECTIONS;
    std::set<std::string> m_INPUT_EXCLUDE_COLLECTIONS;
    bool m_run_forever=false;

    bool m_inflight = false; // is an event currently in flight (and therefore using the EventStore)?

};

template <>
double JEventSourceGeneratorT<JEventSourcePODIOsimple>::CheckOpenable(std::string);

#endif // _JEventSourcePODIOsimple_h_

