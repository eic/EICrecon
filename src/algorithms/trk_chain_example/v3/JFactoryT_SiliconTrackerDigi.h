#pragma once

#include <random>

#include <fmt/core.h>

#include <services/io/podio/JFactoryPodioT.h>

#include <edm4hep/SimTrackerHit.h>

#include "algorithms/digi/RawTrackerHit.h"
#include "SimTrackerHitDigi.h"

template <typename InputT, typename OutputT>
class JFactoryT_SiliconTrackerDigi : public eicrecon::JFactoryPodioT<OutputT>, SimTrackerHitDigi {

public:
    JFactoryT_SiliconTrackerDigi() {

        // Static test that input and output are from  RawTrackerHit
        static_assert(std::is_base_of<edm4hep::SimTrackerHit, InputT>::value);
        static_assert(std::is_base_of<eicrecon::RawTrackerHit, OutputT>::value);

        // Tell algorithm to create objects of type RawTrackerHitOutT
        new_RawTrackerHit = [](double one, double two, double three){new OutputT(one, two, three);};
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
void JFactoryT_SiliconTrackerDigi<SimTrackerHitInT, RawTrackerHitOutT>::Init() {
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
void JFactoryT_SiliconTrackerDigi<SimTrackerHitInT, RawTrackerHitOutT>::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /** On run change preparations **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<>::ChangeRun(...)\n");
}

template<typename SimTrackerHitInT, typename RawTrackerHitOutT>
void JFactoryT_SiliconTrackerDigi<SimTrackerHitInT, RawTrackerHitOutT>::Process(const std::shared_ptr<const JEvent> &event) {
    /** Event by event processing **/

    // Read input data directly into SimTrackerHitDigi::m_simhits
    auto m_simHits = event->Get<SimTrackerHitInT>(); // (this will eventually need to specify a tag, but OK for this example)

    // Call SimTrackerHitDigi::execute() to fill in m_hits member
    execute();

    // Convert output into pointers to RawTrackerHitOutT
    std::vector<RawTrackerHitOutT*> hits;
    std::transform(m_hits.begin(), m_hits.end(), hits.begin(), [](T* t) { return rinterpret_cast<RawTrackerHitOutT*>(t); });

    // Add data as a factory output
    this->Set(hits);

    fmt::print("JFactory_BEMCRawCalorimeterHit<>::Process(...)\n");
}
