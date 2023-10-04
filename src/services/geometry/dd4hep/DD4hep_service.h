// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Whitney Armstrong, Wouter Deconinck, David Lawrence
//

#pragma once

#include <vector>
#include <string>

#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>

class DD4hep_service : public JService
{
public:
    DD4hep_service( JApplication *app ) : app(app) {}
    virtual ~DD4hep_service();

    virtual dd4hep::Detector* detector();
    virtual std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> cellIDPositionConverter();
protected:
    void Initialize();

private:
    DD4hep_service()=default;

    std::once_flag init_flag;
    JApplication *app = nullptr;
    std::unique_ptr<const dd4hep::Detector> m_dd4hepGeo = nullptr;
    std::unique_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;
    std::vector<std::string> m_xml_files;

    /// Ensures there is a geometry file that should be opened
    std::string resolveFileName(const std::string &filename, char *detector_path_env);
};
