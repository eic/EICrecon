// Created by Tyler Kutz
// Subject to the terms in the LICENSE file found in the top-level directory.

#include <JANA/JEvent.h>
#include "TrackPropagation_factory.h"
#include "algorithms/tracking/JugTrack/TrackingResultTrajectory.hpp"
#include "services/geometry/acts/ACTSGeo_service.h"

#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>

#include <extensions/spdlog/SpdlogExtensions.h>
#include <services/log/Log_service.h>
#include <spdlog/fmt/ostr.h>

#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegment.h>

void eicrecon::TrackPropagation_factory::Init() {

	auto app = GetApplication();

	// SpdlogMixin logger initialization, sets m_log
	InitLogger(app, GetTag());
	
	m_input_tag = "CentralCKFTrajectories";

	auto acts_service = GetApplication()->GetService<ACTSGeo_service>();
	m_track_propagation_algo.init(acts_service->actsGeoProvider(), logger());

        m_geoSvc = app->template GetService<JDD4hep_service>();

	SetPropagationSurfaces();

}

void eicrecon::TrackPropagation_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
	JFactoryT::ChangeRun(event);
}

void eicrecon::TrackPropagation_factory::Process(const std::shared_ptr<const JEvent> &event) {

	auto trajectories = event->Get<eicrecon::TrackingResultTrajectory>(m_input_tag);

	edm4eic::TrackSegmentCollection propagated_tracks;

	for(auto traj: trajectories) {
		edm4eic::MutableTrackSegment this_propagated_track;	
		for(unsigned short isurf = 0; auto surf: m_target_surface_list) {		  
			try {
				auto prop_point = m_track_propagation_algo.propagate(traj, surf);
				if(!prop_point) continue;
				prop_point->surface = m_target_surface_ID[isurf];
				this_propagated_track.addToPoints(*prop_point);
			} catch (std::exception& e) {
				m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.what());
			}
			++isurf;
		}
		propagated_tracks.push_back(this_propagated_track); 
	}
		
	SetCollection(std::move(propagated_tracks));
}


void eicrecon::TrackPropagation_factory::SetPropagationSurfaces() {

	auto transform = Acts::Transform3::Identity();

	// shift projection surface to average cluster depth
	// shift in z for endcaps, shift in r for barrel
	double ECAL_avgClusterDepth = 50.;	// mm
	double HCAL_avgClusterDepth = 150.; 	// mm

	// extend surfaces by ten percent for tracks close to the edge
	// extend in r for endcaps, extend in z for barrel
	double extend = 1.1;

	// Create propagation surface for EEMC 
	std::shared_ptr<Acts::DiscSurface> m_EEMC_prop_surface;
	const double EEMC_Z    = -(m_geoSvc->detector()->constant<double>("EcalEndcapN_zmin") / dd4hep::mm) - ECAL_avgClusterDepth;
	const double EEMC_MinR = 0.0;    
	const double EEMC_MaxR = (m_geoSvc->detector()->constant<double>("EcalEndcapN_structure_Oring_min") / dd4hep::mm) * extend;
	auto EEMC_Bounds       = std::make_shared<Acts::RadialBounds>(EEMC_MinR, EEMC_MaxR);
	auto EEMC_Trf          = transform * Acts::Translation3(Acts::Vector3(0, 0, EEMC_Z));
	m_EEMC_prop_surface    = Acts::Surface::makeShared<Acts::DiscSurface>(EEMC_Trf, EEMC_Bounds);
	m_target_surface_list.push_back(m_EEMC_prop_surface);
	m_target_surface_ID.push_back(m_geoSvc->detector()->constant<int32_t>("ECalEndcapN_ID"));
	
	// Create propagation surface for FEMC
	std::shared_ptr<Acts::DiscSurface> m_FEMC_prop_surface;
	const double FEMC_Z    = (m_geoSvc->detector()->constant<double>("EcalEndcapP_zmin") / dd4hep::mm) + ECAL_avgClusterDepth;
	const double FEMC_MinR   = 0.0;      
	const double FEMC_MaxR   = (m_geoSvc->detector()->constant<double>("EcalEndcapP_rmax") / dd4hep::mm) * extend;
	auto FEMC_Bounds       = std::make_shared<Acts::RadialBounds>(FEMC_MinR, FEMC_MaxR);
	auto FEMC_Trf          = transform * Acts::Translation3(Acts::Vector3(0, 0, FEMC_Z));
	m_FEMC_prop_surface    = Acts::Surface::makeShared<Acts::DiscSurface>(FEMC_Trf, FEMC_Bounds);
	m_target_surface_list.push_back(m_FEMC_prop_surface);
	m_target_surface_ID.push_back(m_geoSvc->detector()->constant<int32_t>("ECalEndcapP_ID"));

	// Create propagation surface for BEMC
	std::shared_ptr<Acts::CylinderSurface> m_BEMC_prop_surface;
	const double BEMC_R     = (m_geoSvc->detector()->constant<double>("EcalBarrel_rmin") / dd4hep::mm) + ECAL_avgClusterDepth;
	const double BEMC_halfz = (std::max(m_geoSvc->detector()->constant<double>("EcalBarrelBackward_zmax"), m_geoSvc->detector()->constant<double>("EcalBarrelForward_zmax")) / dd4hep::mm) * extend;
	auto BEMC_Trf         = transform * Acts::Translation3(Acts::Vector3(0, 0, 0));
	m_BEMC_prop_surface = Acts::Surface::makeShared<Acts::CylinderSurface>(BEMC_Trf, BEMC_R, BEMC_halfz);
	m_target_surface_list.push_back(m_BEMC_prop_surface);
	m_target_surface_ID.push_back(m_geoSvc->detector()->constant<int32_t>("ECalBarrel_ID"));

	// Create propagation surface for LFHCAL
	std::shared_ptr<Acts::DiscSurface> m_LFHCAL_prop_surface;
	const double LFHCAL_Z    = (m_geoSvc->detector()->constant<double>("HcalEndcapP_zmin") / dd4hep::mm) + HCAL_avgClusterDepth;
	const double LFHCAL_MinR = 0.0;
	const double LFHCAL_MaxR = (m_geoSvc->detector()->constant<double>("HcalEndcapP_rmax") / dd4hep::mm) * extend;
	auto LFHCAL_Bounds       = std::make_shared<Acts::RadialBounds>(LFHCAL_MinR, LFHCAL_MaxR);
	auto LFHCAL_Trf          = transform * Acts::Translation3(Acts::Vector3(0, 0, LFHCAL_Z));
	m_LFHCAL_prop_surface    = Acts::Surface::makeShared<Acts::DiscSurface>(LFHCAL_Trf, LFHCAL_Bounds);
	m_target_surface_list.push_back(m_LFHCAL_prop_surface);
	m_target_surface_ID.push_back(m_geoSvc->detector()->constant<int32_t>("HCalEndcapP_ID"));

	// Create propagation surface for OHCAL
	std::shared_ptr<Acts::CylinderSurface> m_OHCAL_prop_surface;
	const double OHCAL_R     = (m_geoSvc->detector()->constant<double>("HcalBarrel_rmin") / dd4hep::mm) + HCAL_avgClusterDepth;
	const double OHCAL_halfz = (std::max(m_geoSvc->detector()->constant<double>("HcalBarrelBackward_zmax"), m_geoSvc->detector()->constant<double>("HcalBarrelForward_zmax")) / dd4hep::mm) * extend;
	auto OHCAL_Trf           = transform * Acts::Translation3(Acts::Vector3(0, 0, 0));
	m_OHCAL_prop_surface = Acts::Surface::makeShared<Acts::CylinderSurface>(OHCAL_Trf, OHCAL_R, OHCAL_halfz);
	m_target_surface_list.push_back(m_OHCAL_prop_surface);
	m_target_surface_ID.push_back(m_geoSvc->detector()->constant<int32_t>("HCalBarrel_ID"));

	// Create propagation surface for EHCAL
	std::shared_ptr<Acts::DiscSurface> m_EHCAL_prop_surface;
	const double EHCAL_Z    = -(m_geoSvc->detector()->constant<double>("HcalEndcapN_zmin") / dd4hep::mm) - HCAL_avgClusterDepth;
	const double EHCAL_MinR = 0.0;    
	const double EHCAL_MaxR = (m_geoSvc->detector()->constant<double>("HcalEndcapN_rmax") / dd4hep::mm) * extend;
	auto EHCAL_Bounds       = std::make_shared<Acts::RadialBounds>(EHCAL_MinR, EHCAL_MaxR);
	auto EHCAL_Trf          = transform * Acts::Translation3(Acts::Vector3(0, 0, EHCAL_Z));
	m_EHCAL_prop_surface  = Acts::Surface::makeShared<Acts::DiscSurface>(EHCAL_Trf, EHCAL_Bounds);
	m_target_surface_list.push_back(m_EHCAL_prop_surface);
	m_target_surface_ID.push_back(m_geoSvc->detector()->constant<int32_t>("HCalEndcapN_ID"));

	/*	
	std::cout << std::endl;
	std::cout << "Setting track propagation surfaces to:" << std::endl;
	std::cout << std::endl;

	std::cout << "EEMC_Z    = " << EEMC_Z << std::endl;
	std::cout << "EEMC_MinR = " << EEMC_MinR << std::endl;
	std::cout << "EEMC_MaxR = " << EEMC_MaxR << std::endl;
	
	std::cout << std::endl;

	std::cout << "FEMC_Z    = " << FEMC_Z << std::endl;
	std::cout << "FEMC_MinR = " << FEMC_MinR << std::endl;
	std::cout << "FEMC_MaxR = " << FEMC_MaxR << std::endl;

	std::cout << std::endl;

	std::cout << "BEMC_R    = " << BEMC_R << std::endl;
	std::cout << "BEMC_Z    = " << BEMC_halfz << std::endl;

	std::cout << std::endl;

	std::cout << "LFHCAL_Z    = " << LFHCAL_Z << std::endl;
	std::cout << "LFHCAL_MinR = " << LFHCAL_MinR << std::endl;
	std::cout << "LFHCAL_MaxR = " << LFHCAL_MaxR << std::endl;

	std::cout << std::endl;

	std::cout << "EHCAL_Z     = " << EHCAL_Z << std::endl;
	std::cout << "EHCAL_MinR  = " << EHCAL_MinR << std::endl;
	std::cout << "EHCAL_MaxR  = " << EHCAL_MaxR << std::endl;

	std::cout << std::endl;

	std::cout << "OHCAL_R     = " << OHCAL_R << std::endl;
	std::cout << "OHCAL_Z     = " << OHCAL_halfz << std::endl;

	std::cout << std::endl;
	std::cout << std::endl;
	*/

}




