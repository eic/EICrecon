// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <edm4eic/vector_utils.h>

#include "FarDetectorLinearProjection.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <iterator>
#include <algorithm>
#include <map>



namespace eicrecon {


    void FarDetectorLinearProjection::init(std::shared_ptr<spdlog::logger>& logger) {

      m_log      = logger;

      // plane position
      m_plane_position << m_cfg.plane_position[0], m_cfg.plane_position[1], m_cfg.plane_position[2];
      m_directions.block<3,1>(0,0) << m_cfg.plane_a[0], m_cfg.plane_a[1], m_cfg.plane_a[2];
      m_directions.block<3,1>(0,1) << m_cfg.plane_b[0], m_cfg.plane_b[1], m_cfg.plane_b[2];

    }

    std::unique_ptr<edm4eic::TrackParametersCollection> FarDetectorLinearProjection::produce(const edm4eic::TrackSegmentCollection &inputSegments) {

      auto outputTracks = std::make_unique<edm4eic::TrackParametersCollection>();

      for( auto segment: inputSegments ) {

	auto inputPoint = segment.getPoints()[0];

	Eigen::Vector3d point_position(inputPoint.position.x,inputPoint.position.y,inputPoint.position.z);
	Eigen::Vector3d positionDiff = point_position - m_plane_position;
	m_directions.block<3,1>(0,2) << inputPoint.momentum.x,inputPoint.momentum.y,inputPoint.momentum.z;

	auto projectedPoint = m_directions.inverse()*positionDiff;

	// Create track parameters edm4eic structure
	// TODO - populate more of the fields correctly
	std::int32_t type = 0;
	// Plane Point
	edm4hep::Vector2f loc(projectedPoint[0],projectedPoint[1]); //Temp unit transform
	// Point Error
	edm4eic::Cov2f locError;
	float theta = inputPoint.theta;//edm4eic::anglePolar(outVec);
	float phi   = inputPoint.phi  ;//edm4eic::angleAzimuthal(outVec);
	float qOverP;
	edm4eic::Cov3f momentumError;
	float time      = 0;
	float timeError = 0;
	float charge    = -1;
	
	outputTracks->create(type,loc,locError,theta,phi,qOverP,momentumError,time,timeError,charge);
      }

      return outputTracks;

    }


}
