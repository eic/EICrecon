// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackSeeding.h"


#include <TDatabasePDG.h>

void eicrecon::TrackSeeding::init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> log) {

    m_log = log;

    m_geoSvc = geo_svc;

    m_BField = std::dynamic_pointer_cast<const eicrecon::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
    m_fieldctx = eicrecon::BField::BFieldVariant(m_BField);

    // eta bins, chi2 and #sourclinks per surface cutoffs
    m_sourcelinkSelectorCfg = {
            {Acts::GeometryIdentifier(),
             {m_cfg.m_etaBins, m_cfg.m_chi2CutOff,
              {m_cfg.m_numMeasurementsCutOff.begin(), m_cfg.m_numMeasurementsCutOff.end()}
             }
            },
    };


}

std::vector<edm4eic::TrackParameters*> eicrecon::TrackSeeding::produce(std::vector<const edm4eic::TrackerHit*> trk_hits) {

    // getGeneratorStatus = 1 means thrown G4Primary
    std::vector<edm4eic::TrackParameters*> result;

    return result;
}
