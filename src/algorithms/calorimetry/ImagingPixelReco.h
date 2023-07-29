// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Chao, Whitney Armstrong, Wouter Deconinck, Jihee Kim, David Lawrence

// Reconstruct digitized outputs of ImagingCalorimeter
// It converts digitized ADC/TDC values to energy/time, and looks for geometrical information of the
// readout pixels Author: Chao Peng Date: 06/02/2021

#pragma once

#include <algorithm>
#include <bitset>
#include <memory>

#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/Surface.h>
#include <DDRec/SurfaceManager.h>

// Event Model related classes
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/RawCalorimeterHit.h>

#include "services/geometry/dd4hep/JDD4hep_service.h"
#include <spdlog/spdlog.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "ImagingPixelRecoConfig.h"

namespace eicrecon {

/** Imaging calorimeter pixel hit reconstruction.
 *
 * Reconstruct digitized outputs of ImagingCalorimeter
 * It converts digitized ADC/TDC values to energy/time, and looks for geometrical information of the
 *
 * \ingroup reco
 */
  class ImagingPixelReco : public WithPodConfig<ImagingPixelRecoConfig> {

  private:
    const dd4hep::Detector* m_detector;
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_converter;
    std::shared_ptr<spdlog::logger> m_log;

  protected:

    // visit readout fields
    dd4hep::BitFieldCoder *id_dec;
    size_t sector_idx{0}, layer_idx{0};

  public:

    void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger) {
        m_detector = detector;
        m_log = logger;

        if (m_detector == nullptr) {
            m_log->error("Unable to locate Geometry Service. \nMake sure you have GeoSvc and SimSvc in the right order in the configuration.");
            throw std::runtime_error("Unable to locate Geometry Service. \nMake sure you have GeoSvc and SimSvc in the right order in the configuration.");
        }

        m_converter = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(const_cast<dd4hep::Detector&>(*detector));

        if (m_cfg.readout.empty()) {
            m_log->error("readoutClass is not provided, it is needed to know the fields in readout ids");
            throw std::runtime_error("readoutClass is not provided, it is needed to know the fields in readout ids");
        }

        try {
            id_dec = m_detector->readout(m_cfg.readout).idSpec().decoder();
            sector_idx = id_dec->index(m_cfg.sectorField);
            layer_idx = id_dec->index(m_cfg.layerField);
        } catch (...) {
            m_log->warn("Failed to load ID decoder for {}", m_cfg.readout);
            throw;
        }
    }

    std::unique_ptr<edm4eic::CalorimeterHitCollection> process(const edm4hep::RawCalorimeterHitCollection& rawhits) {

        auto recohits = std::make_unique<edm4eic::CalorimeterHitCollection>();

        // energy time reconstruction
        for (const auto &rh: rawhits) {

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wsign-conversion"

            // did not pass the threshold
            if (rh.getAmplitude() < m_cfg.pedMeanADC + m_cfg.thresholdFactor * m_cfg.pedSigmaADC) {
                continue;
            }
            const double energy =
                    (((signed) rh.getAmplitude() - (signed) m_cfg.pedMeanADC)) / (double) m_cfg.capADC * m_cfg.dyRangeADC /
                    m_cfg.sampFrac; // convert ADC -> energy
            const double time = rh.getTimeStamp() * 1.e-6; // dd4hep::ns

#pragma GCC diagnostic pop

            try {
                const auto id = rh.getCellID();
                // @TODO remove
                const int lid = (int) id_dec->get(id, layer_idx);
                const int sid = (int) id_dec->get(id, sector_idx);

                // global positions
                const auto gpos = m_converter->position(id);
                // local positions
                const auto volman = m_detector->volumeManager();
                // TODO remove
                const auto alignment = volman.lookupDetElement(id).nominal();
                const auto pos = alignment.worldToLocal(dd4hep::Position(gpos.x(), gpos.y(), gpos.z()));


                // create const vectors for passing to hit initializer list
                const decltype(edm4eic::CalorimeterHitData::position) position(
                        gpos.x(), gpos.y(), gpos.z()
                );
                const decltype(edm4eic::CalorimeterHitData::local) local(
                        pos.x(), pos.y(), pos.z()
                );

                recohits->create(id,                         // cellID
                            static_cast<float>(energy), // energy
                            0,                          // energyError
                            static_cast<float>(time),   // time
                            0,                          // timeError TODO
                            position,                   // global pos
                            edm4hep::Vector3f({0, 0, 0}), // @TODO: add dimension
                            sid, lid,
                            local);                    // local pos
            } catch(std::exception &e) {
                m_log->error("ImagingPixelReco::execute {}", e.what());
            }
        }
        return recohits;
    }
};

} // namespace eicrecon
