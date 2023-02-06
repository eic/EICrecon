// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#pragma once

#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>

#include <services/io/podio/EICEventStore.h>
#include <services/io/podio/EICRootReader.h>

/// JANA Event source that can read from podio/edm4hep root files.
class JEventSourcePODIO : public JEventSource {

public:
    JEventSourcePODIO(std::string resource_name, JApplication* app);

    virtual ~JEventSourcePODIO();

    void Open() override;

    void GetEvent(std::shared_ptr<JEvent>) override;
    void FinishEvent(JEvent&) override ;

    static std::string GetDescription();

    void PrintCollectionTypeTable(void);

protected:
	EICRootReader reader;
	size_t Nevents_in_file = 0;
	size_t Nevents_read = 0;

    std::string m_include_collections_str;
    std::string m_exclude_collections_str;
    std::set<std::string> m_INPUT_INCLUDE_COLLECTIONS;
    std::set<std::string> m_INPUT_EXCLUDE_COLLECTIONS;
	bool run_forever=false;
};

template <>
double JEventSourceGeneratorT<JEventSourcePODIO>::CheckOpenable(std::string);
