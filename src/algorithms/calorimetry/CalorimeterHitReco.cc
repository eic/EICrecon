// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Wouter Deconinck, Chao, Whitney Armstrong

// Reconstruct digitized outputs, paired with Jug::Digi::CalorimeterHitDigi
// Author: Chao Peng
// Date: 06/14/2021

#include "CalorimeterHitReco.h"

#include <JANA/JEvent.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
#include <cctype>

using namespace dd4hep;

//this algorithm converted from https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/CalorimeterHitReco.cpp


//------------------------
// AlgorithmInit
//------------------------
void CalorimeterHitReco::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {

    m_log=logger;

    // threshold for firing
    thresholdADC = m_thresholdFactor * m_pedSigmaADC + m_thresholdValue;
    // TDC channels to timing conversion
    stepTDC = dd4hep::ns / m_resolutionTDC;

    // do not get the layer/sector ID if no readout class provided
    if (m_readout.empty()) {
        return;
    }

    // First, try and get the IDDescriptor. This will throw an exception if it fails.
    try{
        auto id_spec = m_geoSvc->detector()->readout(m_readout).idSpec();
    } catch(...) {
        m_log->warn("Failed to get idSpec for {}", m_readout);
        return;
    }

    // Get id_spec again, but here it should always succeed.
    // TODO: This is a bit of a hack so should be cleaned up.
    auto id_spec = m_geoSvc->detector()->readout(m_readout).idSpec();
    try {
        id_dec = id_spec.decoder();
        if (!m_sectorField.empty()) {
            sector_idx = id_dec->index(m_sectorField);
            m_log->info("Find sector field {}, index = {}", m_sectorField, sector_idx);
        }
        if (!m_layerField.empty()) {
            layer_idx = id_dec->index(m_layerField);
            m_log->info("Find layer field {}, index = {}", m_layerField, sector_idx);
        }
        if (!u_maskPosFields.empty()) {
            size_t tmp_mask = 0;
            for (auto &field : u_maskPosFields) {
                tmp_mask |= id_spec.field(field)->mask();
            }
            // assign this mask if all fields succeed
            gpos_mask = tmp_mask;
        }
    } catch (...) {
        if (!id_dec) {
            m_log->warn("Failed to load ID decoder for {}", m_readout);
            std::stringstream readouts;
            for (auto r: m_geoSvc->detector()->readouts()) readouts << "\"" << r.first << "\", ";
            m_log->warn("Available readouts: {}", readouts.str() );
        } else {
            m_log->warn("Failed to find field index for {}.", m_readout);
            if (!m_sectorField.empty()) { m_log->warn(" -- looking for sector field \"{}\".", m_sectorField); }
            if (!m_layerField.empty()) { m_log->warn(" -- looking for layer field  \"{}\".", m_layerField); }
            if (!u_maskPosFields.empty()) {
                m_log->warn(" -- looking for masking fields  \"{}\".", fmt::join(u_maskPosFields, ", "));
            }
            std::stringstream fields;
            for (auto field: id_spec.decoder()->fields()) fields << "\"" << field.name() << "\", ";
            m_log->warn("Available fields: {}", fields.str() );
            m_log->warn("n.b. The local position, sector id and layer id will not be correct for this.");
            m_log->warn("Position masking may not be applied.");
            m_log->warn("however, the position, energy, and time values should still be good.");
        }

        return;
    }


    // local detector name has higher priority
    if (!m_localDetElement.empty()) {
        try {
            local = m_geoSvc->detector()->detector(m_localDetElement);
            m_log->info("local coordinate system from DetElement {}", m_localDetElement);
        } catch (...) {
            m_log->error("failed to load local coordinate system from DetElement {}", m_localDetElement);
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

    // For some detectors, the cellID in the raw hits may be broken
    // (currently this is the HcalBarrel). In this case, dd4hep
    // prints an error message and throws an exception. We catch
    // the exception and handle it, but the screen gets flooded
    // with these messages. Keep a count of these and if a max
    // number is encountered disable this algorithm. A useful message
    // indicating what is going on is printed below where the
    // error is detector.
    auto decoder = m_geoSvc->detector()->readout(m_readout).idSpec().decoder();
    if (NcellIDerrors >= MaxCellIDerrors) return;

    auto converter = m_geoSvc->cellIDPositionConverter();
    for (const auto rh: rawhits) {
//        #pragma GCC diagnostic push
//        #pragma GCC diagnostic error "-Wsign-converstion"

        //did not pass the zero-suppresion threshold
        const auto cellID = rh->getCellID();
        if (rh->getAmplitude() < m_pedMeanADC + thresholdADC) {
            continue;
        }

        // convert ADC to energy
        float energy = (((signed) rh->getAmplitude() - (signed) m_pedMeanADC)) / static_cast<float>(m_capADC) * m_dyRangeADC /
                m_sampFrac;
        if (m_readout == "LFHCALHits" && m_sampFracLayer[0] != 0.){
          energy = (((signed) rh->getAmplitude() - (signed) m_pedMeanADC)) / static_cast<float>(m_capADC) * m_dyRangeADC /
                    m_sampFracLayer[decoder->get(cellID, decoder->index("rlayerz"))]; // use readout layer depth information from decoder
        }

        const float time = rh->getTimeStamp() / stepTDC;
        m_log->trace("cellID {}, \t energy: {},  TDC: {}, time: ", cellID, energy, rh->getTimeStamp(), time);

        const int lid =
                id_dec != nullptr && !m_layerField.empty() ? static_cast<int>(id_dec->get(cellID, layer_idx)) : -1;
        const int sid =
                id_dec != nullptr && !m_sectorField.empty() ? static_cast<int>(id_dec->get(cellID, sector_idx)) : -1;
        dd4hep::Position gpos;
        try {
            // global positions
            gpos = converter->position(cellID);

            // masked position (look for a mother volume)
            if (gpos_mask != 0) {
                auto mpos = converter->position(cellID & ~gpos_mask);
                // replace corresponding coords
                for (const char &c : m_maskPos) {
                    switch (std::tolower(c)) {
                    case 'x':
                        gpos.SetX(mpos.X());
                        break;
                    case 'y':
                        gpos.SetY(mpos.Y());
                        break;
                    case 'z':
                        gpos.SetZ(mpos.Z());
                        break;
                    default:
                        break;
                    }
                }
            }

            // local positions
            if (m_localDetElement.empty()) {
                auto volman = m_geoSvc->detector()->volumeManager();
                local = volman.lookupDetElement(cellID & local_mask);
            }
        } catch (...) {
            // Error looking up cellID. Messages should already have been printed.
            // Also, see comment at top of this method.
            if (++NcellIDerrors >= MaxCellIDerrors) {
                m_log->error("Maximum number of errors reached: {}", MaxCellIDerrors);
                m_log->error("This is likely an issue with the cellID being unknown.");
                m_log->error("Note: local_mask={:X} example cellID={:x}", local_mask, cellID);
                m_log->error("Disabling this algorithm since it requires a valid cellID.");
                m_log->error("(See {}:{})", __FILE__,__LINE__);
            }
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
                                                                      cdim.size() > 2? cdim[2] / m_lUnit: 0);
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
