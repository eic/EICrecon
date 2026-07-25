// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace eicrecon::geo {

enum class MissingReadoutPolicy { Disable, Throw };

/**
 * Non-throwing geometry lookup helpers.
 *
 * DD4hep's `dd4hep::Detector::readout(name)` throws (deep in `getRefChild`) when the named
 * readout is not present in the loaded geometry. That is undesirable when running a
 * reconstruction that is wider than the loaded geometry: e.g. a reduced-geometry detector
 * (such as `epic_craterlake_tracking_only`) legitimately does not contain every readout that
 * an algorithm might be configured for. In that situation "the readout is absent" is a valid
 * configuration, not an error, and callers need to be able to distinguish it from a genuine
 * failure without catching a broad exception.
 *
 * These helpers query geometry without throwing on absence: presence is reported via a boolean
 * or via `std::optional` emptiness, so callers can gracefully disable a detector-specific code
 * path (e.g. produce empty output) instead of letting an exception propagate through the JANA2
 * dataflow. A thrown exception continues to mean a genuine failure.
 *
 * They operate on a plain `dd4hep::Detector` reference so they can be used from both the
 * framework-independent algorithms (via `algorithms::GeoSvc::instance().detector()`) and from
 * JANA services (via `DD4hep_service::detector()`).
 */

/// Returns true if a readout with the given name exists in the detector, false otherwise.
/// Does not throw.
inline bool hasReadout(const dd4hep::Detector& detector, const std::string& readout_name) {
  const auto& readouts = detector.readouts();
  return readouts.find(readout_name) != readouts.end();
}

/// Parse the policy for handling geometry readouts that are absent from the loaded detector.
/// - "disable" (or empty) => disable the producer and emit empty output collections
/// - "throw" => fail loudly with an exception
inline MissingReadoutPolicy parseMissingReadoutPolicy(std::string_view policy) {
  if (policy.empty() || policy == "disable") {
    return MissingReadoutPolicy::Disable;
  }
  if (policy == "throw") {
    return MissingReadoutPolicy::Throw;
  }
  throw std::runtime_error("Invalid missingReadoutPolicy value '" + std::string(policy) +
                           "'. Expected 'disable' or 'throw'.");
}

/// Returns the ID descriptor for the named readout, or std::nullopt if the readout is absent
/// (or present but does not carry a valid ID descriptor). Does not throw on absence.
inline std::optional<dd4hep::IDDescriptor> readoutIdSpec(const dd4hep::Detector& detector,
                                                         const std::string& readout_name) {
  if (!hasReadout(detector, readout_name)) {
    return std::nullopt;
  }
  const dd4hep::IDDescriptor id_spec = detector.readout(readout_name).idSpec();
  if (!id_spec.isValid()) {
    return std::nullopt;
  }
  return id_spec;
}

/// Returns the segmentation for the named readout, or std::nullopt if the readout is absent
/// (or present but does not carry a valid segmentation). Does not throw on absence.
inline std::optional<dd4hep::Segmentation> readoutSegmentation(const dd4hep::Detector& detector,
                                                               const std::string& readout_name) {
  if (!hasReadout(detector, readout_name)) {
    return std::nullopt;
  }
  const dd4hep::Segmentation segmentation = detector.readout(readout_name).segmentation();
  if (!segmentation.isValid()) {
    return std::nullopt;
  }
  return segmentation;
}

} // namespace eicrecon::geo
