#ifndef EICRECON_SPACE_POINT_H
#define EICRECON_SPACE_POINT_H

#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Seeding/Seed.hpp>
#include "ActsGeometryProvider.h"
namespace eicrecon {

class SpacePoint : public edm4eic::TrackerHit
{
 public:
  const Acts::Surface *_surface = nullptr;

 SpacePoint(const TrackerHit& hit) : TrackerHit(hit) {}

  void setSurface(std::shared_ptr<const ActsGeometryProvider> m_geoSvc)
    {
      const auto its = m_geoSvc->surfaceMap().find(getCellID());
      if (its == m_geoSvc->surfaceMap().end()) {
	_surface = nullptr;
      }
      else {
	_surface = its->second;
      }
    }

  float x() const { return getPosition()[0]; }
  float y() const { return getPosition()[1]; }
  float z() const { return getPosition()[2]; }
  float r() const { return std::hypot(x(), y()); }
  float varianceR() const
  {
    return (std::pow(x(), 2) * getPositionError().xx +
	    std::pow(y(), 2) * getPositionError().yy) /
      (std::pow(x(), 2) + std::pow(y(), 2));
  }
  float varianceZ() const { return getPositionError().zz; }
  
  bool isOnSurface() const {
    if (_surface == nullptr) {
      return false;
    }
    return _surface->isOnSurface(Acts::GeometryContext(), {x(), y(), z()},
				 {0, 0, 0});
  }
};

inline bool operator==(SpacePoint a, SpacePoint b)
{
  return (a.getObjectID() == b.getObjectID()); 
}
static bool spCompare(SpacePoint r, SpacePoint s)
{
  return
    std::hypot(r.x(), r.y(), r.z()) <
    std::hypot(s.x(), s.y(), s.z());
}

using SpacePointPtr = std::unique_ptr<SpacePoint>;
/// Container of sim seed
using SeedContainer = std::vector<Acts::Seed<SpacePoint>>;

}

#endif
