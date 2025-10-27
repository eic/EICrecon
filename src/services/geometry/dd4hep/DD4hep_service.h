// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Whitney Armstrong, Wouter Deconinck, David Lawrence
//

#pragma once

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JServiceFwd.h>
#include <spdlog/logger.h>
#include <gsl/pointers>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class DD4hep_service : public JService {
public:
  DD4hep_service(JApplication* app) : m_app(app) {}
  virtual ~DD4hep_service();

  virtual gsl::not_null<const dd4hep::Detector*> detector();
  virtual gsl::not_null<const dd4hep::rec::CellIDPositionConverter*> converter();

protected:
  void Initialize();

private:
  DD4hep_service() = default;
  void acquire_services(JServiceLocator*) override;

  std::once_flag init_flag;
  JApplication* m_app                                                            = nullptr;
  std::unique_ptr<const dd4hep::Detector> m_dd4hepGeo                            = nullptr;
  std::unique_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;
  std::vector<std::string> m_xml_files;

  /// Ensures there is a geometry file that should be opened
  static std::string resolveFileName(const std::string& filename, char* detector_path_env);

  std::shared_ptr<spdlog::logger> m_log;
};
