// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#ifndef _JEventSourcePODIO_h_
#define  _JEventSourcePODIO_h_

#include <map>

#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>

#include <services/io/podio/EICEventStore.h>
#include <services/io/podio/EICRootReader.h>


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
	bool run_forever=false;
};

template <>
double JEventSourceGeneratorT<JEventSourcePODIO>::CheckOpenable(std::string);

#endif // _JEventSourcePODIO_h_

