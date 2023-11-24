// Created by Shujie Li
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackerMeasurementFromHits_factory.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <edm4eic/TrackerHitCollection.h>
#include <exception>
#include <gsl/pointers>

#include "algorithms/tracking/TrackerMeasurementFromHits.h"
#include "datamodel_glue.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {


    void TrackerMeasurementFromHits_factory::Init() {

        // This prefix will be used for parameters
        std::string plugin_name = GetPluginName();
        std::string param_prefix = plugin_name+ ":" + GetTag();

        // Initialize logger
        InitLogger(GetApplication(), param_prefix, "info");

        // Get ACTS context from ACTSGeo service
        auto acts_service   = GetApplication()->GetService<ACTSGeo_service>();
        auto dd4hep_service = GetApplication()->GetService<DD4hep_service>();

        // Initialize algorithm
        m_measurement.init(dd4hep_service->detector(), dd4hep_service->converter(), acts_service->actsGeoProvider(), m_log);
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

        try {
            auto result = m_measurement.produce(total_hits);
            SetCollection<edm4eic::Measurement2D>(GetOutputTags()[0], std::move(result));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }
} // eicrecon
