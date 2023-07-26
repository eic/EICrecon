//
//
// TODO: Fix the following:
// This class inspired by and benefited from the work done here at the following
// links. A couple of lines were outright copied.
//
// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugBase/src/components/GeoSvc.h
// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugBase/src/components/GeoSvc.cpp
//
// Copyright carried by that code:
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck,
// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//


#pragma once


#include <iostream>
#include <vector>
#include <string>

#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/SurfaceManager.h>
#include <DDRec/Surface.h>
#include <DD4hep/DD4hepUnits.h>
#include "algorithms/tracking/ActsGeometryProvider.h"


class ACTSGeo_service : public JService
{
public:
    ACTSGeo_service( JApplication *app ) : m_app(app) {}
    virtual ~ACTSGeo_service();

    virtual std::shared_ptr<const ActsGeometryProvider> actsGeoProvider();

protected:


private:
    ACTSGeo_service()=default;
    void acquire_services(JServiceLocator *) override;

    std::once_flag init_flag;
    JApplication *m_app = nullptr;
    dd4hep::Detector* m_dd4hepGeo = nullptr;
    std::shared_ptr<ActsGeometryProvider> m_acts_provider;
	//std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;
    //std::vector<std::string> m_xmlFileNames;

    // General acts log
    std::shared_ptr<spdlog::logger> m_log;
    std::shared_ptr<spdlog::logger> m_init_log;
};
