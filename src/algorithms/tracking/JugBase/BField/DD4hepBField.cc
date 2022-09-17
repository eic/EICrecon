// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck

#include "DD4hepBField.h"

#include <cmath>
#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/Objects.h"

//#include "JugBase/VectorHelpers.hpp"

//using Vec        = Jug::Helpers::VectorToActs<ROOT::Math::XYZVector>;
//using Vec2DD4hep = Jug::Helpers::ArrayToRoot<Acts::Vector3>;

namespace Jug::BField {

  Acts::Result<Acts::Vector3> DD4hepBField::getField(const Acts::Vector3& position,
                                                     Acts::MagneticFieldProvider::Cache& /*cache*/) const
  {
    dd4hep::Position pos(position[0]/10.0,position[1]/10.0,position[2]/10.0);
    auto fieldObj = m_det->field();


    auto field = fieldObj.magneticField(pos) * (Acts::UnitConstants::T / dd4hep::tesla);
    return Acts::Result<Acts::Vector3>::success({field.x(), field.y(),field.z()});
  }

  Acts::Result<Acts::Vector3> DD4hepBField::getFieldGradient(const Acts::Vector3& position,
                                                             Acts::ActsMatrix<3, 3>& /*derivative*/,
                                                             Acts::MagneticFieldProvider::Cache& cache) const
  {
    return this->getField(position, cache);
  }
} // namespace Jug::BField
