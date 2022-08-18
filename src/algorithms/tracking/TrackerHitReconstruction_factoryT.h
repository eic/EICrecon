#ifndef EICRECON_TRACKER_HIT_RECONSTRUCTION_FACTORYT_H
#define EICRECON_TRACKER_HIT_RECONSTRUCTION_FACTORYT_H

#include <random>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <JANA/JFactoryT.h>

#include <edm4hep/SimTrackerHit.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/SimTrackerHitCollection.h>

#include <eicd/RawTrackerHit.h>
#include <eicd/RawTrackerHitCollection.h>
#include <eicd/MutableRawTrackerHit.h>

#include <TGeoSystemOfUnits.h>
#include <TRandomGen.h>
#include <eicd/MutableTrackerHit.h>

#include "TrackerHitReconstructionConfig.h"


template <typename InputT, typename OutputT>
class TrackerHitReconstruction_factoryT : public JFactoryT<OutputT> {

public:
    TrackerHitReconstruction_factoryT() {
        // Static test that input and output are from  RawTrackerHit
        static_assert(std::is_base_of<edm4hep::SimTrackerHit, InputT>::value);
        static_assert(std::is_base_of<eicd::MutableRawTrackerHit, OutputT>::value);
    }

    /** One time initialization **/
    void Init() override;

    /** On run change preparations **/
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override;

private:

    std::string m_config_prefix;    /// A prefix to use for command line parameters

    std::shared_ptr<spdlog::logger> m_log;

    // This config
    eicrecon::TrackerHitReconstructionConfig m_cfg;

    // Random number generation
    TRandomMixMax m_random;
    std::function<double()> m_gauss;
};

template<typename SimTrackerHitInT, typename RawTrackerHitOutT>
void TrackerHitReconstruction_factoryT<SimTrackerHitInT, RawTrackerHitOutT>::Init() {
    /** Initialization **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<{}, {}>::Init()\n",
               typeid(SimTrackerHitInT).name(),
               typeid(RawTrackerHitOutT).name());

    // We will use plugin name to get parameters for correct factory
    // So if we use <plugin name>:parameter whichever plugin uses this template. eg:
    //    "BTRK:parameter" or "FarForward:parameter"
    // That has limitations but the convenient in the most of the cases
    m_config_prefix = this->GetPluginName()+"_trk_hit_reco";

    // Create plugin level sub-log
    m_log = spdlog::get(m_config_prefix);

    // This level will work for this plugin only
    m_log->set_level(spdlog::level::debug);

    m_gauss = [&](){
        return 0;// m_random.Gaus(0, m_cfg.timeResolution);
    };

}


template<typename RawTrackerHitInT, typename TrackerHitOutT>
void TrackerHitReconstruction_factoryT<RawTrackerHitInT, TrackerHitOutT>::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /** On run change preparations **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<>::ChangeRun(...)\n");
}


template<typename RawTrackerHitInT, typename TrackerHitOutT>
void TrackerHitReconstruction_factoryT<RawTrackerHitInT, TrackerHitOutT>::Process(const std::shared_ptr<const JEvent> &event) {
    /** Event by event processing **/
    namespace units = TGeoUnit;
    static_assert(std::is_base_of<eicd::MutableTrackerHit, TrackerHitOutT>::value);

    // Read input data
    auto raw_hits = event->Get<RawTrackerHitInT>();

    // Add data as a factory output
    //this->Set(rawhits);

    // >oO debug
    m_log->template trace("TrackerHitReconstruction_factoryT<>::Process(...) end\n");
}


#endif //EICRECON_TRACKER_HIT_RECONSTRUCTION_FACTORYT_H
