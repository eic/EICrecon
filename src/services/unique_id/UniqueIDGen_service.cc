#include "services/unique_id/UniqueIDGen_service.h"

#include <JANA/JApplication.h>
#include <JANA/JService.h>

UniqueIDGen_service::UniqueIDGen_service(JApplication* app)
    : m_app(app), m_uniqueGenIDSvc(algorithms::UniqueIDGenSvc::instance()) {
  size_t seed = 1;
  m_app->SetDefaultParameter("seed", seed, "Random seed for the internal random engine");
  m_uniqueGenIDSvc.setProperty("seed", seed);
}
