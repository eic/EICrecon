// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>

#include <podio/ROOTFrameReader.h>

class JEventSourcePODIO : public JEventSource {

public:
    JEventSourcePODIO(std::string resource_name, JApplication* app);

    virtual ~JEventSourcePODIO();

    void Open() override;

    void Close() override;

    void GetEvent(std::shared_ptr<JEvent>) override;

    static std::string GetDescription();

    void PrintCollectionTypeTable(void);

protected:
    podio::ROOTFrameReader m_reader;
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
