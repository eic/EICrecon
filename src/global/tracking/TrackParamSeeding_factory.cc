// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <vector>

#include "TrackParamSeeding_factory.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "extensions/string/StringHelpers.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>
#include <JANA/JEvent.h>

#include <Acts/Definitions/Units.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>

#include <edm4eic/TrackParameters.h>


void eicrecon::TrackParamSeeding_factory::Init() {
    auto app = GetApplication();

    // This prefix will be used for parameters
    std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
    std::string param_prefix = plugin_name+ ":" + GetTag();

    // Initialize input tags
    InitDataTags(param_prefix);

    // Initialize logger
    InitLogger(param_prefix, "info");

}

void eicrecon::TrackParamSeeding_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::TrackParamSeeding_factory::Process(const std::shared_ptr<const JEvent> &event) {

    using Acts::UnitConstants::GeV;
    using Acts::UnitConstants::MeV;
    using Acts::UnitConstants::mm;
    using Acts::UnitConstants::um;
    using Acts::UnitConstants::ns;

    // Get Seeds
    auto seed_params = event->Get<edm4eic::TrackParameters>(GetInputTags()[0]);

    try {
        // Produce track parameters out of Seeds
        std::vector<eicrecon::TrackParameters *> results;
        for (auto aseed: seed_params) {

 	    //Check for well-defined seeds
 	    if( std::isnan(aseed->getLoc().a)  || std::isnan(aseed->getLoc().b) ||
		std::isnan(aseed->getPhi())    || std::isnan(aseed->getTheta()) ||
		std::isnan(aseed->getQOverP()) || std::isnan(aseed->getTime()) ) continue;

            //Seed Parameters
            Acts::BoundVector  params;
            params(Acts::eBoundLoc0)   = aseed->getLoc().a * mm ;  // cylinder radius
            params(Acts::eBoundLoc1)   = aseed->getLoc().b * mm ;  // cylinder length
            params(Acts::eBoundPhi)    = aseed->getPhi();
            params(Acts::eBoundTheta)  = aseed->getTheta();
            params(Acts::eBoundQOverP) = aseed->getQOverP()/ GeV;
            params(Acts::eBoundTime)   = aseed->getTime() * ns;

            //Get charge
            double charge = aseed->getCharge();

            //Build seed Covariance matrix
            Acts::BoundSymMatrix cov                    = Acts::BoundSymMatrix::Zero();
            cov(Acts::eBoundLoc0, Acts::eBoundLoc0)     = std::pow( aseed->getLocError().xx ,2)*mm*mm;
            cov(Acts::eBoundLoc1, Acts::eBoundLoc1)     = std::pow( aseed->getLocError().yy,2)*mm*mm;
            cov(Acts::eBoundPhi, Acts::eBoundPhi)       = std::pow( aseed->getMomentumError().xx,2);
            cov(Acts::eBoundTheta, Acts::eBoundTheta)   = std::pow( aseed->getMomentumError().yy,2);
            cov(Acts::eBoundQOverP, Acts::eBoundQOverP) = std::pow( aseed->getMomentumError().zz,2) / (GeV*GeV);
            cov(Acts::eBoundTime, Acts::eBoundTime)     = std::pow( aseed->getTimeError(),2)*ns*ns;

            //Construct a perigee surface as the target surface
            auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0,0,0));

            // Do conversion
            auto result = new eicrecon::TrackParameters({pSurface, params, charge, cov});
            if (!result) continue;
            results.push_back(result);

        }
        Set(results);
    }
    catch(std::exception &e) {
        m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
    }
}
