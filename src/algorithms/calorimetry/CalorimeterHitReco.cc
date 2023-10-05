// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Wouter Deconinck, Chao, Whitney Armstrong

// Reconstruct digitized outputs, paired with Jug::Digi::CalorimeterHitDigi
// Author: Chao Peng
// Date: 06/14/2021

#include "CalorimeterHitReco.h"

#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
#include <cctype>

using namespace dd4hep;

namespace eicrecon {

void CalorimeterHitReco::init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger) {
    m_detector = detector;
    m_converter = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(const_cast<dd4hep::Detector&>(*detector));
    m_log = logger;

    // threshold for firing
    thresholdADC = m_cfg.thresholdFactor * m_cfg.pedSigmaADC + m_cfg.thresholdValue;
    // TDC channels to timing conversion
    stepTDC = dd4hep::ns / m_cfg.resolutionTDC;

    // do not get the layer/sector ID if no readout class provided
    if (m_cfg.readout.empty()) {
        return;
    }

    // First, try and get the IDDescriptor. This will throw an exception if it fails.
    IDDescriptor id_spec;
    try {
        id_spec = m_detector->readout(m_cfg.readout).idSpec();
    } catch(...) {
        m_log->warn("Failed to get idSpec for {}", m_cfg.readout);
        return;
    }
    // Next, try and get the readout fields. This will throw a different exception.
    try {
        id_dec = id_spec.decoder();
        if (!m_cfg.sectorField.empty()) {
            sector_idx = id_dec->index(m_cfg.sectorField);
            m_log->debug("Find sector field {}, index = {}", m_cfg.sectorField, sector_idx);
        }
        if (!m_cfg.layerField.empty()) {
            layer_idx = id_dec->index(m_cfg.layerField);
            m_log->debug("Find layer field {}, index = {}", m_cfg.layerField, sector_idx);
        }
        if (!m_cfg.maskPosFields.empty()) {
            size_t tmp_mask = 0;
            for (auto &field : m_cfg.maskPosFields) {
                tmp_mask |= id_spec.field(field)->mask();
            }
            // assign this mask if all fields succeed
            gpos_mask = tmp_mask;
        }
    } catch (...) {
        if (!id_dec) {
            m_log->warn("Failed to load ID decoder for {}", m_cfg.readout);
            std::stringstream readouts;
            for (auto r: m_detector->readouts()) readouts << "\"" << r.first << "\", ";
            m_log->warn("Available readouts: {}", readouts.str() );
        } else {
            m_log->warn("Failed to find field index for {}.", m_cfg.readout);
            if (!m_cfg.sectorField.empty()) { m_log->warn(" -- looking for sector field \"{}\".", m_cfg.sectorField); }
            if (!m_cfg.layerField.empty()) { m_log->warn(" -- looking for layer field  \"{}\".", m_cfg.layerField); }
            if (!m_cfg.maskPosFields.empty()) {
                m_log->warn(" -- looking for masking fields  \"{}\".", fmt::join(m_cfg.maskPosFields, ", "));
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
    if (!m_cfg.localDetElement.empty()) {
        try {
            local = m_detector->detector(m_cfg.localDetElement);
            m_log->info("local coordinate system from DetElement {}", m_cfg.localDetElement);
        } catch (...) {
            m_log->error("failed to load local coordinate system from DetElement {}", m_cfg.localDetElement);
            return;
        }
    } else {
        std::vector <std::pair<std::string, int >> fields;
        for (auto f : m_cfg.localDetFields) {
            fields.emplace_back(f, 0);
        }
        local_mask = id_spec.get_mask(fields);
        // use all fields if nothing provided
        if (fields.empty()) {
            local_mask = ~static_cast<decltype(local_mask)>(0);
        }
    }

    return;
}


std::unique_ptr<edm4eic::CalorimeterHitCollection> CalorimeterHitReco::process(const edm4hep::RawCalorimeterHitCollection &rawhits) {
    auto recohits = std::make_unique<edm4eic::CalorimeterHitCollection>();

    // For some detectors, the cellID in the raw hits may be broken
    // (currently this is the HcalBarrel). In this case, dd4hep
    // prints an error message and throws an exception. We catch
    // the exception and handle it, but the screen gets flooded
    // with these messages. Keep a count of these and if a max
    // number is encountered disable this algorithm. A useful message
    // indicating what is going on is printed below where the
    // error is detector.
    if (NcellIDerrors >= MaxCellIDerrors) return std::move(recohits);

    for (const auto &rh: rawhits) {

        //did not pass the zero-suppresion threshold
        const auto cellID = rh.getCellID();
        if (rh.getAmplitude() < m_cfg.pedMeanADC + thresholdADC) {
            continue;
        }

        // get layer and sector ID
        const int lid =
                id_dec != nullptr && !m_cfg.layerField.empty() ? static_cast<int>(id_dec->get(cellID, layer_idx)) : -1;
        const int sid =
                id_dec != nullptr && !m_cfg.sectorField.empty() ? static_cast<int>(id_dec->get(cellID, sector_idx)) : -1;

        // determine sampling fraction
        float sampFrac = m_cfg.sampFrac;
        if (! m_cfg.sampFracLayer.empty()) {
            if (0 <= lid && lid < m_cfg.sampFracLayer.size()) {
                sampFrac = m_cfg.sampFracLayer[lid];
            } else {
                throw std::runtime_error(fmt::format("CalorimeterHitReco: layer-specific sampling fraction undefined for index {}", lid));
            }
        }

        // convert ADC to energy
        float energy = (((signed) rh.getAmplitude() - (signed) m_cfg.pedMeanADC)) / static_cast<float>(m_cfg.capADC) * m_cfg.dyRangeADC /
                m_cfg.sampFrac;

        const float time = rh.getTimeStamp() / stepTDC;
        m_log->trace("cellID {}, \t energy: {},  TDC: {}, time: ", cellID, energy, rh.getTimeStamp(), time);

        dd4hep::Position gpos;
        try {
            // global positions
            gpos = m_converter->position(cellID);

            // masked position (look for a mother volume)
            if (gpos_mask != 0) {
                auto mpos = m_converter->position(cellID & ~gpos_mask);
                // replace corresponding coords
                for (const char &c : m_cfg.maskPos) {
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
            if (m_cfg.localDetElement.empty()) {
                auto volman = m_detector->volumeManager();
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
        std::vector<double> cdim;
        // get segmentation dimensions
        auto segmentation_type = m_converter->findReadout(local).segmentation().type();
        if (segmentation_type == "CartesianGridXY") {
            auto cell_dim = m_converter->cellDimensions(cellID);
            cdim.resize(3);
            cdim[0] = cell_dim[0];
            cdim[1] = cell_dim[1];
            m_log->debug("Using segmentation for cell dimensions: {}", fmt::join(cdim, ", "));
        } else {
            if ((segmentation_type != "NoSegmentation") && (!warned_unsupported_segmentation)) {
                m_log->warn("Unsupported segmentation type \"{}\"", segmentation_type);
                warned_unsupported_segmentation = true;
            }

            // Using bounding box instead of actual solid so the dimensions are always in dim_x, dim_y, dim_z
            cdim = m_converter->findContext(cellID)->volumePlacement().volume().boundingBox().dimensions();
            std::transform(cdim.begin(), cdim.end(), cdim.begin(),
                           std::bind(std::multiplies<double>(), std::placeholders::_1, 2));
            m_log->debug("Using bounding box for cell dimensions: {}", fmt::join(cdim, ", "));
        }

        //create constant vectors for passing to hit initializer list
        //FIXME: needs to come from the geometry service/converter
        const decltype(edm4eic::CalorimeterHitData::position) position(gpos.x() / dd4hep::mm, gpos.y() / dd4hep::mm,
                                                                    gpos.z() / dd4hep::mm);
        const decltype(edm4eic::CalorimeterHitData::dimension) dimension(cdim.at(0) / dd4hep::mm, cdim.at(1) / dd4hep::mm,
                                                                      cdim.at(2) / dd4hep::mm);
        const decltype(edm4eic::CalorimeterHitData::local) local_position(pos.x() / dd4hep::mm, pos.y() / dd4hep::mm,
                                                                       pos.z() / dd4hep::mm);

        recohits->create(
            rh.getCellID(),
            energy,
            0,
            time,
            0,
            position,
            dimension,
            sid,
            lid,
            local_position);
    }

    return recohits;
}

} // namespace eicrecon
