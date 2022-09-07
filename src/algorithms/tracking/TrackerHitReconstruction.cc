// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackerHitReconstruction.h"

namespace {
    inline double get_resolution(const double pixel_size) {
        constexpr const double sqrt_12 = 3.4641016151;
        return pixel_size / sqrt_12;
    }
    inline double get_variance(const double pixel_size) {
        const double res = get_resolution(pixel_size);
        return res * res;
    }
} // namespace

void eicrecon::TrackerHitReconstruction::init(dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger) {

    m_log = logger;

    // Create CellID convertor
    m_cellid_converter = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*detector);
}

eicd::TrackerHit *eicrecon::TrackerHitReconstruction::produce(const eicd::RawTrackerHit *raw_hit) {
    constexpr auto mm = dd4hep::mm;

    auto id = raw_hit->getCellID();

    // Get position and dimension
    auto pos = m_cellid_converter->position(id);
    auto dim = m_cellid_converter->cellDimensions(id);

    // >oO trace
    if(m_log->level() == spdlog::level::trace) {
        m_log->trace("position x={:.2f} y={:.2f} z={:.2f} [mm]: ", pos.x()/ mm, pos.y()/ mm, pos.z()/ mm);
        m_log->trace("dimension size: {}", dim.size());
        for (size_t j = 0; j < std::size(dim); ++j) {
            m_log->trace(" - dimension {:<5} size: {:.2}",  j, dim[j]);
        }
    }

    // Note about variance:
    //    The variance is used to obtain a diagonal covariance matrix.
    //    Note that the covariance matrix is written in DD4hep surface coordinates,
    //    *NOT* global position coordinates. This implies that:
    //      - XY segmentation: xx -> sigma_x, yy-> sigma_y, zz -> 0, tt -> 0
    //      - XZ segmentation: xx -> sigma_x, yy-> sigma_z, zz -> 0, tt -> 0
    //      - XYZ segmentation: xx -> sigma_x, yy-> sigma_y, zz -> sigma_z, tt -> 0
    //    This is properly in line with how we get the local coordinates for the hit
    //    in the TrackerSourceLinker.
    return new eicd::TrackerHit(
            raw_hit->getCellID(), // Raw DD4hep cell ID
         {static_cast<float>(pos.x() / mm), static_cast<float>(pos.y() / mm), static_cast<float>(pos.z() / mm)}, // mm
         {get_variance(dim[0] / mm), get_variance(dim[1] / mm), // variance (see note above)
          std::size(dim) > 2 ? get_variance(dim[2] / mm) : 0.},
            static_cast<float>(raw_hit->getTimeStamp() / 1000), // ns
         m_cfg.time_resolution,                            // in ns
         static_cast<float>(raw_hit->getCharge() / 1.0e6),   // Collected energy (GeV)
         0.0F);                                       // Error on the energy
}


