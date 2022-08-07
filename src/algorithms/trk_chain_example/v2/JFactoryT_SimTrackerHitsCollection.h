//
// Created by romanov on 8/6/22.
//

#ifndef EICRECON_JFACTORY_SIMTRACKERHITSCOLLECTIONT_H
#define EICRECON_JFACTORY_SIMTRACKERHITSCOLLECTIONT_H

#include <random>

#include <fmt/core.h>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <edm4hep/SimTrackerHit.h>



template <typename OutputT>
class JFactoryT_SimTrackerHitsCollection : public JFactoryT<OutputT> {

public:
    JFactoryT_SimTrackerHitsCollection() {

        // Static test that OutputType is inherited from SimTrackerHit
        static_assert(std::is_base_of<edm4hep::SimTrackerHit, OutputT>::value);
    }

    /** One time initialization **/
    void Init() override;

    /** On run change preparations **/
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override;

private:

};

template<typename OutputT>
void JFactoryT_SimTrackerHitsCollection<OutputT>::Init() {
    /** Initialization **/
    fmt::print("JFactory_BEMCRawCalorimeterHit<{}>::Init()\n", typeid(OutputT).name());
}

template<typename SimTrackerHitOutT>
void JFactoryT_SimTrackerHitsCollection<SimTrackerHitOutT>::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /** On run change preparations **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<{}>::ChangeRun(...)\n", typeid(SimTrackerHitOutT).name());

    // This assertion is basically for code autocompletion
    static_assert(std::is_base_of<edm4hep::SimTrackerHit, SimTrackerHitOutT>::value);

    // Fill sample hit with some dumb data
    std::vector<SimTrackerHitOutT*> hits;
    hits.push_back(new SimTrackerHitOutT(1, 2, 3, 4, 5, {6, 7, 8}, {9, 10, 11}));

    // Add data as a factory output
    this->Set(hits);
}

template<typename OutputT>
void JFactoryT_SimTrackerHitsCollection<OutputT>::Process(const std::shared_ptr<const JEvent> &event) {
    /** Event by event processing **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<{}>::Process(...)\n", typeid(OutputT).name());
}

#endif //EICRECON_JFACTORY_SIMTRACKERHITSCOLLECTIONT_H
