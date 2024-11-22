// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <podio/podioVersion.h>
#if podio_VERSION >= PODIO_VERSION(0, 99, 0)
#include <podio/ROOTReader.h>
#else
#include <podio/ROOTFrameReader.h>
#endif
#include <stddef.h>
#include <memory>
#include <set>
#include <string>

#if ((JANA_VERSION_MAJOR == 2) && (JANA_VERSION_MINOR >= 3)) || (JANA_VERSION_MAJOR > 2)
#define JANA_NEW_CALLBACK_STYLE 1
#else
#define JANA_NEW_CALLBACK_STYLE 0
#endif

class JEventSourcePODIO : public JEventSource {

public:
    JEventSourcePODIO(std::string resource_name, JApplication* app);

    virtual ~JEventSourcePODIO();

    void Open() override;

    void Close() override;

#if JANA_NEW_CALLBACK_STYLE
    Result Emit(JEvent& event) override;
#else
    void GetEvent(std::shared_ptr<JEvent>) override;
#endif

    static std::string GetDescription();

    void PrintCollectionTypeTable(void);

protected:
#if podio_VERSION >= PODIO_VERSION(0, 99, 0)
    podio::ROOTReader m_reader;
#else
    podio::ROOTFrameReader m_reader;
#endif

    size_t Nevents_in_file = 0;
    size_t Nevents_read = 0;

    std::string m_include_collections_str;
    std::string m_exclude_collections_str;
    std::set<std::string> m_INPUT_INCLUDE_COLLECTIONS;
    std::set<std::string> m_INPUT_EXCLUDE_COLLECTIONS;
    bool m_run_forever=false;

};

template <>
double JEventSourceGeneratorT<JEventSourcePODIO>::CheckOpenable(std::string);
