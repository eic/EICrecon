// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck

#include "DD4hepBField.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <DD4hep/Fields.h>
#include <DD4hep/Objects.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <Eigen/Core>
#include <limits>

namespace eicrecon::BField {

  Acts::Result<Acts::Vector3> DD4hepBField::getField(const Acts::Vector3& position,
                                                     Acts::MagneticFieldProvider::Cache& /*cache*/) const
  {
    dd4hep::Position pos(
      position[0] * (dd4hep::mm / Acts::UnitConstants::mm),
      position[1] * (dd4hep::mm / Acts::UnitConstants::mm),
      position[2] * (dd4hep::mm / Acts::UnitConstants::mm));

    auto fieldObj = m_det->field();
    auto field = fieldObj.magneticField(pos) * (Acts::UnitConstants::T / dd4hep::tesla);

    // FIXME Acts doesn't seem to like exact zero components
    if (field.x() * field.y() * field.z() == 0) {
      static dd4hep::Direction epsilon{
        std::numeric_limits<double>::epsilon(),
        std::numeric_limits<double>::epsilon(),
        std::numeric_limits<double>::epsilon()
      };
      field += epsilon;
    }

    return Acts::Result<Acts::Vector3>::success({field.x(), field.y(), field.z()});
  }

  Acts::Result<Acts::Vector3> DD4hepBField::getFieldGradient(const Acts::Vector3& position,
                                                             Acts::ActsMatrix<3, 3>& /*derivative*/,
                                                             Acts::MagneticFieldProvider::Cache& cache) const
  {
    return this->getField(position, cache);
  }

} // namespace eicrecon::BField
