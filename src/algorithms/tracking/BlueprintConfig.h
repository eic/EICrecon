// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace eicrecon {

/// Binning specification for navigation policies
struct BinningSpec {
  enum class Mode {
    Auto,          // Let Acts auto-detect from geometry
    Fixed,         // Fixed number of bins [nphi, nz] or [nr, nz]
    FromConstants, // Read bin counts from DD4hep constants
    FromExpression // Evaluate expression using DD4hep constants
  };

  Mode mode = Mode::Auto;

  // For Fixed mode: literal values like {"128", "100"}
  // For FromConstants: constant names like {"SiBarrelStave1_count", "100"}
  // For FromExpression: expressions like {"5 * SiTrackerEndcapMod_count", "100"}
  std::vector<std::string> values;

  bool operator==(const BinningSpec&) const = default;
};

/// Navigation policy specification
struct NavigationPolicySpec {
  enum class Type {
    Cylinder,             // Basic cylinder navigation
    SurfaceArrayCylinder, // Surface array for barrel layers
    SurfaceArrayDisc,     // Surface array for endcap/disc layers
    TryAll                // Try all surfaces (fallback)
  };

  Type type;
  std::optional<BinningSpec> binning; // Required for SurfaceArray types

  bool operator==(const NavigationPolicySpec&) const = default;
};

/// Envelope/tolerance specification
struct EnvelopeSpec {
  double r_min = 0.0; // mm - inner radial envelope
  double r_max = 0.0; // mm - outer radial envelope
  double z_min = 0.0; // mm - negative z envelope
  double z_max = 0.0; // mm - positive z envelope

  bool operator==(const EnvelopeSpec&) const = default;
};

/// Layer template - reusable configuration for similar detectors
struct LayerTemplateSpec {
  std::string name;

  enum class LayerType {
    Barrel, // Cylindrical layers (r-phi-z)
    Endcap  // Disc layers (r-phi at fixed z)
  };
  LayerType type;

  // Coordinate axes: "XYZ" for barrel, "XZY" for endcap
  std::string axes = "XYZ";

  // Geometric tolerances/envelopes
  EnvelopeSpec envelope;

  // Navigation policies applied to layers
  std::vector<NavigationPolicySpec> navigation_policies;

  // How to attach this volume to parent
  enum class AttachmentStrategy {
    First,   // Attach to first boundary surface
    Second,  // Attach to second boundary surface
    Midpoint // Attach at midpoint
  };
  AttachmentStrategy attachment = AttachmentStrategy::First;

  bool operator==(const LayerTemplateSpec&) const = default;
};

/// Detector component specification
struct DetectorSpec {
  // Unique name for this detector component
  std::string name;

  // Reference to layer template
  std::string template_name;

  // Regex pattern to match DD4hep DetElements
  std::string pattern;

  // DD4hep container/assembly name
  std::string container;

  // Binning configuration (may override template)
  BinningSpec binning;

  // Optional FIXME/TODO notes preserved from migration
  std::optional<std::string> note;

  // Optional overrides of template values
  std::optional<std::string> axes_override;
  std::optional<EnvelopeSpec> envelope_override;
  std::optional<LayerTemplateSpec::AttachmentStrategy> attachment_override;

  bool operator==(const DetectorSpec&) const = default;
};

/// Hierarchy node - represents tree structure
struct HierarchyNode {
  // Node identifier
  std::string name;

  enum class NodeType {
    Detector, // Leaf node - references a DetectorSpec by name
    Container // Branch node - contains child nodes
  };
  NodeType type;

  // For Container nodes: list of children
  std::vector<HierarchyNode> children;

  bool operator==(const HierarchyNode&) const = default;
};

/// Complete blueprint configuration
struct BlueprintConfig {
  // Detector name
  std::string name = "ePIC";

  // Root volume envelope
  EnvelopeSpec root_envelope;

  // Layer templates (reusable configurations)
  std::vector<LayerTemplateSpec> templates;

  // Detector components
  std::vector<DetectorSpec> detectors;

  // Hierarchy structure
  HierarchyNode hierarchy;

  /// Create default ePIC configuration (migrated from hardcoded Gen3)
  static BlueprintConfig defaultEpic();

  /// Validate configuration consistency
  /// Returns error message if invalid, empty string if valid
  std::string validate() const;

  bool operator==(const BlueprintConfig&) const = default;
};

} // namespace eicrecon
