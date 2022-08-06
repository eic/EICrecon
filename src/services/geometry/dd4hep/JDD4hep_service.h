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
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck
// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//


#ifndef __JDD4hep_service_h__
#define __JDD4hep_service_h__


#include <iostream>
#include <vector>
#include <string>

#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>

#include "DD4hep/Detector.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/SurfaceManager.h"
#include "DDRec/Surface.h"
#include "DD4hep/DD4hepUnits.h"


class JDD4hep_service : public JService
{
public:
    JDD4hep_service( JApplication *app ):app(app){}

    void Initialize();

    dd4hep::Detector* detector(void) { if(!m_dd4hepGeo) Initialize(); return (m_dd4hepGeo); }

private:
    JDD4hep_service(){}

    JApplication *app = nullptr;
    dd4hep::Detector* m_dd4hepGeo = nullptr;
	std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;
    std::vector<std::string> m_xmlFileNames;
};

#endif // __JDD4hep_service_h__
