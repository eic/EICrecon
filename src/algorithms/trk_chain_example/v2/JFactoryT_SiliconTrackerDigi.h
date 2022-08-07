#ifndef EICRECON_JFACTORY_SILICONTRACKERDIGIT_H
#define EICRECON_JFACTORY_SILICONTRACKERDIGIT_H

#include <random>

#include <fmt/core.h>

#include <JANA/JFactoryT.h>

#include <edm4hep/SimTrackerHit.h>

#include "algorithms/digi/RawTrackerHit.h"

template <typename InputT, typename OutputT>
class JFactoryT_SiliconTrackerDigi : public JFactoryT<OutputT> {

public:
    JFactoryT_SiliconTrackerDigi() {

        // Static test that input and output are from  RawTrackerHit
        static_assert(std::is_base_of<edm4hep::SimTrackerHit, InputT>::value);
        static_assert(std::is_base_of<eicrecon::RawTrackerHit, OutputT>::value);
    }

    /** One time initialization **/
    void Init() override;

    /** On run change preparations **/
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override;

private:

};

template<typename SimTrackerHitInT, typename RawTrackerHitOutT>
void JFactoryT_SiliconTrackerDigi<SimTrackerHitInT, RawTrackerHitOutT>::Init() {
    /** Initialization **/
    fmt::print("JFactory_BEMCRawCalorimeterHit<{}, {}>::Init()\n",
               typeid(SimTrackerHitInT).name(),
               typeid(RawTrackerHitOutT).name());
}

template<typename SimTrackerHitInT, typename RawTrackerHitOutT>
void JFactoryT_SiliconTrackerDigi<SimTrackerHitInT, RawTrackerHitOutT>::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /** On run change preparations **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<>::ChangeRun(...)\n");
}

template<typename SimTrackerHitInT, typename RawTrackerHitOutT>
void JFactoryT_SiliconTrackerDigi<SimTrackerHitInT, RawTrackerHitOutT>::Process(const std::shared_ptr<const JEvent> &event) {
    /** Event by event processing **/

    // Read input data
    auto simHits = event->Get<SimTrackerHitInT>();

    // Fill sample hit with some dumb data
    std::vector<RawTrackerHitOutT*> hits;
    hits.push_back(new RawTrackerHitOutT(1, 2, 3));

    // Add data as a factory output
    this->Set(hits);

    fmt::print("JFactory_BEMCRawCalorimeterHit<>::Process(...)\n");
}


#endif //EICRECON_JFACTORY_SILICONTRACKERDIGIT_H
