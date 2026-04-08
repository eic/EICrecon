// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/EventData/SourceLink.hpp>
#include <ActsPlugins/EDM4hep/PodioUtil.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>

#include <functional>
#include <memory>
#include <optional>

namespace eicrecon {

/**
 * @brief Conversion helper for Acts Podio backend using geometry ID lookups
 *
 * Implements the ActsPlugins::PodioUtil::ConversionHelper interface to provide
 * conversions between Acts objects (Surface, SourceLink) and geometry identifiers
 * needed by the Podio track container backends.
 *
 * This helper uses geometry IDs as the intermediate representation for serialization:
 * - Surfaces are identified by their Acts::GeometryIdentifier
 * - Source links are converted to/from geometry IDs
 *
 * Thread Safety: Create a new instance per event to ensure thread safety in
 * concurrent event processing.
 */
class PodioGeometryIdConversionHelper : public ActsPlugins::PodioUtil::ConversionHelper {
public:
  PodioGeometryIdConversionHelper(const Acts::GeometryContext& gctx,
                                  std::shared_ptr<const Acts::TrackingGeometry> trackingGeo)
      : geoCtx(gctx), trackingGeometry(std::move(trackingGeo)) {}

  std::reference_wrapper<const Acts::GeometryContext> geoCtx;
  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry;

  std::optional<ActsPlugins::PodioUtil::Identifier>
  surfaceToIdentifier(const Acts::Surface& surface) const override {
    // Only return an identifier if the surface is in the tracking geometry
    // Otherwise return nullopt to force full surface serialization
    auto geoId = surface.geometryId();
    if (geoId.value() != 0 && trackingGeometry->findSurface(geoId) != nullptr) {
      return geoId.value();
    }
    return std::nullopt;
  }

  const Acts::Surface*
  identifierToSurface(ActsPlugins::PodioUtil::Identifier identifier) const override {
    return trackingGeometry->findSurface(Acts::GeometryIdentifier{identifier});
  }

  ActsPlugins::PodioUtil::Identifier sourceLinkToIdentifier(const Acts::SourceLink& sl) override {
#if Acts_VERSION_MAJOR >= 39
    // In Acts 39+, source links are type-erased wrappers
    // Need to extract the concrete IndexSourceLink first
    const auto& indexSourceLink = sl.get<ActsExamples::IndexSourceLink>();
    return indexSourceLink.geometryId().value();
#else
    return sl.get<ActsExamples::IndexSourceLink>().geometryId().value();
#endif
  }

  Acts::SourceLink
  identifierToSourceLink(ActsPlugins::PodioUtil::Identifier identifier) const override {
    // Note: ActsPodioEdm backend only stores GeometryIdentifier, not the measurement index
    // The index is set to 0 as it's not needed for track storage/retrieval
    // The geometry ID is sufficient to identify which detector surface the measurement came from
    return Acts::SourceLink{ActsExamples::IndexSourceLink{Acts::GeometryIdentifier{identifier}, 0}};
  }
};

} // namespace eicrecon
