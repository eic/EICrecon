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


#pragma once


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
    ~JDD4hep_service();

    dd4hep::Detector* detector();
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> cellIDPositionConverter(){ return m_cellid_converter; }

protected:
    void Initialize();

private:
    JDD4hep_service()=default;

    std::once_flag init_flag;
    JApplication *app = nullptr;
    dd4hep::Detector* m_dd4hepGeo = nullptr;
	std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;
    std::vector<std::string> m_xml_files;

    /// Ensures there is a geometry file that should be opened
    std::string  resolveFileName(const std::string &filename, char *detector_path_env);
};

