// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/JServiceFwd.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>

class JApplication;
class JServiceLocator;
namespace dd4hep::rec {
class CellIDPositionConverter;
}
namespace dd4hep {
class Detector;
}
namespace richgeo {
class ActsGeo;
}
namespace richgeo {
class IrtGeo;
}
namespace richgeo {
class ReadoutGeo;
}
namespace spdlog {
class logger;
}

class RichGeo_service : public JService {
public:
  RichGeo_service(JApplication* /* app */) {}
  virtual ~RichGeo_service();

  // return pointer to the main DD4hep Detector
  virtual const dd4hep::Detector* GetDD4hepGeo() { return m_dd4hepGeo; };

  // return pointers to geometry bindings; initializes the bindings upon the first time called
  virtual richgeo::IrtGeo* GetIrtGeo(std::string detector_name);
  virtual const richgeo::ActsGeo* GetActsGeo(std::string detector_name);
  virtual std::shared_ptr<richgeo::ReadoutGeo> GetReadoutGeo(std::string detector_name,
                                                             std::string readout_class);

private:
  RichGeo_service() = default;
  void acquire_services(JServiceLocator*) override;

  std::mutex m_init_lock;
  std::map<std::string, std::once_flag> m_init_irt;
  std::map<std::string, std::once_flag> m_init_acts;
  std::map<std::string, std::once_flag> m_init_readout;

  const dd4hep::Detector* m_dd4hepGeo                     = nullptr;
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;
  richgeo::IrtGeo* m_irtGeo                               = nullptr;
  richgeo::ActsGeo* m_actsGeo                             = nullptr;
  std::shared_ptr<richgeo::ReadoutGeo> m_readoutGeo;

  std::shared_ptr<spdlog::logger> m_log;
};
