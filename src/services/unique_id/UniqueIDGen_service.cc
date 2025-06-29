#include "services/unique_id/UniqueIDGen_service.h"

#include <JANA/JApplication.h>
#include <JANA/JService.h>
#include <algorithms/property.h>

UniqueIDGen_service::UniqueIDGen_service(JApplication* app)
    : m_app(app), m_uniqueGenIDSvc(algorithms::UniqueIDGenSvc::instance()) {
  for (const auto& [key, prop] : m_uniqueGenIDSvc.getProperties()) {
    std::visit(
        [this, key = key](auto&& val) {
          m_app->SetDefaultParameter(std::string(key), val); // FIXME add description
          this->m_uniqueGenIDSvc.setProperty(key, val);
        },
        prop.get());
  }
}
