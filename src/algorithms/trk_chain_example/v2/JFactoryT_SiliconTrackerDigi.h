#pragma once

#include <random>

#include <fmt/core.h>

#include <services/io/podio/JFactoryPodioT.h>

#include <edm4hep/SimTrackerHit.h>

#include "algorithms/digi/RawTrackerHit.h"

template <typename InputT, typename OutputT>
class SiliconTrackerDigi_factoryT : public eicrecon::JFactoryPodioT<OutputT> {

public:
    SiliconTrackerDigi_factoryT() {

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

    std::string m_config_prefix;    /// A prefix to use for command line parameters

};

template<typename SimTrackerHitInT, typename RawTrackerHitOutT>
void SiliconTrackerDigi_factoryT<SimTrackerHitInT, RawTrackerHitOutT>::Init() {
    /** Initialization **/
    fmt::print("JFactory_BEMCRawCalorimeterHit<{}, {}>::Init()\n",
               typeid(SimTrackerHitInT).name(),
               typeid(RawTrackerHitOutT).name());

    // We will use plugin name to get parameters for correct factory
    // So if we use <plugin name>:parameter it will be:
    // BTRK:parameter FarForward:paraeter, etc. whichever plugin uses this template
    // That has limitations but the convenient in the most of the cases
    m_config_prefix = this->GetPluginName();

    //... process m_config_prefix
}

template<typename SimTrackerHitInT, typename RawTrackerHitOutT>
void SiliconTrackerDigi_factoryT<SimTrackerHitInT, RawTrackerHitOutT>::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /** On run change preparations **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<>::ChangeRun(...)\n");
}

template<typename SimTrackerHitInT, typename RawTrackerHitOutT>
void SiliconTrackerDigi_factoryT<SimTrackerHitInT, RawTrackerHitOutT>::Process(const std::shared_ptr<const JEvent> &event) {
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
