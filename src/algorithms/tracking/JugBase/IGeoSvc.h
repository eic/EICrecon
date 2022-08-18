// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong

#ifndef IGEOSVC_H
#define IGEOSVC_H

#include <unordered_map>
#include <map>

namespace dd4hep {
  class Detector;
  class DetElement;
  namespace rec {
    class CellIDPositionConverter;
    class Surface;
  }
} // namespace dd4hep

namespace Acts {
  class TrackingGeometry;
  class Surface;
  class MagneticFieldProvider;
}

namespace genfit {
  class DetPlane;
}

/** Geometry service interface.
 *
 * \ingroup base
 * \ingroup geosvc
 */
class IGeoSvc {
public:
  using VolumeSurfaceMap = std::unordered_map<uint64_t, const Acts::Surface*>;

public:

  // receive DD4hep Geometry
  virtual dd4hep::DetElement getDD4HepGeo() = 0;
  virtual dd4hep::Detector* detector() = 0;
  virtual std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> cellIDPositionConverter() const = 0;
  virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const = 0;

  virtual std::shared_ptr<const Acts::MagneticFieldProvider>  getFieldProvider() const = 0;
  virtual double centralMagneticField() const = 0;

  virtual const VolumeSurfaceMap& surfaceMap() const = 0;

  // Note this hsould return a const& but is just copied for the moment to get around genfit's api
  /// Genfit DetPlane map
  virtual std::map<int64_t, std::shared_ptr<genfit::DetPlane>> getDetPlaneMap() const = 0;
  virtual std::map< int64_t, dd4hep::rec::Surface* > getDD4hepSurfaceMap() const =0;

  //virtual std::map< int64_t, dd4hep::rec::Surface* > getDetPlaneMap() const = 0 ;

  virtual ~IGeoSvc() {}
};

#endif  // IGEOSVC_H
