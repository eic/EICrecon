// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "BlueprintConfig.h"

#include <algorithm>
#include <format>
#include <functional>
#include <unordered_set>

namespace eicrecon {

BlueprintConfig BlueprintConfig::defaultEpic() {
  BlueprintConfig config;
  config.name          = "ePIC";
  config.root_envelope = {.r_min = 0.0, .r_max = 20.0, .z_min = 20.0, .z_max = 20.0};

  // Define reusable templates
  config.templates = {
      // Barrel tracker template
      {.name     = "barrel_tracker",
       .type     = LayerTemplateSpec::LayerType::Barrel,
       .axes     = "XYZ",
       .envelope = {.r_min = 5.0, .r_max = 5.0, .z_min = 5.0, .z_max = 5.0},
       .navigation_policies =
           {
               {.type = NavigationPolicySpec::Type::Cylinder, .binning = std::nullopt},
               {.type    = NavigationPolicySpec::Type::SurfaceArrayCylinder,
                .binning = BinningSpec{.mode = BinningSpec::Mode::Auto, .values = {}}},
           },
       .attachment = LayerTemplateSpec::AttachmentStrategy::First},

      // Endcap tracker template
      {.name     = "endcap_tracker",
       .type     = LayerTemplateSpec::LayerType::Endcap,
       .axes     = "XZY",
       .envelope = {.r_min = 5.0, .r_max = 5.0, .z_min = 5.0, .z_max = 5.0},
       .navigation_policies =
           {
               {.type = NavigationPolicySpec::Type::Cylinder, .binning = std::nullopt},
               {.type    = NavigationPolicySpec::Type::SurfaceArrayDisc,
                .binning = BinningSpec{.mode = BinningSpec::Mode::Auto, .values = {}}},
           },
       .attachment = LayerTemplateSpec::AttachmentStrategy::First},
  };

  // Define detector components
  config.detectors = {
      // Barrel detectors
      {.name                = "VertexBarrel",
       .template_name       = "barrel_tracker",
       .pattern             = R"(VertexBarrel_layer\d)",
       .container           = "VertexBarrel",
       .binning             = {.mode   = BinningSpec::Mode::FromConstants,
                               .values = {"VertexBarrelL{}_nphi", "VertexBarrelL{}_nz"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name          = "SagittaSiBarrel",
       .template_name = "barrel_tracker",
       .pattern       = R"(SagittaSiBarrel_layer\d)",
       .container     = "SagittaSiBarrel",
       .binning       = {.mode   = BinningSpec::Mode::FromConstants,
                         .values = {"SiBarrelStave1_count", "100"}},
       .note = "Volumes not aligned: translation in x or y - requires changing number of modules "
               "to multiple of 4",
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name          = "OuterSiBarrel",
       .template_name = "barrel_tracker",
       .pattern       = R"(OuterSiBarrel_layer\d)",
       .container     = "OuterSiBarrel",
       .binning       = {.mode = BinningSpec::Mode::Fixed, .values = {"128", "100"}},
       .note = "Volumes not aligned: translation in x or y - requires changing number of modules "
               "to multiple of 4",
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name                = "InnerMPGDBarrel",
       .template_name       = "barrel_tracker",
       .pattern             = R"(InnerMPGDBarrel_layer\d)",
       .container           = "InnerMPGDBarrel",
       .binning             = {.mode = BinningSpec::Mode::Fixed, .values = {"128", "100"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name                = "BarrelTOF",
       .template_name       = "barrel_tracker",
       .pattern             = R"(BarrelTOF_layer\d)",
       .container           = "BarrelTOF",
       .binning             = {.mode = BinningSpec::Mode::Fixed, .values = {"128", "100"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name                = "MPGDOuterBarrel",
       .template_name       = "barrel_tracker",
       .pattern             = R"(MPGDOuterBarrel_layer\d)",
       .container           = "MPGDOuterBarrel",
       .binning             = {.mode = BinningSpec::Mode::Fixed, .values = {"128", "100"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      // Endcap detectors
      {.name                = "InnerTrackerEndcapP",
       .template_name       = "endcap_tracker",
       .pattern             = R"(InnerTrackerEndcapP_layer\d_P)",
       .container           = "InnerTrackerEndcapP",
       .binning             = {.mode   = BinningSpec::Mode::FromExpression,
                               .values = {"5 * SiTrackerEndcapMod_count", "100"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name                = "InnerTrackerEndcapN",
       .template_name       = "endcap_tracker",
       .pattern             = R"(InnerTrackerEndcapN_layer\d_N)",
       .container           = "InnerTrackerEndcapN",
       .binning             = {.mode   = BinningSpec::Mode::FromExpression,
                               .values = {"5 * SiTrackerEndcapMod_count", "100"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name                = "MiddleTrackerEndcapP",
       .template_name       = "endcap_tracker",
       .pattern             = R"(MiddleTrackerEndcapP_layer\d_P)",
       .container           = "MiddleTrackerEndcapP",
       .binning             = {.mode   = BinningSpec::Mode::FromExpression,
                               .values = {"5 * SiTrackerEndcapMod_count", "100"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name                = "MiddleTrackerEndcapN",
       .template_name       = "endcap_tracker",
       .pattern             = R"(MiddleTrackerEndcapN_layer\d_N)",
       .container           = "MiddleTrackerEndcapN",
       .binning             = {.mode   = BinningSpec::Mode::FromExpression,
                               .values = {"5 * SiTrackerEndcapMod_count", "100"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name                = "OuterTrackerEndcapP",
       .template_name       = "endcap_tracker",
       .pattern             = R"(OuterTrackerEndcapP_layer\d_P)",
       .container           = "OuterTrackerEndcapP",
       .binning             = {.mode   = BinningSpec::Mode::FromExpression,
                               .values = {"5 * SiTrackerEndcapMod_count", "100"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name                = "OuterTrackerEndcapN",
       .template_name       = "endcap_tracker",
       .pattern             = R"(OuterTrackerEndcapN_layer\d_N)",
       .container           = "OuterTrackerEndcapN",
       .binning             = {.mode   = BinningSpec::Mode::FromExpression,
                               .values = {"5 * SiTrackerEndcapMod_count", "100"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name                = "ForwardMPGD",
       .template_name       = "endcap_tracker",
       .pattern             = R"(ForwardMPGD_layer\d_P)",
       .container           = "ForwardMPGD",
       .binning             = {.mode   = BinningSpec::Mode::FromConstants,
                               .values = {"ForwardMPGDEndcapMod_count", "30"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      {.name                = "BackwardMPGD",
       .template_name       = "endcap_tracker",
       .pattern             = R"(BackwardMPGD_layer\d_N)",
       .container           = "BackwardMPGD",
       .binning             = {.mode   = BinningSpec::Mode::FromConstants,
                               .values = {"BackwardMPGDEndcapMod_count", "50"}},
       .note                = std::nullopt,
       .axes_override       = std::nullopt,
       .envelope_override   = std::nullopt,
       .attachment_override = std::nullopt},

      // Commented out in original - detector elements exist but no surfaces
      // {.name = "ForwardTOF",
      //  .template_name = "endcap_tracker",
      //  .pattern = R"(ForwardTOF_layer\d)",
      //  .container = "ForwardTOF",
      //  .binning = {.mode = BinningSpec::Mode::Fixed, .values = {"128", "100"}},
      //  .note = "LayerBlueprintNode: no surfaces provided for ForwardTOF_layer2"},

      // {.name = "B0Tracker",
      //  .template_name = "endcap_tracker",
      //  .pattern = R"(B0Tracker_layer\d)",
      //  .container = "B0Tracker",
      //  .binning = {.mode = BinningSpec::Mode::Fixed, .values = {"64", "50"}},
      //  .note = "Off-axis detector - r=[35-150], z=[+5895,+6705]"},
  };

  // Define hierarchy structure
  config.hierarchy = {
      .name     = "root",
      .type     = HierarchyNode::NodeType::Container,
      .children = {
          {.name = "tracker0",
           .type = HierarchyNode::NodeType::Container,
           .children =
               {
                   {.name     = "InnerTrackerEndcapN",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
                   {.name     = "VertexBarrel",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
                   {.name     = "InnerTrackerEndcapP",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
               }},
          {.name = "tracker1",
           .type = HierarchyNode::NodeType::Container,
           .children =
               {
                   {.name     = "SagittaSiBarrel",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
                   {.name     = "OuterSiBarrel",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
               }},
          {.name = "tracker2",
           .type = HierarchyNode::NodeType::Container,
           .children =
               {
                   {.name     = "BackwardMPGD",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
                   {.name     = "OuterTrackerEndcapN",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
                   {.name     = "MiddleTrackerEndcapN",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
                   {.name     = "MiddleTrackerEndcapP",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
                   {.name     = "OuterTrackerEndcapP",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
                   {.name     = "ForwardMPGD",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
               }},
          {.name = "tracker3",
           .type = HierarchyNode::NodeType::Container,
           .children =
               {
                   {.name     = "InnerMPGDBarrel",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
                   {.name = "BarrelTOF", .type = HierarchyNode::NodeType::Detector, .children = {}},
                   {.name     = "MPGDOuterBarrel",
                    .type     = HierarchyNode::NodeType::Detector,
                    .children = {}},
               }},
      }};

  return config;
}

std::string BlueprintConfig::validate() const {
  // Check templates exist
  std::unordered_set<std::string> template_names;
  for (const auto& tmpl : templates) {
    if (tmpl.name.empty()) {
      return "Template with empty name found";
    }
    if (!template_names.insert(tmpl.name).second) {
      return std::format("Duplicate template name: {}", tmpl.name);
    }
  }

  // Check detectors reference valid templates
  std::unordered_set<std::string> detector_names;
  for (const auto& det : detectors) {
    if (det.name.empty()) {
      return "Detector with empty name found";
    }
    if (!detector_names.insert(det.name).second) {
      return std::format("Duplicate detector name: {}", det.name);
    }
    if (!template_names.contains(det.template_name)) {
      return std::format("Detector '{}' references unknown template '{}'", det.name,
                         det.template_name);
    }
    if (det.pattern.empty()) {
      return std::format("Detector '{}' has empty pattern", det.name);
    }
    if (det.container.empty()) {
      return std::format("Detector '{}' has empty container", det.name);
    }
  }

  // Recursively validate hierarchy references valid detectors
  std::function<std::string(const HierarchyNode&)> validate_node;
  validate_node = [&](const HierarchyNode& node) -> std::string {
    if (node.name.empty()) {
      return "Hierarchy node with empty name found";
    }
    if (node.type == HierarchyNode::NodeType::Detector) {
      if (!detector_names.contains(node.name)) {
        return std::format("Hierarchy references unknown detector: {}", node.name);
      }
    } else {
      for (const auto& child : node.children) {
        if (auto err = validate_node(child); !err.empty()) {
          return err;
        }
      }
    }
    return "";
  };

  if (auto err = validate_node(hierarchy); !err.empty()) {
    return err;
  }

  return ""; // Valid
}

} // namespace eicrecon
