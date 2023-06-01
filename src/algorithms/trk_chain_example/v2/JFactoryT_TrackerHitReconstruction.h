#pragma once


#include <fmt/core.h>

#include <services/io/podio/JFactoryPodioT.h>
#include <edm4hep/TrackerHit.h>



template <typename OutputType>
class JFactoryT_TrackerHitReconstruction : public eicrecon::JFactoryPodioT<OutputType> {

public:
    JFactoryT_TrackerHitReconstruction() {

        // Static test that OutputType is inherited from SimTrackerHit
        static_assert(std::is_base_of<edm4hep::TrackerHit, OutputType>::value);
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
void JFactoryT_TrackerHitReconstruction<OutputType>::Init() {
    /** Initialization **/
    fmt::print("JFactory_BEMCRawCalorimeterHit<{}>::Init()\n", typeid(OutputType).name());
}

template<typename OutputType>
void JFactoryT_TrackerHitReconstruction<OutputType>::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /** On run change preparations **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<{}>::ChangeRun(...)\n", typeid(OutputType).name());
}

template<typename OutputType>
void JFactoryT_TrackerHitReconstruction<OutputType>::Process(const std::shared_ptr<const JEvent> &event) {
    /** Event by event processing **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<{}>::Process(...)\n", typeid(OutputType).name());
}
