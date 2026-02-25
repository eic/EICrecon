#pragma once

#include <Acts/EventData/Seed.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include "ActsDD4hepDetector.h"

namespace eicrecon {

class SpacePoint : public edm4eic::TrackerHit {
public:
  const Acts::Surface* m_surface = nullptr;

  SpacePoint(const TrackerHit& hit) : TrackerHit(hit) {}

  void setSurface(std::shared_ptr<const eicrecon::ActsDD4hepDetector> acts_detector) {
    const auto its = acts_detector->surfaceMap().find(getCellID());
    if (its == acts_detector->surfaceMap().end()) {
      m_surface = nullptr;
    } else {
      m_surface = its->second;
    }
  }

  float x() const { return getPosition()[0]; }
  float y() const { return getPosition()[1]; }
  float z() const { return getPosition()[2]; }
  float r() const { return std::hypot(x(), y()); }
  float varianceR() const {
    return (std::pow(x(), 2) * getPositionError().xx + std::pow(y(), 2) * getPositionError().yy) /
           (std::pow(x(), 2) + std::pow(y(), 2));
  }
  float varianceZ() const { return getPositionError().zz; }

  float t() const { return getTime(); }
  float varianceT() const { return getTimeError(); }

  bool isOnSurface(const Acts::GeometryContext& gctx) const {
    if (m_surface == nullptr) {
      return false;
    }
    return m_surface->isOnSurface(gctx, {x(), y(), z()}, {0, 0, 0});
  }
};

inline bool operator==(SpacePoint a, SpacePoint b) { return (a.getObjectID() == b.getObjectID()); }

using SpacePointPtr = std::unique_ptr<SpacePoint>;
/// Container of sim seed
using SeedContainer = std::vector<Acts::Seed<SpacePoint>>;

} // namespace eicrecon
