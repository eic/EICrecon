// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Tyler Kutz

#include "TrackPropagation_factory.h"

#include <JANA/JEvent.h>
#include <algorithms/tracking/JugTrack/TrackingResultTrajectory.hpp>
#include <services/geometry/acts/ACTSGeo_service.h>

#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>

#include <extensions/spdlog/SpdlogExtensions.h>
#include <services/log/Log_service.h>

#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegment.h>

void eicrecon::TrackPropagation_factory::Init() {

    auto app = GetApplication();

    // SpdlogMixin logger initialization, sets m_log
    InitLogger(app, GetTag());

    auto acts_service = GetApplication()->GetService<ACTSGeo_service>();
    m_track_propagation_algo.init(acts_service->actsGeoProvider(), logger());

    m_geoSvc = app->template GetService<JDD4hep_service>();

    SetPropagationSurfaces();

}

void eicrecon::TrackPropagation_factory::Process(const std::shared_ptr<const JEvent> &event) {

    auto trajectories = event->Get<eicrecon::TrackingResultTrajectory>(GetInputTags()[0]);

    edm4eic::TrackSegmentCollection propagated_tracks;

    for(auto traj: trajectories) {
        edm4eic::MutableTrackSegment this_propagated_track;	
        for(size_t isurf = 0; auto surf: m_target_surface_list) {		  
            auto prop_point = m_track_propagation_algo.propagate(traj, surf);
            if(!prop_point) continue;
            prop_point->surface = m_target_surface_ID[isurf];
            prop_point->system = m_target_detector_ID[isurf];
            this_propagated_track.addToPoints(*prop_point);
            isurf++;
        }
        propagated_tracks.push_back(this_propagated_track); 
    }

    SetCollection<edm4eic::TrackSegment>(GetOutputTags()[0], std::move(propagated_tracks));

}


void eicrecon::TrackPropagation_factory::SetPropagationSurfaces() {

    auto transform = Acts::Transform3::Identity();
    
    // shift projection surface to average cluster depth
    // shift in z for endcaps, shift in r for barrel
    double ECAL_avgClusterDepth = 50.;	  // mm
    double HCAL_avgClusterDepth = 150.;   // mm
    
    // extend surfaces by ten percent for tracks close to the edge
    // extend in r for endcaps, extend in z for barrel
    double extend = 1.1;

    // Create propagation surface for BEMC
    const double BEMC_R     = (m_geoSvc->detector()->constant<double>("EcalBarrel_rmin") / dd4hep::mm) * Acts::UnitConstants::mm;
    const double BEMC_halfz = (std::max(m_geoSvc->detector()->constant<double>("EcalBarrelBackward_zmax"), 
			    m_geoSvc->detector()->constant<double>("EcalBarrelForward_zmax")) / dd4hep::mm) * extend * Acts::UnitConstants::mm;
    auto BEMC_Trf         = transform * Acts::Translation3(Acts::Vector3(0, 0, 0));
    auto m_BEMC_prop_surface1  = Acts::Surface::makeShared<Acts::CylinderSurface>(BEMC_Trf, BEMC_R, BEMC_halfz);
    auto m_BEMC_prop_surface2  = Acts::Surface::makeShared<Acts::CylinderSurface>(BEMC_Trf, BEMC_R + ECAL_avgClusterDepth, BEMC_halfz);
    m_target_surface_list.push_back(m_BEMC_prop_surface1);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("ECalBarrel_ID"));
    m_target_surface_ID.push_back(1);
    m_target_surface_list.push_back(m_BEMC_prop_surface2);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("ECalBarrel_ID"));
    m_target_surface_ID.push_back(2);

    // Create propagation surface for FEMC
    const double FEMC_Z    = (m_geoSvc->detector()->constant<double>("EcalEndcapP_zmin") / dd4hep::mm) * Acts::UnitConstants::mm;
    const double FEMC_MinR   = 0.0;      
    const double FEMC_MaxR   = (m_geoSvc->detector()->constant<double>("EcalEndcapP_rmax") / dd4hep::mm) * extend * Acts::UnitConstants::mm;
    auto FEMC_Bounds       = std::make_shared<Acts::RadialBounds>(FEMC_MinR, FEMC_MaxR);
    auto FEMC_Trf1         = transform * Acts::Translation3(Acts::Vector3(0, 0, FEMC_Z));
    auto FEMC_Trf2         = transform * Acts::Translation3(Acts::Vector3(0, 0, FEMC_Z + ECAL_avgClusterDepth));
    auto m_FEMC_prop_surface1   = Acts::Surface::makeShared<Acts::DiscSurface>(FEMC_Trf1, FEMC_Bounds);
    auto m_FEMC_prop_surface2   = Acts::Surface::makeShared<Acts::DiscSurface>(FEMC_Trf2, FEMC_Bounds);
    m_target_surface_list.push_back(m_FEMC_prop_surface1);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("ECalEndcapP_ID"));
    m_target_surface_ID.push_back(1);
    m_target_surface_list.push_back(m_FEMC_prop_surface2);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("ECalEndcapP_ID"));
    m_target_surface_ID.push_back(2);
    
    // Create propagation surface for EEMC 
    const double EEMC_Z    = -(m_geoSvc->detector()->constant<double>("EcalEndcapN_zmin") / dd4hep::mm) * Acts::UnitConstants::mm;
    const double EEMC_MinR = 0.0;    
    const double EEMC_MaxR = (m_geoSvc->detector()->constant<double>("EcalEndcapN_structure_Oring_min") / dd4hep::mm) * extend * Acts::UnitConstants::mm;
    auto EEMC_Bounds       = std::make_shared<Acts::RadialBounds>(EEMC_MinR, EEMC_MaxR);
    auto EEMC_Trf1         = transform * Acts::Translation3(Acts::Vector3(0, 0, EEMC_Z));
    auto EEMC_Trf2         = transform * Acts::Translation3(Acts::Vector3(0, 0, EEMC_Z - ECAL_avgClusterDepth));
    auto m_EEMC_prop_surface1   = Acts::Surface::makeShared<Acts::DiscSurface>(EEMC_Trf1, EEMC_Bounds);
    auto m_EEMC_prop_surface2   = Acts::Surface::makeShared<Acts::DiscSurface>(EEMC_Trf2, EEMC_Bounds);
    m_target_surface_list.push_back(m_EEMC_prop_surface1);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("ECalEndcapN_ID"));
    m_target_surface_ID.push_back(1);
    m_target_surface_list.push_back(m_EEMC_prop_surface2);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("ECalEndcapN_ID"));
    m_target_surface_ID.push_back(2);
      
    // Create propagation surface for OHCAL
    const double OHCAL_R     = (m_geoSvc->detector()->constant<double>("HcalBarrel_rmin") / dd4hep::mm) * Acts::UnitConstants::mm;
    const double OHCAL_halfz = (std::max(m_geoSvc->detector()->constant<double>("HcalBarrelBackward_zmax"), 
			    m_geoSvc->detector()->constant<double>("HcalBarrelForward_zmax")) / dd4hep::mm) * extend * Acts::UnitConstants::mm;
    auto OHCAL_Trf           = transform * Acts::Translation3(Acts::Vector3(0, 0, 0));
    auto m_OHCAL_prop_surface1    = Acts::Surface::makeShared<Acts::CylinderSurface>(OHCAL_Trf, OHCAL_R, OHCAL_halfz);
    auto m_OHCAL_prop_surface2    = Acts::Surface::makeShared<Acts::CylinderSurface>(OHCAL_Trf, OHCAL_R + HCAL_avgClusterDepth, OHCAL_halfz);
    m_target_surface_list.push_back(m_OHCAL_prop_surface1);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("HCalBarrel_ID"));
    m_target_surface_ID.push_back(1);
    m_target_surface_list.push_back(m_OHCAL_prop_surface2);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("HCalBarrel_ID"));
    m_target_surface_ID.push_back(2);

    // Create propagation surface for LFHCAL
    const double LFHCAL_Z    = (m_geoSvc->detector()->constant<double>("HcalEndcapP_zmin") / dd4hep::mm) * Acts::UnitConstants::mm;
    const double LFHCAL_MinR = 0.0;
    const double LFHCAL_MaxR = (m_geoSvc->detector()->constant<double>("HcalEndcapP_rmax") / dd4hep::mm) * extend * Acts::UnitConstants::mm;
    auto LFHCAL_Bounds       = std::make_shared<Acts::RadialBounds>(LFHCAL_MinR, LFHCAL_MaxR);
    auto LFHCAL_Trf1         = transform * Acts::Translation3(Acts::Vector3(0, 0, LFHCAL_Z));
    auto LFHCAL_Trf2         = transform * Acts::Translation3(Acts::Vector3(0, 0, LFHCAL_Z + HCAL_avgClusterDepth));
    auto m_LFHCAL_prop_surface1   = Acts::Surface::makeShared<Acts::DiscSurface>(LFHCAL_Trf1, LFHCAL_Bounds);
    auto m_LFHCAL_prop_surface2   = Acts::Surface::makeShared<Acts::DiscSurface>(LFHCAL_Trf2, LFHCAL_Bounds);
    m_target_surface_list.push_back(m_LFHCAL_prop_surface1);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("HCalEndcapP_ID"));
    m_target_surface_ID.push_back(1);
    m_target_surface_list.push_back(m_LFHCAL_prop_surface2);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("HCalEndcapP_ID"));
    m_target_surface_ID.push_back(2);
   
    // Create propagation surface for EHCAL
    const double EHCAL_Z    = -(m_geoSvc->detector()->constant<double>("HcalEndcapN_zmin") / dd4hep::mm) * Acts::UnitConstants::mm;
    const double EHCAL_MinR = 0.0;    
    const double EHCAL_MaxR = (m_geoSvc->detector()->constant<double>("HcalEndcapN_rmax") / dd4hep::mm) * extend * Acts::UnitConstants::mm;
    auto EHCAL_Bounds       = std::make_shared<Acts::RadialBounds>(EHCAL_MinR, EHCAL_MaxR);
    auto EHCAL_Trf1         = transform * Acts::Translation3(Acts::Vector3(0, 0, EHCAL_Z));
    auto EHCAL_Trf2         = transform * Acts::Translation3(Acts::Vector3(0, 0, EHCAL_Z - HCAL_avgClusterDepth));
    auto m_EHCAL_prop_surface1   = Acts::Surface::makeShared<Acts::DiscSurface>(EHCAL_Trf1, EHCAL_Bounds);
    auto m_EHCAL_prop_surface2   = Acts::Surface::makeShared<Acts::DiscSurface>(EHCAL_Trf2, EHCAL_Bounds);
    m_target_surface_list.push_back(m_EHCAL_prop_surface1);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("HCalEndcapN_ID"));
    m_target_surface_ID.push_back(1);
    m_target_surface_list.push_back(m_EHCAL_prop_surface2);
    m_target_detector_ID.push_back(m_geoSvc->detector()->constant<uint16_t>("HCalEndcapN_ID"));
    m_target_surface_ID.push_back(2);
    
    	
    m_log->info("Setting track propagation surfaces to:");
    m_log->info("EEMC_Z    = {}", EEMC_Z);
    m_log->info("EEMC_MinR = {}", EEMC_MinR);
    m_log->info("EEMC_MaxR = {}", EEMC_MaxR);
    m_log->info("FEMC_Z    = {}", FEMC_Z);
    m_log->info("FEMC_MinR = {}", FEMC_MinR);
    m_log->info("FEMC_MaxR = {}", FEMC_MaxR);
    m_log->info("BEMC_R    = {}", BEMC_R);
    m_log->info("BEMC_Z    = {}", BEMC_halfz);
    m_log->info("LFHCAL_Z    = {}", LFHCAL_Z);
    m_log->info("LFHCAL_MinR = {}", LFHCAL_MinR);
    m_log->info("LFHCAL_MaxR = {}", LFHCAL_MaxR);
    m_log->info("EHCAL_Z     = {}", EHCAL_Z);
    m_log->info("EHCAL_MinR  = {}", EHCAL_MinR);
    m_log->info("EHCAL_MaxR  = {}", EHCAL_MaxR);
    m_log->info("OHCAL_R     = {}", OHCAL_R);
    m_log->info("OHCAL_Z     = {}", OHCAL_halfz);
   
}
