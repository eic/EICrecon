//
// Created by romanov on 8/6/22.
//

#pragma once

#include <random>

#include <fmt/core.h>

#include <services/io/podio/JFactoryPodioT.h>
#include <JANA/JEvent.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <edm4hep/SimTrackerHit.h>
#include <algorithms/interfaces/ICollectionProducer.h>


template <typename OutputT>
class JFactoryT_SimTrackerHitsCollector : public eicrecon::JFactoryPodioT<OutputT>, eicrecon::ICollectionProducer {

public:
    JFactoryT_SimTrackerHitsCollector() {

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

    std::string m_config_prefix;    /// A prefix to use for command line parameters
};

template<typename OutputT>
void JFactoryT_SimTrackerHitsCollector<OutputT>::Init() {
    /** Initialization **/
    fmt::print("JFactoryT_SimTrackerHitsCollector<{}>::Init()\n", typeid(OutputT).name());

    // We will use plugin name to get parameters for correct factory
    // So if we use <plugin name>:parameter it will be:
    // BTRK:parameter FarForward:paraeter, etc. whichever plugin uses this template
    // That has limitations but the convenient in the most of the cases
    m_config_prefix = this->GetPluginName();

    // plugin name comes like BTRK.so we don't need this extension
    std::string::size_type pos = m_config_prefix.find('.');
    if (pos != std::string::npos) {
        m_config_prefix =  m_config_prefix.substr(0, pos);
    } else {
        fmt::print("  WARNING GetPluginName() = '{}'\n. Awaited extension like .so", m_config_prefix);
    }

    // >oO Debug
    fmt::print("  m_config_prefix = '{}'\n", m_config_prefix);

}

template<typename SimTrackerHitOutT>
void JFactoryT_SimTrackerHitsCollector<SimTrackerHitOutT>::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /** On run change preparations **/

    fmt::print("JFactoryT_SimTrackerHitsCollector<{}>::ChangeRun(...)\n", typeid(SimTrackerHitOutT).name());
}

template<typename SimTrackerHitOutT>
void JFactoryT_SimTrackerHitsCollector<SimTrackerHitOutT>::Process(const std::shared_ptr<const JEvent> &event) {
    /** Event by event processing **/

    fmt::print("JFactoryT_SimTrackerHitsCollector<{}>::Process(...)\n", typeid(SimTrackerHitOutT).name());

    auto simhits = event->Get<edm4hep::SimTrackerHit>( "TrackerBarrelHits" );

    // This assertion is basically for code autocompletion
    static_assert(std::is_base_of<edm4hep::SimTrackerHit, SimTrackerHitOutT>::value);

    // Fill sample hit with some dumb data
    std::vector<SimTrackerHitOutT*> hits;
    hits.push_back(new SimTrackerHitOutT(1, 2, 3, 4, 5, {6, 7, 8}, {9, 10, 11}));

    // Add data as a factory output
    this->Set(hits);
}
