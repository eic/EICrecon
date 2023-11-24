// Created by Shujie Li
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackerMeasurementFromHits_factory.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/Services/JParameterManager.h>
#include <edm4eic/TrackerHitCollection.h>
#include <exception>
#include <gsl/pointers>

#include "algorithms/tracking/TrackerMeasurementFromHits.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/io/podio/JFactoryPodioT.h"
#include "services/log/Log_service.h"

namespace eicrecon {


    void TrackerMeasurementFromHits_factory::Init() {
        // Ask JApplication and parameter managers
        auto *app =  this->GetApplication();
        auto *pm = app->GetJParameterManager();

        // This prefix will be used for parameters
        std::string plugin_name = GetPluginName();
        std::string param_prefix = plugin_name+ ":" + GetTag();

        // Now we check that user provided an input names
        pm->SetDefaultParameter(param_prefix + ":InputTags", m_input_tags, "Input data tag name");
        if(m_input_tags.empty()) {
            m_input_tags = GetDefaultInputTags();
        }

        // Logger and log level from user parameter or default
        m_log = app->GetService<Log_service>()->logger(param_prefix);
        std::string log_level_str = "info";
        pm->SetDefaultParameter(param_prefix + ":LogLevel", log_level_str, "Log level: trace, debug, info, warn, err, critical, off");
        m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

        // Get ACTS context from ACTSGeo service
        auto acts_service   = GetApplication()->GetService<ACTSGeo_service>();

        auto dd4hep_service = GetApplication()->GetService<DD4hep_service>();

        // Initialize algorithm
        m_measurement.init(dd4hep_service->detector(), dd4hep_service->converter(), acts_service->actsGeoProvider(), m_log);
    }




    void TrackerMeasurementFromHits_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

    }

    void TrackerMeasurementFromHits_factory::Process(const std::shared_ptr<const JEvent> &event) {
        // Collect all hits
        edm4eic::TrackerHitCollection total_hits;
        total_hits.setSubsetCollection();

        for (const auto& input_tag: m_input_tags) {
            auto hits = static_cast<const edm4eic::TrackerHitCollection*>(event->GetCollectionBase(input_tag));
            for (const auto& hit : *hits) {
                total_hits.push_back(hit);
            }
        }
        m_log->debug("TrackerMeasurementFromHits_factory::Process");

        try {
            auto result = m_measurement.produce(total_hits);
            SetCollection(std::move(result));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }
} // eicrecon
