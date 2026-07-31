// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 ePIC Collaboration

#include <algorithms/service.h>
#include <cmath>
#include <cstdint>
#include <array>
#include <edm4eic/AlignmentDerivativeSetCollection.h>
#include <edm4eic/Measurement2D.h>
#include <edm4eic/MutableAlignmentDerivativeSet.h>
#include <edm4eic/TrackCollection.h>
#include <edm4hep/utils/vector_utils.h>

#include "MeasurementToMille.h"
#include "SiliconAlignmentLabels.h"
#include "algorithms/interfaces/ActsSvc.h"

namespace eicrecon {

void MeasurementToMille::init() {
  m_geo = algorithms::ActsSvc::instance().acts_geometry_provider();

  const auto& trackingGeo    = *m_geo->trackingGeometry();
  const auto& dd4hepDetector = *m_geo->dd4hepDetector();

  // Pass nullptr for the optional spdlog logger; the algorithms framework
  // logger (info/debug methods below) is used instead.
  m_surfaceToLayer = buildSiliconSurfaceLabelMap(trackingGeo, dd4hepDetector);

  info("Initialized: {} silicon surfaces mapped across {} known layers", m_surfaceToLayer.size(),
       kNSiliconLayers);
}

void MeasurementToMille::process(const Input& input, const Output& output) const {
  const auto [tracks, measurements] = input;
  auto [derivatives]                = output;

  std::size_t nDerivatives = 0;

  for (const auto& track : *tracks) {
    // ------------------------------------------------------------------
    // Track quality cuts
    // ------------------------------------------------------------------
    const std::uint32_t ndf = track.getNdf();
    if (ndf == 0) {
      continue;
    }
    const float chi2PerNDF = track.getChi2() / static_cast<float>(ndf);
    if (chi2PerNDF > m_cfg.maxChi2PerNDF) {
      debug("Skipping track: chi2/NDF={:.2f} > {:.2f}", chi2PerNDF, m_cfg.maxChi2PerNDF);
      continue;
    }

    const auto& mom = track.getMomentum();
    const float p   = edm4hep::utils::magnitude(mom);
    if (p < m_cfg.minMomentum) {
      debug("Skipping track: p={:.3f} GeV/c < {:.3f} GeV/c", p, m_cfg.minMomentum);
      continue;
    }

    // ------------------------------------------------------------------
    // Iterate over measurements associated with this track
    // ------------------------------------------------------------------
    for (const auto& meas : track.getMeasurements()) {
      const std::uint64_t geoId = meas.getSurface();

      // Look up which silicon layer this surface belongs to
      auto layerIt = m_surfaceToLayer.find(geoId);
      if (layerIt == m_surfaceToLayer.end()) {
        continue; // not a silicon alignment surface
      }
      const int layerIndex = layerIt->second;

      // Skip fixed (reference) layers
      bool isFixed = false;
      for (const int fl : m_cfg.fixedLayers) {
        if (fl == layerIndex) {
          isFixed = true;
          break;
        }
      }
      if (isFixed) {
        continue;
      }

      // ------------------------------------------------------------------
      // Residual and uncertainty
      //
      // TODO: Replace with the proper residual from the ACTS Kalman smoother:
      //   residual = measured_local_u - predicted_local_u
      // where predicted_local_u comes from Acts::detail::makeTrackAlignmentState().
      // The current approximation sets predicted = 0, so residual = measured_u.
      // This is only valid for perfectly centred sensors and is a placeholder
      // until the ACTS Alignment kernel is accessible from EICrecon.
      // ------------------------------------------------------------------
      const float measuredU      = meas.getLoc().a;
      const float residual       = measuredU; // placeholder: predicted = 0
      const float residualUncert = std::sqrt(std::abs(meas.getCovariance().xx));

      // ------------------------------------------------------------------
      // Local derivatives: ∂residual/∂local_track_parameters
      //
      // For a 1-D strip-like measurement in the bending plane (local u),
      // the simplified Jacobian row is [1, 0, 0, 0, 0] corresponding to the
      // five bound track parameters (loc0, loc1, phi, theta, qOverP).
      //
      // TODO: Use Acts::detail::makeTrackAlignmentState().localDerivatives
      // for the correct Jacobian once ACTS integration is complete.
      // ------------------------------------------------------------------
      constexpr int kNLocalParams                       = 5;
      const std::array<float, kNLocalParams> localDeriv = {1.f, 0.f, 0.f, 0.f, 0.f};

      // ------------------------------------------------------------------
      // Global derivatives: ∂residual/∂alignment_DOFs
      //
      // For a translation in the sensor local-u direction (tx), moving the
      // sensor by δ shifts the residual by −δ → derivative = -1.
      // All other DOFs produce zero to first order at normal incidence.
      //
      // Layout: [tx, ty, tz, rx, ry, rz]
      //
      // TODO: Replace with Acts::detail::makeTrackAlignmentState().globalDerivatives
      // which accounts for sensor orientation and track incidence angle.
      // ------------------------------------------------------------------
      const std::array<float, kAlignNDOF> globalDeriv = {
          -1.f, // tx: translation along local u directly shifts residual
          0.f,  // ty: orthogonal to measurement direction
          0.f,  // tz: along surface normal — negligible at normal incidence
          0.f,  // rx: rotation about local x — no first-order effect on u at normal incidence
          0.f,  // ry: rotation about local y — no first-order effect on u at normal incidence
          0.f,  // rz: rotation about local z — couples v into u; zero in simplified model
      };

      // ------------------------------------------------------------------
      // Fill output object
      // ------------------------------------------------------------------
      auto deriv = derivatives->create(geoId, residual, residualUncert);

      for (int i = 0; i < kNLocalParams; ++i) {
        deriv.addToLocalDerivatives(localDeriv[i]);
      }

      for (int dof = 0; dof < kAlignNDOF; ++dof) {
        deriv.addToGlobalLabels(siliconAlignmentLabel(layerIndex, static_cast<AlignmentDOF>(dof)));
        deriv.addToGlobalDerivatives(globalDeriv[dof]);
      }

      ++nDerivatives;
    } // end loop over track measurements
  } // end loop over tracks

  debug("Filled {} AlignmentDerivativeSet entries from {} tracks", nDerivatives, tracks->size());
  // The measurements collection is available for future use (e.g. outlier cross-checks)
  (void)measurements;
}

} // namespace eicrecon
