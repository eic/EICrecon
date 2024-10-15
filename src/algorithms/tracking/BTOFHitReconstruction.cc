// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#include "BTOFHitReconstruction.h"

#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4eic/Cov3f.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <spdlog/common.h>
#include <stddef.h>
#include <iterator>
#include <utility>
#include <vector>

#include "TMatrixT.h"

namespace eicrecon {

void BTOFHitReconstruction::init(const dd4hep::rec::CellIDPositionConverter* converter, 
		                 const dd4hep::Detector* detector,
				 std::shared_ptr<spdlog::logger>& logger) {

    m_log = logger;

    m_converter = converter;
    m_detector = detector;
}

dd4hep::rec::CellID BTOFHitReconstruction::getDetInfos(const dd4hep::rec::CellID& id) {
    // retrieve segmentation class if that hasn't been done
    if(! m_decoder) {
        const auto det = m_converter -> findContext(id) -> element;
        auto readout = m_converter -> findReadout(det);
        auto seg = readout.segmentation();
        m_decoder = seg.decoder();
    }

    // CellID for BarrelTOF is composed of 6 parts
    // system, layer, module, sensor, x, y
    // If we fix x and y to zero, what remains will be the detector information only
    auto id_return = id;
    m_decoder -> set(id_return, "x", 0);
    m_decoder -> set(id_return, "y", 0);
    return id_return;
}

std::unique_ptr<edm4eic::TrackerHitCollection> BTOFHitReconstruction::process(const edm4eic::RawTrackerHitCollection& TDCADC_hits) {
    using dd4hep::mm;

    auto rec_hits { std::make_unique<edm4eic::TrackerHitCollection>() };

    // collection of ADC values from all sensors
    std::unordered_map<dd4hep::rec::CellID, 
	               std::vector<HitInfo>> hitsBySensors;

    for (const auto& TDCADC_hit : TDCADC_hits) {

        auto id = TDCADC_hit.getCellID();

        // Get position and dimension
        auto pos = m_converter->position(id);
	// Get sensors info
	auto detID = this -> getDetInfos(id);
	hitsBySensors[detID].emplace_back(pos.x(), pos.y(), pos.z(), int(TDCADC_hit.getCharge()), int(TDCADC_hit.getTimeStamp()), id);

    }

    auto geoManager = m_detector -> world().volume() -> GetGeoManager();

    // loop through each sensors for Hit information
    TMatrixT<double> varLocal(3, 3);
    for(int i = 0; i < 3; ++i)
	    for(int j = 0; j < 3; ++j)
		    varLocal[i][j] = 0;

    for (const auto& sensor : hitsBySensors) {
	// INSERT clustering algorithm for each sensors here
	// Right now I just perform a simple average over all hits in a sensors
	// Will be problematic near the edges, but it's just an illustration
        double ave_x = 0, ave_y = 0, ave_z = 0;
	double tot_charge = 0;
	const auto& hits = sensor.second;
	// find cellID for the cell with maximum ADC value within a sensor
	// I don't know why you need cellID for reconstructed hits, but we'll do it anyway
	auto id = hits[0].id;
	auto curr_adc = hits[0].adc;
	auto first_tdc = hits[0].tdc;
	for(const auto& hit : hits) {
            // weigh all hits by ADC value
            ave_x += hit.adc*hit.x;
	    ave_y += hit.adc*hit.y;
	    ave_z += hit.adc*hit.z;

	    tot_charge += hit.adc;
	    if(hit.adc > curr_adc) {
	        curr_adc = hit.adc;
		id = hit.id;
	    }
	    first_tdc = std::min(first_tdc, hit.tdc);
	}

	ave_x /= tot_charge;
	ave_y /= tot_charge;
	ave_z /= tot_charge;

	auto cellSize = m_converter -> cellDimensions(id);

	// get rotation matrix
	auto node = geoManager -> FindNode(hits[0].x, hits[0].y, hits[0].z);
	auto currMatrix = geoManager -> GetCurrentMatrix();
	auto rotMatrixElements = currMatrix -> GetRotationMatrix();

	// rotMatrix transforms local coordinates to global coordinates
	// see line 342 of https://root.cern.ch/doc/master/TGeoMatrix_8cxx_source.html#l00342
	TMatrixT<double> rot(3, 3, rotMatrixElements);
	TMatrixT<double> rotT(3, 3);
	rotT.Transpose(rot);

	varLocal[0][0] = cellSize[0]*cellSize[0] / mm / mm / 12.; // final division by 12 because I assumed uniform distribution
	varLocal[1][1] = cellSize[1]*cellSize[1] / mm / mm / 12.; // Std. dev of uniformation = width/sqrt(12)
	varLocal[2][2] = 0;

	// transform variance. see https://robotics.stackexchange.com/questions/2556/how-to-rotate-covariance
	auto varGlobal = rot*varLocal*rotT;


	// adc to charge
	float charge = tot_charge * m_cfg.c_slope + m_cfg.c_intercept;
	// TDC to time
	float time = first_tdc * m_cfg.t_slope + m_cfg.t_intercept;
        // >oO trace
        rec_hits->create(
	    id,
            edm4hep::Vector3f{static_cast<float>(ave_x / mm), 
	                      static_cast<float>(ave_y / mm), 
			      static_cast<float>(ave_z / mm)}, // mm
            edm4eic::CovDiag3f{varGlobal[0][0], varGlobal[1][1], varGlobal[2][2]}, // should be the covariance of position
            time, // ns
            0.0F,                            // covariance of time
            charge,   // total ADC sum
            0.0F);                                       // Error on the energy

    }

    return std::move(rec_hits);
}

} // namespace eicrecon
