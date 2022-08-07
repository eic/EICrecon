#ifndef EICRECON_JFACTORY_SILICONTRACKERDIGIT_H
#define EICRECON_JFACTORY_SILICONTRACKERDIGIT_H

#include <random>

#include <fmt/core.h>

#include <JANA/JFactoryT.h>
#include "algorithms/digi/RawTrackerHit.h"

template <typename OutputType>
class JFactoryT_SiliconTrackerDigi : public JFactoryT<OutputType> {

public:
    JFactoryT_SiliconTrackerDigi() {

        // Static test that OutputType is inherited from RawTrackerHit
        static_assert(std::is_base_of<eicrecon::RawTrackerHit, OutputType>::value);
    }

    /** One time initialization **/
    void Init() override;

    /** On run change preparations **/
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override;

private:

};

template<typename OutputType>
void JFactoryT_SiliconTrackerDigi<OutputType>::Init() {
    /** Initialization **/
    fmt::print("JFactory_BEMCRawCalorimeterHit<{}>::Init()\n", typeid(OutputType).name());
}

template<typename OutputType>
void JFactoryT_SiliconTrackerDigi<OutputType>::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /** On run change preparations **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<{}>::ChangeRun(...)\n", typeid(OutputType).name());
}

template<typename OutputType>
void JFactoryT_SiliconTrackerDigi<OutputType>::Process(const std::shared_ptr<const JEvent> &event) {
    /** Event by event processing **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<{}>::Process(...)\n", typeid(OutputType).name());
}


#endif //EICRECON_JFACTORY_SILICONTRACKERDIGIT_H
