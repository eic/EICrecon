// This file is part of the Acts project.
//
// Copyright (C) 2016-2018 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <DD4hep/Detector.h>
#include <gsl/pointers>
#include <memory>
#include <utility>
#include <variant>

namespace eicrecon::BField {

///// The Context to be handed around
//struct ScalableBFieldContext {
//  double scalor = 1.;
//};

/** Use the dd4hep magnetic field in acts.
   *
   * \ingroup magnets
   * \ingroup magsvc
   */
class DD4hepBField final : public Acts::MagneticFieldProvider {
public:
  gsl::not_null<const dd4hep::Detector*> m_det;

public:
  struct Cache {
    Cache(const Acts::MagneticFieldContext& /*mcfg*/) {}
  };

  Acts::MagneticFieldProvider::Cache
  makeCache(const Acts::MagneticFieldContext& mctx) const override {
    return Acts::MagneticFieldProvider::Cache(std::in_place_type<Cache>, mctx);
  }

  /** construct constant magnetic field from field vector.
    *
    * @param [in] DD4hep detector instance
    */
  explicit DD4hepBField(gsl::not_null<const dd4hep::Detector*> det) : m_det(det) {}

  /**  retrieve magnetic field value.
     *
     *  @param [in] position global position
     *  @param [in] cache Cache object (is ignored)
     *  @return magnetic field vector
     *
     *  @note The @p position is ignored and only kept as argument to provide
     *        a consistent interface with other magnetic field services.
     */
  Acts::Result<Acts::Vector3> getField(const Acts::Vector3& position,
                                       Acts::MagneticFieldProvider::Cache& cache) const override;

#if Acts_VERSION_MAJOR < 39
  /** @brief retrieve magnetic field value & its gradient
     *
     * @param [in]  position   global position
     * @param [out] derivative gradient of magnetic field vector as (3x3)
     * matrix
     * @param [in] cache Cache object (is ignored)
     * @return magnetic field vector
     *
     * @note The @p position is ignored and only kept as argument to provide
     *       a consistent interface with other magnetic field services.
     * @note currently the derivative is not calculated
     * @todo return derivative
     */
  Acts::Result<Acts::Vector3>
  getFieldGradient(const Acts::Vector3& position, Acts::ActsMatrix<3, 3>& /*derivative*/,
                   Acts::MagneticFieldProvider::Cache& cache) const override;
#endif
};

using BFieldVariant = std::variant<std::shared_ptr<const DD4hepBField>>;

} // namespace eicrecon::BField
