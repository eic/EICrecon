// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Chao, Whitney Armstrong, Wouter Deconinck, Jihee Kim, David Lawrence

// Reconstruct digitized outputs of ImagingCalorimeter
// It converts digitized ADC/TDC values to energy/time, and looks for geometrical information of the
// readout pixels Author: Chao Peng Date: 06/02/2021

#include <algorithm>
#include <bitset>

#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/Surface.h"
#include "DDRec/SurfaceManager.h"

// Event Model related classes
#include "edm4eic/CalorimeterHit.h"
#include "edm4eic/RawCalorimeterHit.h"

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <spdlog/spdlog.h>

/** Imaging calorimeter pixel hit reconstruction.
 *
 * Reconstruct digitized outputs of ImagingCalorimeter
 * It converts digitized ADC/TDC values to energy/time, and looks for geometrical information of the
 *
 * \ingroup reco
 */
class ImagingPixelReco {
protected:
    // geometry service
//    std::string m_geoSvcName; // ; // {this, "geoServiceName", "GeoSvc"};
    std::string m_readout; // {this, "readoutClass", ""};
    std::string m_layerField; // {this, "layerField", "layer"};
    std::string m_sectorField; // {this, "sectorField", "sector"};
    // length unit (from dd4hep geometry service)
    double m_lUnit; // {this, "lengthUnit", dd4hep::mm};
    // digitization parameters
    unsigned int m_capADC; // {this, "capacitysize_tADC", 8096};
    unsigned int m_pedMeanADC; // {this, "pedestalMean", 400};
    double m_dyRangeADC; // {this, "dynamicRangeADC", 100 * dd4hep::MeV};
    double m_pedSigmaADC; // {this, "pedestalSigma", 3.2};
    double m_thresholdFactor; // {this, "thresholdFactor", 3.0};
    // Calibration!
    double m_sampFrac; // {this, "samplingFraction", 1.0};

    // hits containers
    std::vector<const edm4hep::RawCalorimeterHit*> m_inputHits;
    std::vector<edm4eic::CalorimeterHit *> m_outputHits;

    // Pointer to the geometry service
    std::shared_ptr<JDD4hep_service> m_geoSvc;

    // logger
    std::shared_ptr<spdlog::logger> m_log;

    // visit readout fields
    dd4hep::BitFieldCoder *id_dec;
    dd4hep::long64 sector_idx{0}, layer_idx{0};

public:
    ImagingPixelReco() = default;

    void initialize() {
        if (!m_geoSvc) {
            m_log->error("Unable to locate Geometry Service. \nMake sure you have GeoSvc and SimSvc in the right order in the configuration.");
            return;
        }

        if (m_readout.empty()) {
            m_log->error("readoutClass is not provided, it is needed to know the fields in readout ids");
            return;
        }

        try {
            id_dec = m_geoSvc->detector()->readout(m_readout).idSpec().decoder();
            sector_idx = id_dec->index(m_sectorField);
            layer_idx = id_dec->index(m_layerField);
        } catch (...) {
            m_log->warn(fmt::format("Failed to load ID decoder for {}", m_readout));
            return;
        }
    }

    void execute() {
        // input collections
        const auto &rawhits = m_inputHits;
        // Create output collections
        auto &hits = m_outputHits;

        // energy time reconstruction
        for (const auto &rh: rawhits) {

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wsign-conversion"

            // did not pass the threshold
            if (rh->getAmplitude() < m_pedMeanADC + m_thresholdFactor * m_pedSigmaADC) {
                continue;
            }
            const double energy =
                    (((signed) rh->getAmplitude() - (signed) m_pedMeanADC)) / (double) m_capADC * m_dyRangeADC /
                    m_sampFrac; // convert ADC -> energy
            const double time = rh->getTimeStamp() * 1.e-6;                                       // dd4hep::ns

#pragma GCC diagnostic pop

            try {
                const auto id = rh->getCellID();
                // @TODO remove
                const int lid = (int) id_dec->get(id, layer_idx);
                const int sid = (int) id_dec->get(id, sector_idx);

                // global positions
                const auto gpos = m_geoSvc->cellIDPositionConverter()->position(id);
                // local positions
                const auto volman = m_geoSvc->detector()->volumeManager();
                // TODO remove
                const auto alignment = volman.lookupDetElement(id).nominal();
                const auto pos = alignment.worldToLocal(dd4hep::Position(gpos.x(), gpos.y(), gpos.z()));


                // create const vectors for passing to hit initializer list
                const decltype(edm4eic::CalorimeterHitData::position) position(
                        gpos.x() / m_lUnit, gpos.y() / m_lUnit, gpos.z() / m_lUnit
                );
                const decltype(edm4eic::CalorimeterHitData::local) local(
                        pos.x() / m_lUnit, pos.y() / m_lUnit, pos.z() / m_lUnit
                );

                hits.push_back(new edm4eic::CalorimeterHit{id,                         // cellID
                                                           static_cast<float>(energy), // energy
                                                           0,                          // energyError
                                                           static_cast<float>(time),   // time
                                                           0,                          // timeError TODO
                                                           position,                   // global pos
                                                           {0, 0, 0}, // @TODO: add dimension
                                                           sid, lid,
                                                           local});                    // local pos
            }catch(std::exception &e){
                m_log->error("ImagingPixelReco::execute {}", e.what());
            }
        }
    }
};
