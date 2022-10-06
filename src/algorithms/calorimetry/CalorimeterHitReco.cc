
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Wouter Deconinck, Chao, Whitney Armstrong

// Reconstruct digitized outputs, paired with Jug::Digi::CalorimeterHitDigi
// Author: Chao Peng
// Date: 06/14/2021

#include "CalorimeterHitReco.h"

#include <JANA/JEvent.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>

using namespace dd4hep;

//this algorithm converted from https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/CalorimeterHitReco.cpp


//------------------------
// AlgorithmInit
//------------------------
void CalorimeterHitReco::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {

    //unitless conversion
    m_logger=logger;
    dyRangeADC = m_dyRangeADC / GeV;
    // threshold for firing
    thresholdADC = m_thresholdFactor * m_pedSigmaADC + m_thresholdValue;
    // TDC channels to timing conversion
    stepTDC = ns / m_resolutionTDC;

    // do not get the layer/sector ID if no readout class provided
    if (m_readout.empty()) {
        return;
    }

    auto id_spec = m_geoSvc->detector()->readout(m_readout).idSpec();
    try {
        id_dec = id_spec.decoder();
        if (!m_sectorField.empty()) {
            sector_idx = id_dec->index(m_sectorField);
            //LOG_INFO(default_cerr_logger) << "Find sector field " << m_sectorField << ", index = " << sector_idx  << LOG_END;
            m_logger->info("Find sector field {}, index = {}", m_sectorField, sector_idx);
        }
        if (!m_layerField.empty()) {
            layer_idx = id_dec->index(m_layerField);
            //LOG_INFO(default_cerr_logger) << "Find layer field " << m_layerField << ", index = " << sector_idx << LOG_END;
            m_logger->info("Find layer field {}, index = {}", m_layerField, sector_idx);
        }
    } catch (...) {
        //LOG_ERROR(default_cerr_logger) << "Failed to load ID decoder for " << m_readout << LOG_END;
        m_logger->error("Failed to load ID decoder for {}", m_readout);
        return;
    }


    // local detector name has higher priority
    if (!m_localDetElement.empty()) {
        try {
            local = m_geoSvc->detector()->detector(m_localDetElement);
            //LOG_INFO(default_cerr_logger) << "local coordinate system from DetElement " << m_localDetElement << LOG_END;
            m_logger->info("local coordinate system from DetElement {}", m_localDetElement);
        } catch (...) {
            //LOG_ERROR(default_cerr_logger) << "failed to load local coordinate system from DetElement " << m_localDetElement << LOG_END;
            m_logger->error("failed to load local coordinate system from DetElement {}", m_localDetElement);
            return;
        }
    } else {
        std::vector <std::pair<std::string, int >> fields;
        for (auto f : u_localDetFields) {
            fields.emplace_back(f, 0);
        }
        local_mask = id_spec.get_mask(fields);
        // use all fields if nothing provided
        if (fields.empty()) {
            local_mask = ~0;
        }
        // TODO: Fix the broken fmt::join for the fields type
//    LOG_INFO(default_cerr_logger) << fmt::format("Local DetElement mask {:#064b} from fields [{}]", local_mask, fmt::join(fields, ", "))
//				  << LOG_END;

    }

    return;
}

//------------------------
// AlgorithmChangeRun
//------------------------
void CalorimeterHitReco::AlgorithmChangeRun() {
}

//------------------------
// AlgorithmProcess
//------------------------
void CalorimeterHitReco::AlgorithmProcess() {

    auto converter = m_geoSvc->cellIDPositionConverter();
    for (const auto rh: rawhits) {
//        #pragma GCC diagnostic push
//        #pragma GCC diagnostic error "-Wsign-converstion"

        //did not pass the zero-suppresion threshold
        if (rh->getAmplitude() < m_pedMeanADC + thresholdADC) {
            continue;
        }

        // convert ADC to energy
        const float energy =
                (((signed) rh->getAmplitude() - (signed) m_pedMeanADC)) / static_cast<float>(m_capADC) * dyRangeADC /
                m_sampFrac;
        const float time = rh->getTimeStamp() / stepTDC;

//        #pragma GCC diagnostic pop

        const auto cellID = rh->getCellID();
        const int lid =
                id_dec != nullptr && !m_layerField.empty() ? static_cast<int>(id_dec->get(cellID, layer_idx)) : -1;
        const int sid =
                id_dec != nullptr && !m_sectorField.empty() ? static_cast<int>(id_dec->get(cellID, sector_idx)) : -1;

        dd4hep::Position gpos;
        try {
            // global positions
            gpos = converter->position(cellID);

            // local positions
            if (m_localDetElement.empty()) {
                auto volman = m_geoSvc->detector()->volumeManager();
                local = volman.lookupDetElement(cellID & local_mask);
            }
        } catch (...) {
            // Error looking up cellID. Messages should already have been printed
            // so just skip this hit. User will decide what to do with error messages
            continue;
        }

        const auto pos = local.nominal().worldToLocal(gpos);
//                dd4hep::Position(gpos.x(), gpos.y(), gpos.z()));//dd4hep::Position(gpos.x, gpos.y, gpos.z)
        std::vector<double> cdim;
        // get segmentation dimensions
        if (converter->findReadout(local).segmentation().type() != "NoSegmentation") {
            cdim = converter->cellDimensions(cellID);
        } else {
            // Using bounding box instead of actual solid so the dimensions are always in dim_x, dim_y, dim_z
            cdim = converter->findContext(cellID)->volumePlacement().volume().boundingBox().dimensions();
            std::transform(cdim.begin(), cdim.end(), cdim.begin(),
                           std::bind(std::multiplies<double>(), std::placeholders::_1, 2));
        }

        //create constant vectors for passing to hit initializer list
        //FIXME: needs to come from the geometry service/converter
        const decltype(edm4eic::CalorimeterHitData::position) position(gpos.x() / m_lUnit, gpos.y() / m_lUnit,
                                                                    gpos.z() / m_lUnit);
        const decltype(edm4eic::CalorimeterHitData::dimension) dimension(cdim[0] / m_lUnit, cdim[1] / m_lUnit,
                                                                      cdim[2] / m_lUnit);
        const decltype(edm4eic::CalorimeterHitData::local) local_position(pos.x() / m_lUnit, pos.y() / m_lUnit,
                                                                       pos.z() / m_lUnit);

        auto hit = new edm4eic::CalorimeterHit(rh->getCellID(),
                                            energy,
                                            0,
                                            time,
                                            0,
                                            position,
                                            dimension,
                                            sid,
                                            lid,
                                            local_position);
        hits.push_back(hit);
    }
    return;
}
