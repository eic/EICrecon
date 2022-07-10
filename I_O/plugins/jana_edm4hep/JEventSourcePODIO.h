

#ifndef _JEventSourcePODIO_h_
#define  _JEventSourcePODIO_h_

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

    virtual ~JEventSourcePODIO() = default;

    void Open() override;

    void GetEvent(std::shared_ptr<JEvent>) override;
    
    static std::string GetDescription();

protected:
	podio::ROOTReader reader;
	podio::EventStore store;
	size_t Nevents_in_file = 0;
	size_t Nevents_read = 0;
	bool run_forever=false;

	template <class T, class C>
	void GetPODIOData( const char *name, std::shared_ptr <JEvent> &event );
};

template <>
double JEventSourceGeneratorT<JEventSourcePODIO>::CheckOpenable(std::string);

#endif // _JEventSourcePODIO_h_

