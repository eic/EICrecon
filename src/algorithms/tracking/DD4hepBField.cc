// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Dmitry Kalinkin

#include "DD4hepBField.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Utilities/Result.hpp>
#include <DD4hep/Fields.h>
#include <DD4hep/Objects.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <Eigen/Core>
#include <cmath>
#include <limits>

namespace eicrecon::BField {

Acts::Result<Acts::Vector3>
DD4hepBField::getField(const Acts::Vector3& position,
                       Acts::MagneticFieldProvider::Cache& /*cache*/) const {
  dd4hep::Position pos(position[0] * (dd4hep::mm / Acts::UnitConstants::mm),
                       position[1] * (dd4hep::mm / Acts::UnitConstants::mm),
                       position[2] * (dd4hep::mm / Acts::UnitConstants::mm));

  // Avoid crash during lookup.
  // Infinite values are possible due to https://github.com/acts-project/acts/issues/4166.
  if ((!std::isfinite(position[0])) && (!std::isfinite(position[1])) &&
      (!std::isfinite(position[2]))) {
    return Acts::Result<Acts::Vector3>::success({0., 0., 0.});
  }

  auto fieldObj = m_det->field();
  auto field    = fieldObj.magneticField(pos) * (Acts::UnitConstants::T / dd4hep::tesla);

  // FIXME Acts doesn't seem to like exact zero components
  if (field.x() * field.y() * field.z() == 0) {
    static dd4hep::Direction epsilon{std::numeric_limits<double>::epsilon(),
                                     std::numeric_limits<double>::epsilon(),
                                     std::numeric_limits<double>::epsilon()};
    field += epsilon;
  }

  return Acts::Result<Acts::Vector3>::success({field.x(), field.y(), field.z()});
}

#if Acts_VERSION_MAJOR < 39
Acts::Result<Acts::Vector3>
DD4hepBField::getFieldGradient(const Acts::Vector3& position,
                               Acts::ActsMatrix<3, 3>& /*derivative*/,
                               Acts::MagneticFieldProvider::Cache& cache) const {
  return this->getField(position, cache);
}
#endif

} // namespace eicrecon::BField
