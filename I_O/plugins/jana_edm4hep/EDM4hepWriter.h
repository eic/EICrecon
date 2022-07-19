// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#ifndef _EDM4hepWriter_h_
#define _EDM4hepWriter_h_

// podio specific includes
#include <podio/EventStore.h>
#include <podio/podioVersion.h>

#include <JANA/JEventProcessor.h>

class EDM4hepWriter : public JEventProcessor {

    // Shared state (e.g. histograms, TTrees, TFiles) live
    std::mutex m_mutex;
    
public:

    EDM4hepWriter();
    virtual ~EDM4hepWriter() = default;

    void Init() override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void Finish() override;

protected:

    std::string m_OUTPUT_FILE = "podio_output.root";
    std::string m_collection_names_str;
    std::vector<std::string> m_OUTPUT_COLLECTIONS;
    podio::EventStore store;

};


#endif // _EDM4hepWriter_h_

