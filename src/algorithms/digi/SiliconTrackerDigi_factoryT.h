#ifndef EICRECON_JFACTORY_SILICONTRACKERDIGIT_H
#define EICRECON_JFACTORY_SILICONTRACKERDIGIT_H

#include <random>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>

#include <edm4hep/SimTrackerHit.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/SimTrackerHitCollection.h>

#include <eicd/RawTrackerHit.h>
#include <eicd/RawTrackerHitCollection.h>
#include <eicd/MutableRawTrackerHit.h>

#include <TGeoSystemOfUnits.h>
#include <TRandomGen.h>

#include "SiliconTrackerDigiConfig.h"




template <typename InputT, typename OutputT>
class SiliconTrackerDigi_factoryT : public JFactoryT<OutputT> {

public:
    SiliconTrackerDigi_factoryT() {
        // Static test that input and output are from  RawTrackerHit
        static_assert(std::is_base_of<edm4hep::SimTrackerHit, InputT>::value);
        static_assert(std::is_base_of<eicd::RawTrackerHit, OutputT>::value);
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
    eicrecon::SiliconTrackerDigiConfig m_cfg;

    std::normal_distribution<double> m_normal_distribution;

    // Random number generation
    TRandomMixMax m_random;
    std::function<double()> m_gauss;

    // SimTrackerHit input names
    std::vector<std::string> m_input_names;
};

template<typename SimTrackerHitInT, typename RawTrackerHitOutT>
void SiliconTrackerDigi_factoryT<SimTrackerHitInT, RawTrackerHitOutT>::Init() {
    /** Initialization **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<{}, {}>::Init()\n",
               typeid(SimTrackerHitInT).name(),
               typeid(RawTrackerHitOutT).name());

    // We will use plugin name to get parameters for correct factory
    // So if we use <plugin name>:parameter whichever plugin uses this template. eg:
    //    "BTRK:parameter" or "FarForward:parameter"
    // That has limitations but the convenient in the most of the cases
    m_config_prefix = this->GetPluginName() + "_digi";   // Will be something like BTRK_digi

    // Create plugin level sub-log
    m_log = spdlog::get(m_config_prefix);

    // This level will work for this plugin only
    m_log->set_level(spdlog::level::debug);

    m_gauss = [&](){
        return m_random.Gaus(0, m_cfg.timeResolution);
    };

    // Ask service locator for parameter manager. We want to get this plugin parameters.
    auto pm = this->GetApplication()->GetJParameterManager();

    pm->SetDefaultParameter(m_config_prefix + ":verbose", m_cfg.verbose, "verbosity: 0 - none, 1 - default, 2 - debug, 3 - trace");
    pm->SetDefaultParameter(m_config_prefix + ":threshold", m_cfg.threshold, "threshold");
    pm->SetDefaultParameter(m_config_prefix + ":time_res", m_cfg.threshold, "Time resolution. Probably ns. Fix my units!!!!");
    pm->SetDefaultParameter(m_config_prefix + ":input_names", m_input_names, "Time resolution. Probably ns. Fix my units!!!!");
}


template<typename SimTrackerHitInT, typename RawTrackerHitOutT>
void SiliconTrackerDigi_factoryT<SimTrackerHitInT, RawTrackerHitOutT>::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /** On run change preparations **/

    fmt::print("JFactory_BEMCRawCalorimeterHit<>::ChangeRun(...)\n");
}


template<typename SimTrackerHitInT, typename RawTrackerHitOutT>
void SiliconTrackerDigi_factoryT<SimTrackerHitInT, RawTrackerHitOutT>::Process(const std::shared_ptr<const JEvent> &event) {
    /** Event by event processing **/
    namespace units = TGeoUnit;
    static_assert(std::is_base_of<eicd::RawTrackerHit, RawTrackerHitOutT>::value);

    // Input data
    auto simHits = event->Get<SimTrackerHitInT>();

    // A map of unique cellIDs with temporary structure RawHit
    struct RawHit {
        std::int32_t charge;
        std::int32_t time_stamp;
    };
    std::unordered_map<std::uint64_t, RawHit> cell_hit_map;


    for (const auto ahit : simHits) {

        m_log->debug("--------------------\n");
        m_log->debug("Hit cellID   = {}\n", ahit->getCellID());
        m_log->debug("   position  = ({}, {}, {})\n", ahit->getPosition().x, ahit->getPosition().y,ahit->getPosition().z);
        m_log->debug("   xy_radius = {:.2f}\n", std::hypot(ahit->getPosition().x, ahit->getPosition().y));
        m_log->debug("   momentum  = ({}, {}, {})\n", ahit->getMomentum().x, ahit->getMomentum().y,ahit->getMomentum().z);
        m_log->debug("   edep = {} \n", ahit->getEDep());

        if (ahit->getEDep() * units::keV < m_cfg.threshold) {
            m_log->debug("  edep is below threshold of {} [keV]\n", m_cfg.threshold / units::keV);
            continue;
        }

        if (cell_hit_map.count(ahit->getCellID()) == 0) {
            // This cell doesn't have hits
            cell_hit_map[ahit->getCellID()] = {
                    (std::int32_t) (ahit->getMCParticle().getTime() * 1e6 +  m_gauss() * 1e3), // ns->fs
                    (std::int32_t) std::llround(ahit->getEDep() * 1e6)};

        } else {
            // There is previous values in the cell
            RawHit &hit = cell_hit_map[ahit->getCellID()];
            hit.time_stamp = (std::int32_t) (ahit->getMCParticle().getTime() * 1e6 + m_gauss() * 1e3);
            hit.charge += (std::int32_t) std::llround(ahit->getEDep() * 1e6);
        }
    }


    // Create and fill output array
    std::vector<RawTrackerHitOutT*> rawhits(cell_hit_map.size());
    for (auto item : cell_hit_map) {
        rawhits.push_back(new RawTrackerHitOutT(
                item.first,
                item.second.charge,
                item.second.time_stamp));
    }

    // Add data as a factory output
    this->Set(rawhits);

    // >oO debug
    m_log->template trace("SiliconTrackerDigi_factoryT<>::Process(...) end\n");
}


#endif //EICRECON_JFACTORY_SILICONTRACKERDIGIT_H
