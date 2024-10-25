// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#include "TrackerHitReconstruction.h"

#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <spdlog/common.h>
#include <stddef.h>
#include <iterator>
#include <utility>
#include <vector>
#include <random>
#include <bitset> // For displaying bits (optional)

namespace eicrecon {

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

void TrackerHitReconstruction::init(const dd4hep::rec::CellIDPositionConverter* converter, std::shared_ptr<spdlog::logger>& logger) {

    m_log = logger;

    m_converter = converter;
}

std::unique_ptr<edm4eic::TrackerHitCollection> TrackerHitReconstruction::process(const edm4eic::RawTrackerHitCollection& raw_hits) {
    using dd4hep::mm;

    auto rec_hits { std::make_unique<edm4eic::TrackerHitCollection>() };


    // add hits from DD4hep
    for (const auto& raw_hit : raw_hits) {

        auto id = raw_hit.getCellID();

        // Get position and dimension
        auto pos = m_converter->position(id);
        auto dim = m_converter->cellDimensions(id);

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
 #if EDM4EIC_VERSION_MAJOR >= 7
       auto rec_hit =
 #endif
       rec_hits->create(
            raw_hit.getCellID(), // Raw DD4hep cell ID
            edm4hep::Vector3f{static_cast<float>(pos.x() / mm), static_cast<float>(pos.y() / mm), static_cast<float>(pos.z() / mm)}, // mm
            edm4eic::CovDiag3f{get_variance(dim[0] / mm), get_variance(dim[1] / mm), // variance (see note above)
            std::size(dim) > 2 ? get_variance(dim[2] / mm) : 0.},
                static_cast<float>((double)(raw_hit.getTimeStamp()) / 1000.0), // ns
            m_cfg.timeResolution,                            // in ns
            static_cast<float>(raw_hit.getCharge() / 1.0e6),   // Collected energy (GeV)
            0.0F);                                       // Error on the energy
#if EDM4EIC_VERSION_MAJOR >= 7
        rec_hit.setRawHit(raw_hit);
#endif

    }

    //------------------------------------------------
    // Example code of adding random noise hits
    // Shujie Li, 08.2024
    // use the first raw hits to obtain the volume ID of the detector system
    // this also make sure the hit container has at least one hit
    int num_hits=10; // total number of noise hits in this detector volume. Should move this to the config file
    std::vector<uint64_t> noise_ids;

    // generate valid random cell id
    for (const auto& hit0 : raw_hits) {
        uint64_t id0,vol_id;        
        id0 = hit0.getCellID();
        // vol ID is the last 8 bits in Si tracker. Need to make it more flexible
        // may want to predefine the layer/module ID as well to speed up the radom ID generation
        vol_id = id0 & 0xFF; 
        // std::cout<<"::: "<<raw_hits.size()<<"/"<<id0<<"=="<<std::bitset<8>(id0)<<"  ::"<<vol_id<<std::endl;

        // Setup random number generator
        std::random_device rd; // Obtain a random number from hardware
        std::mt19937_64 eng(rd()); // Seed the generator
        std::uniform_int_distribution<uint64_t> distr; // Define the range for 64-bit integers
        dd4hep::Position pos;
        int nn=0;
        int i=0;
        while (i < num_hits) {
            uint64_t randomNum = distr(eng); // Generate a random 64-bit number
            randomNum = (randomNum & ~uint64_t(0xFF)) | vol_id; // Clear the last 8 bits and set them to 'vol ID'
            try {
                pos = m_converter->position(randomNum);
                // std::cout<<" ::: converter position "<<pos.x()<<std::endl;
                noise_ids.push_back(randomNum);
                i++;
            } catch(std::exception &e) {
                // std::cout<<"::: cell ID error caught"<<std::endl;
                nn++;
            }
            std::cout <<std::bitset<8>(vol_id) <<":"<< i<<"/"<<nn<<":::"<<randomNum<<"   ";
        }
        break;
    }

    // generate noise hits from ids. 
       for (auto id : noise_ids) {
        // Get position and dimension
        auto pos = m_converter->position(id);
        auto dim = m_converter->cellDimensions(id);
        // >oO trace
        if(m_log->level() == spdlog::level::trace) {
            m_log->trace("Noise hits inserted: position x={:.2f} y={:.2f} z={:.2f} [mm]: ", pos.x()/ mm, pos.y()/ mm, pos.z()/ mm);
            m_log->trace("dimension size: {}", dim.size());
            for (size_t j = 0; j < std::size(dim); ++j) {
                m_log->trace(" - dimension {:<5} size: {:.2}",  j, dim[j]);
            }
        }
 #if EDM4EIC_VERSION_MAJOR >= 7
       auto rec_hit =
 #endif
       rec_hits->create(
            id, // Raw DD4hep cell ID
            edm4hep::Vector3f{static_cast<float>(pos.x() / mm), static_cast<float>(pos.y() / mm), static_cast<float>(pos.z() / mm)}, // mm
            edm4eic::CovDiag3f{get_variance(dim[0] / mm), get_variance(dim[1] / mm), // variance (see note above)
            std::size(dim) > 2 ? get_variance(dim[2] / mm) : 0.},
                static_cast<float>(0), // ns
            m_cfg.timeResolution,                            // in ns
            static_cast<float>(0),   // Collected energy (GeV)
            0.0F);                                       // Error on the energy

    }
    return std::move(rec_hits);
}

} // namespace eicrecon
