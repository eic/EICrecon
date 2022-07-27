// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#ifndef _JEventSourcePODIO_h_
#define  _JEventSourcePODIO_h_

#include <map>

#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>

// podio specific includes
#include "podio/EventStore.h"
#include "podio/IReader.h"
#include "podio/ROOTReader.h"
#include "podio/UserDataCollection.h"
#include "podio/podioVersion.h"

class JEventSourcePODIO : public JEventSource {

    /// Add member variables here

public:
    JEventSourcePODIO(std::string resource_name, JApplication* app);

    virtual ~JEventSourcePODIO();

    void Open() override;

    void GetEvent(std::shared_ptr<JEvent>) override;
    void FinishEvent(JEvent&) override ;
    
    static std::string GetDescription();

    const std::map<std::string, std::string>& GetCollectionNames(void) const { return collection_names; }
    void PrintCollectionTypeTable(void);

protected:
	podio::ROOTReader reader;
	podio::EventStore store;
	size_t Nevents_in_file = 0;
	size_t Nevents_read = 0;
	bool m_event_in_flight = false;
	bool run_forever=false;

	// This holds the collection name (key) and the collection type (value) contained in the
	// input file. The collection type is a class name from the EDM4hep data model. Example is:
	//      collection_name["MRICHHits"] = "SimTrackerHit"
	std::map<std::string, std::string> collection_names;

};

template <>
double JEventSourceGeneratorT<JEventSourcePODIO>::CheckOpenable(std::string);

#endif // _JEventSourcePODIO_h_

