// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "ActsDD4hepDetectorGen3.h"

#include <Acts/Geometry/BlueprintNode.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Surfaces/SurfaceBounds.hpp>
#include <boost/variant/get.hpp>
#include <initializer_list>
#include <vector>

// This class is only available with Gen3 support, see ActsDD4hepDetectorGen3.h
#if HAS_GEN3_SUPPORT

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/Blueprint.hpp>
#include <Acts/Geometry/BlueprintBuilder.hpp>
#include <Acts/Geometry/BlueprintOptions.hpp>
#include <Acts/Geometry/ContainerBlueprintNode.hpp>
#include <Acts/Geometry/CylinderVolumeBounds.hpp>
#include <Acts/Geometry/Extent.hpp>
#include <Acts/Geometry/LayerBlueprintNode.hpp>
#include <Acts/Geometry/MaterialDesignatorBlueprintNode.hpp>
#include <Acts/Geometry/NavigationPolicyFactory.hpp>
#include <Acts/Geometry/PadBlueprintNode.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Geometry/TrackingVolume.hpp>
#include <Acts/Geometry/VolumeAttachmentStrategy.hpp>
#include <Acts/Material/IMaterialDecorator.hpp>
#include <Acts/Material/ISurfaceMaterial.hpp>
#include <Acts/Material/ProtoSurfaceMaterial.hpp>
#include <Acts/Navigation/CylinderNavigationPolicy.hpp>
#include <Acts/Navigation/TryAllNavigationPolicy.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>
#include <Acts/Utilities/AxisSpec.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <ActsPlugins/DD4hep/BlueprintBuilder.hpp>
#include <ActsPlugins/DD4hep/DD4hepConversionHelpers.hpp>
#include <ActsPlugins/DD4hep/DD4hepDetectorElement.hpp>
#include <DD4hep/DetElement.h>
#include <DDRec/DetectorData.h>
#include <algorithm>
#include <cstddef>
#include <exception>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

namespace eicrecon {

ActsDD4hepDetectorGen3::ActsDD4hepDetectorGen3(const Config& cfg)
    : ActsDD4hepDetector(cfg), m_gen3Cfg(cfg) {
  logger().log(Acts::Logging::INFO, "ActsDD4hepDetectorGen3 constructing...");
  construct();
  logger().log(Acts::Logging::INFO, "ActsDD4hepDetectorGen3 construction complete");
}

std::shared_ptr<ActsPlugins::DD4hepDetectorElement>
ActsDD4hepDetectorGen3::defaultDetectorElementFactory(const dd4hep::DetElement& element,
                                                      ActsPlugins::TGeoAxes axes, double scale) {
  return std::make_shared<ActsPlugins::DD4hepDetectorElement>(element, axes, scale);
}

namespace {

  class LayerHelperCompat {
  public:
    using Builder = ActsPlugins::DD4hep::BlueprintBuilder;

    explicit LayerHelperCompat(const Builder& builder) : m_layers(builder.layers()) {}

    LayerHelperCompat&& barrel() && {
      m_layers = std::move(m_layers).barrel();
      return std::move(*this);
    }

    LayerHelperCompat&& endcap() && {
      m_layers = std::move(m_layers).endcap();
      return std::move(*this);
    }

    LayerHelperCompat&& setSensorAxes(ActsPlugins::TGeoAxes axes) && {
      m_layers = std::move(m_layers).setSensorAxes(axes);
      return std::move(*this);
    }

    LayerHelperCompat&& setLayerAxes(ActsPlugins::TGeoAxes axes) && {
      m_layers = std::move(m_layers).setLayerAxes(axes);
      return std::move(*this);
    }

    LayerHelperCompat&& setAxes(ActsPlugins::TGeoAxes axes) && {
      m_layers = std::move(m_layers).setSensorAxes(axes).setLayerAxes(axes);
      return std::move(*this);
    }

    LayerHelperCompat&& setContainer(std::string container) && {
      m_layers = std::move(m_layers).setContainer(std::move(container));
      return std::move(*this);
    }

    LayerHelperCompat&& setPattern(std::string pattern) && {
      m_layers = std::move(m_layers).setLayerFilter(pattern);
      return std::move(*this);
    }

    LayerHelperCompat&& setEnvelope(const Acts::ExtentEnvelope& envelope) && {
      m_layers = std::move(m_layers).setEnvelope(envelope);
      return std::move(*this);
    }

    LayerHelperCompat&& setEmptyOk(bool emptyOk) && {
      m_layers = std::move(m_layers).setEmptyOk(emptyOk);
      return std::move(*this);
    }

    // The customizer may return a node that wraps the layer (e.g. a material
    // designator), which is then added to the container in place of the layer.
    template <typename CustomizerT> LayerHelperCompat&& customize(CustomizerT customizer) && {
      m_layers = std::move(m_layers).onLayer(
          [c = std::move(customizer)](const std::optional<dd4hep::DetElement>& element,
                                      std::shared_ptr<Acts::LayerBlueprintNode> layer) mutable
              -> std::shared_ptr<Acts::BlueprintNode> {
            if (!element.has_value()) {
              throw std::runtime_error("Layer customizer requires a source dd4hep::DetElement");
            }
            if constexpr (std::is_void_v<decltype(c(*element, layer))>) {
              c(*element, layer);
              return layer;
            } else {
              return c(*element, layer);
            }
          });
      return std::move(*this);
    }

    std::shared_ptr<Acts::ContainerBlueprintNode> build() && { return std::move(m_layers).build(); }

  private:
    ActsPlugins::DD4hep::ElementLayerAssembler m_layers;
  };

  enum class LayerKind { Barrel, Endcap };

  struct LayerBuildSpec {
    LayerKind kind;
    ActsPlugins::TGeoAxes sensorAxes;
    std::optional<ActsPlugins::TGeoAxes> layerAxes;
    std::string pattern;
    std::string container;
    std::string label;
    bool emptyOk = false; // if true, skip layers with no surfaces instead of throwing
    // Portal face of each layer volume that carries material (if the layer has
    // <layer_material>): the face towards the interaction point, at the layer envelope.
    std::optional<Acts::CylinderVolumeBounds::Face> materialFace = std::nullopt;
  };

  // Wrap a layer in a material designator, driven by the same DD4hep parameters
  // that the Gen1 DD4hep plugin reads (set by <layer_material> in the ePIC compact
  // files): layer_material_{inner,outer} with _binPhi and _binZ (barrel) or _binR
  // (endcap). Gen1 maps material onto both approach surfaces of a layer; in Gen3 a
  // portal shared by two layer volumes can carry material from one side only, and
  // portals that are merged when containers are stacked cannot carry material, so
  // only one face per layer is designated. The designated portal carries proto
  // material, which a material mapping job replaces with mapped material.
  std::shared_ptr<Acts::BlueprintNode> wrapWithLayerMaterial(
      const dd4hep::DetElement& element, std::optional<Acts::CylinderVolumeBounds::Face> face,
      std::shared_ptr<Acts::LayerBlueprintNode> layer, const Acts::Logger& logger) {
    if (!face.has_value() || !ActsPlugins::getParamOr<bool>("layer_material", element, false)) {
      return layer;
    }
    const auto& params = ActsPlugins::getParams(element);
    std::string tag;
    for (const auto* candidate : {"layer_material_inner", "layer_material_outer"}) {
      if (params.contains(candidate)) {
        tag = candidate;
        break;
      }
    }
    if (tag.empty()) {
      return layer;
    }
    auto bins = [&params, &tag](const std::string& axis) {
      return static_cast<std::size_t>(std::max(1, params.value_or<int>(tag + "_" + axis, 1)));
    };

    using Face = Acts::CylinderVolumeBounds::Face;
    auto material =
        std::make_shared<Acts::MaterialDesignatorBlueprintNode>(layer->name() + "_Material");
    // Deferred axes take their range and boundary type from the portal surface.
    // Cylinders bin in (rphi, z), discs in (r, phi).
    if (*face == Face::InnerCylinder || *face == Face::OuterCylinder) {
      material->configureFace(*face, Acts::AxisSpec::DeferredEquidistant(bins("binPhi")),
                              Acts::AxisSpec::DeferredEquidistant(bins("binZ")));
    } else {
      material->configureFace(*face, Acts::AxisSpec::DeferredEquidistant(bins("binR")),
                              Acts::AxisSpec::DeferredEquidistant(bins("binPhi")));
    }
    logger.log(Acts::Logging::DEBUG, "Designating material on layer " + layer->name());
    material->addChild(layer);
    return material;
  }

} // namespace

void ActsDD4hepDetectorGen3::construct() {
  using namespace Acts;
  using namespace Acts::UnitLiterals;
  using enum AxisDirection;
  using Face = Acts::CylinderVolumeBounds::Face;

  ActsPlugins::DD4hep::BlueprintBuilder builder{
      {
          .elementFactory = m_gen3Cfg.detectorElementFactory,
          .dd4hepDetector = &dd4hepDetector(),
          .lengthScale    = Acts::UnitConstants::cm,
          .gctx           = getActsGeometryContext(),
      },
      logger().cloneWithSuffix("BlpBld")};

  auto makeLayerHelper = [&builder]() { return LayerHelperCompat{builder}; };

  // BARREL: XYZ
  // ENDCAP: XZY

  Acts::Blueprint::Config cfg;
  cfg.envelope[AxisZ] = {20_mm, 20_mm};
  cfg.envelope[AxisR] = {0_mm, 20_mm};
  Acts::Blueprint root{cfg};

  using AttachmentStrategy = Acts::VolumeAttachmentStrategy;
  const auto defaultLayerEnvelope =
      Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm});

  auto makeAxisString = [](ActsPlugins::TGeoAxes axes) {
    std::ostringstream os;
    os << axes;
    return os.str();
  };

  auto buildLayer = [&](const LayerBuildSpec& spec, auto&& customizer) {
    try {
      auto helper = makeLayerHelper();
      if (spec.kind == LayerKind::Barrel) {
        helper = std::move(helper).barrel();
      } else {
        helper = std::move(helper).endcap();
      }
      helper = std::move(helper).setSensorAxes(spec.sensorAxes);
      if (spec.layerAxes.has_value()) {
        helper = std::move(helper).setLayerAxes(*spec.layerAxes);
      }
      return std::move(helper)
          .setPattern(spec.pattern)
          .setContainer(spec.container)
          .setEnvelope(defaultLayerEnvelope)
          .setEmptyOk(spec.emptyOk)
          .customize([&](const dd4hep::DetElement& element,
                         std::shared_ptr<Acts::LayerBlueprintNode> layer) {
            customizer(element, layer);
            return wrapWithLayerMaterial(element, spec.materialFace, std::move(layer), logger());
          })
          .build();
    } catch (const std::exception& e) {
      std::string layerType = spec.kind == LayerKind::Barrel ? "barrel" : "endcap";
      throw std::runtime_error(
          "Gen3 layer assembly failed for '" + spec.label +
          "' "
          "(container='" +
          spec.container + "', pattern='" + spec.pattern + "', type='" + layerType +
          "', sensorAxes='" + makeAxisString(spec.sensorAxes) + "', layerAxes='" +
          (spec.layerAxes.has_value() ? makeAxisString(*spec.layerAxes) : std::string("<unset>")) +
          "'): " + e.what());
    }
  };

  //
  // DEFINE DETECTORS
  //

  auto makeBarrelPolicyFactory = [&]() {
    return NavigationPolicyFactory{}
        .add<CylinderNavigationPolicy>()
        .add<TryAllNavigationPolicy>()
        .asUniquePtr();
  };

  // VertexBarrel
  auto VertexBarrel =
      buildLayer({.kind         = LayerKind::Barrel,
                  .sensorAxes   = "XYZ",
                  .layerAxes    = std::nullopt,
                  .pattern      = "VertexBarrel_layer\\d",
                  .container    = "VertexBarrel",
                  .label        = "VertexBarrel",
                  .materialFace = Face::InnerCylinder},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                   return layer;
                 });
  VertexBarrel->setAttachmentStrategy(AttachmentStrategy::First);
  // The material face of the innermost layer is the inner face of the container, which
  // would be merged with the inner endcaps in Tracker0; padding keeps it unmerged.
  auto VertexBarrelPadded = std::make_shared<Acts::PadBlueprintNode>(
      "VertexBarrelPadded",
      Acts::ExtentEnvelope{}.set(AxisZ, {1_mm, 1_mm}).set(AxisR, {1_mm, 1_mm}));
  VertexBarrelPadded->addChild(VertexBarrel);

  // SagittaSiBarrel: stave-based barrel, sensor axes only (no layer axes to
  // avoid extracting off-axis layer transforms from individual module placements).
  // Use center-of-gravity only in Z so x/y are forced to 0 for CylinderVolumeStack.
  auto SagittaSiBarrel =
      buildLayer({.kind         = LayerKind::Barrel,
                  .sensorAxes   = "XYZ",
                  .layerAxes    = std::nullopt,
                  .pattern      = "SagittaSiBarrel_layer\\d",
                  .container    = "SagittaSiBarrel",
                  .label        = "SagittaSiBarrel",
                  .materialFace = Face::InnerCylinder},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setUseCenterOfGravity(false, false, true);
                   layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                   return layer;
                 });
  SagittaSiBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // OuterSiBarrel: stave-based barrel, sensor axes only (no layer axes).
  // Use center-of-gravity only in Z so x/y are forced to 0 for CylinderVolumeStack.
  auto OuterSiBarrel =
      buildLayer({.kind         = LayerKind::Barrel,
                  .sensorAxes   = "XYZ",
                  .layerAxes    = std::nullopt,
                  .pattern      = "OuterSiBarrel_layer\\d",
                  .container    = "OuterSiBarrel",
                  .label        = "OuterSiBarrel",
                  .materialFace = Face::InnerCylinder},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setUseCenterOfGravity(false, false, true);
                   layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                   return layer;
                 });
  OuterSiBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // endcapPolicyFactory
  std::shared_ptr<NavigationPolicyFactory> SiTrackerEndcapPolicyFactory =
      NavigationPolicyFactory{}
          .add<CylinderNavigationPolicy>()
          .add<TryAllNavigationPolicy>()
          .asUniquePtr();

  // InnerTrackerEndcapP
  auto InnerTrackerEndcapP =
      buildLayer({.kind         = LayerKind::Endcap,
                  .sensorAxes   = "XZY",
                  .layerAxes    = std::nullopt,
                  .pattern      = "InnerTrackerEndcapP_layer\\d_P",
                  .container    = "InnerTrackerEndcapP",
                  .label        = "InnerTrackerEndcapP",
                  .materialFace = Face::NegativeDisc},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  InnerTrackerEndcapP->setAttachmentStrategy(AttachmentStrategy::First);

  // InnerTrackerEndcapN
  auto InnerTrackerEndcapN =
      buildLayer({.kind         = LayerKind::Endcap,
                  .sensorAxes   = "XZY",
                  .layerAxes    = std::nullopt,
                  .pattern      = "InnerTrackerEndcapN_layer\\d_N",
                  .container    = "InnerTrackerEndcapN",
                  .label        = "InnerTrackerEndcapN",
                  .materialFace = Face::PositiveDisc},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  // Extend the layer volume closer to the IP, so the material face (towards the IP)
  // stays at the envelope of its own layer.
  InnerTrackerEndcapN->setAttachmentStrategy(AttachmentStrategy::Second);

  // MiddleTrackerEndcapP
  auto MiddleTrackerEndcapP =
      buildLayer({.kind         = LayerKind::Endcap,
                  .sensorAxes   = "XZY",
                  .layerAxes    = std::nullopt,
                  .pattern      = "MiddleTrackerEndcapP_layer\\d_P",
                  .container    = "MiddleTrackerEndcapP",
                  .label        = "MiddleTrackerEndcapP",
                  .materialFace = Face::NegativeDisc},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  MiddleTrackerEndcapP->setAttachmentStrategy(AttachmentStrategy::First);

  // MiddleTrackerEndcapN
  auto MiddleTrackerEndcapN =
      buildLayer({.kind         = LayerKind::Endcap,
                  .sensorAxes   = "XZY",
                  .layerAxes    = std::nullopt,
                  .pattern      = "MiddleTrackerEndcapN_layer\\d_N",
                  .container    = "MiddleTrackerEndcapN",
                  .label        = "MiddleTrackerEndcapN",
                  .materialFace = Face::PositiveDisc},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  // Extend the layer volume closer to the IP, so the material face (towards the IP)
  // stays at the envelope of its own layer.
  MiddleTrackerEndcapN->setAttachmentStrategy(AttachmentStrategy::Second);

  // OuterTrackerEndcapP
  auto OuterTrackerEndcapP =
      buildLayer({.kind         = LayerKind::Endcap,
                  .sensorAxes   = "XZY",
                  .layerAxes    = std::nullopt,
                  .pattern      = "OuterTrackerEndcapP_layer\\d_P",
                  .container    = "OuterTrackerEndcapP",
                  .label        = "OuterTrackerEndcapP",
                  .materialFace = Face::NegativeDisc},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  OuterTrackerEndcapP->setAttachmentStrategy(AttachmentStrategy::First);

  // OuterTrackerEndcapN
  auto OuterTrackerEndcapN =
      buildLayer({.kind         = LayerKind::Endcap,
                  .sensorAxes   = "XZY",
                  .layerAxes    = std::nullopt,
                  .pattern      = "OuterTrackerEndcapN_layer\\d_N",
                  .container    = "OuterTrackerEndcapN",
                  .label        = "OuterTrackerEndcapN",
                  .materialFace = Face::PositiveDisc},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  // Extend the layer volume closer to the IP, so the material face (towards the IP)
  // stays at the envelope of its own layer.
  OuterTrackerEndcapN->setAttachmentStrategy(AttachmentStrategy::Second);

  // ForwardMPGD
  std::shared_ptr<NavigationPolicyFactory> ForwardMPGDPolicyFactory =
      NavigationPolicyFactory{}
          .add<CylinderNavigationPolicy>()
          .add<TryAllNavigationPolicy>()
          .asUniquePtr();
  auto ForwardMPGD =
      buildLayer({.kind         = LayerKind::Endcap,
                  .sensorAxes   = "XZY",
                  .layerAxes    = std::nullopt,
                  .pattern      = "ForwardMPGD_layer\\d_P",
                  .container    = "ForwardMPGD",
                  .label        = "ForwardMPGD",
                  .materialFace = Face::NegativeDisc},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(ForwardMPGDPolicyFactory);
                   return layer;
                 });
  ForwardMPGD->setAttachmentStrategy(AttachmentStrategy::First);

  // BackwardMPGD
  std::shared_ptr<NavigationPolicyFactory> BackwardMPGDPolicyFactory =
      NavigationPolicyFactory{}
          .add<CylinderNavigationPolicy>()
          .add<TryAllNavigationPolicy>()
          .asUniquePtr();
  auto BackwardMPGD =
      buildLayer({.kind         = LayerKind::Endcap,
                  .sensorAxes   = "XZY",
                  .layerAxes    = std::nullopt,
                  .pattern      = "BackwardMPGD_layer\\d_N",
                  .container    = "BackwardMPGD",
                  .label        = "BackwardMPGD",
                  .materialFace = Face::PositiveDisc},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(BackwardMPGDPolicyFactory);
                   return layer;
                 });
  // Extend the layer volume closer to the IP, so the material face (towards the IP)
  // stays at the envelope of its own layer.
  BackwardMPGD->setAttachmentStrategy(AttachmentStrategy::Second);

  // InnerMPGDBarrel
  auto InnerMPGDBarrel =
      buildLayer({.kind         = LayerKind::Barrel,
                  .sensorAxes   = "XYZ",
                  .layerAxes    = std::nullopt,
                  .pattern      = "InnerMPGDBarrel_layer\\d",
                  .container    = "InnerMPGDBarrel",
                  .label        = "InnerMPGDBarrel",
                  .materialFace = Face::InnerCylinder},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                   return layer;
                 });
  InnerMPGDBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // BarrelTOF
  auto BarrelTOF =
      buildLayer({.kind         = LayerKind::Barrel,
                  .sensorAxes   = "XYZ",
                  .layerAxes    = std::nullopt,
                  .pattern      = "BarrelTOF_layer\\d",
                  .container    = "BarrelTOF",
                  .label        = "BarrelTOF",
                  .materialFace = Face::InnerCylinder},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                   return layer;
                 });
  BarrelTOF->setAttachmentStrategy(AttachmentStrategy::First);

  // MPGDOuterBarrel
  auto MPGDOuterBarrel =
      buildLayer({.kind         = LayerKind::Barrel,
                  .sensorAxes   = "XYZ",
                  .layerAxes    = std::nullopt,
                  .pattern      = "MPGDOuterBarrel_layer\\d",
                  .container    = "MPGDOuterBarrel",
                  .label        = "MPGDOuterBarrel",
                  .materialFace = Face::InnerCylinder},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                   return layer;
                 });
  MPGDOuterBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // ForwardTOF: use emptyOk so non-sensitive structural layers (support, gap) are
  // silently skipped instead of causing "no surfaces provided" failures.
  // ForwardTOF has layers 1-6; only layers 1 and 2 carry sensitive surfaces.
  std::shared_ptr<NavigationPolicyFactory> ForwardTOFPolicyFactory =
      NavigationPolicyFactory{}
          .add<CylinderNavigationPolicy>()
          .add<TryAllNavigationPolicy>()
          .asUniquePtr();
  auto ForwardTOF =
      buildLayer({.kind         = LayerKind::Endcap,
                  .sensorAxes   = "XZY",
                  .layerAxes    = std::nullopt,
                  .pattern      = "ForwardTOF_layer\\d",
                  .container    = "ForwardTOF",
                  .label        = "ForwardTOF",
                  .emptyOk      = true,
                  .materialFace = Face::NegativeDisc},
                 [&](const dd4hep::DetElement&, std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                   layer->setUseCenterOfGravity(false, false, true);
                   layer->setNavigationPolicyFactory(ForwardTOFPolicyFactory);
                   return layer;
                 });
  ForwardTOF->setAttachmentStrategy(AttachmentStrategy::First);

  // B0Tracker (OFF AXIS): x=−160 mm, z=6300 mm in world frame.
  // CylinderVolumeStack requires all volumes to share a common z-axis (no x/y
  // translation). B0 violates this by construction, so this block is kept
  // commented until Acts Gen3 supports off-axis containers.
  // setUseCenterOfGravity(false, false, true) only zeros the *sensor bounding-box
  // centroid* in x/y; it cannot remove the 160 mm x-offset from the layer
  // representative transform itself.
  /*
  auto B0Tracker = buildLayer({.kind       = LayerKind::Endcap,
                               .sensorAxes = "XZY",
                               .layerAxes  = std::nullopt,
                               .pattern    = "B0Tracker_layer\\d",
                               .container  = "B0Tracker",
                               .label      = "B0Tracker",
                               .emptyOk    = true},
                              [&](const dd4hep::DetElement&,
                                  std::shared_ptr<Acts::LayerBlueprintNode> layer) {
                                layer->setUseCenterOfGravity(false, false, true);
                                layer->setNavigationPolicyFactory(NavigationPolicyFactory{}
                                    .add<CylinderNavigationPolicy>()
                                    .add<TryAllNavigationPolicy>()
                                    .asUniquePtr());
                                return layer;
                              });
  B0Tracker->setAttachmentStrategy(AttachmentStrategy::First);
  */

  //
  // PLACE IN NESTED CONTAINERS
  //

  // TODO DetElement acts_beampipe_central is beampipe
  // constants:
  // - IPBeampipeID,
  // - IPBeampipeUpstreamStraightLength,
  // - IPBeampipeDownstreamStraightLength

  // Note: easiest to think from inside to outside
  // The attachment strategies keep the material faces (towards the IP) of the
  // subdetectors at their own envelope: in R the inner volume is extended, in Z gap
  // volumes are inserted (extending either neighbor would move a material face).
  root.addCylinderContainer("Tracker4", AxisZ, [&](auto& tracker4) {
    tracker4.addCylinderContainer("Tracker3", AxisR, [&](auto& tracker3) {
      tracker3.setAttachmentStrategy(AttachmentStrategy::First);
      tracker3.addStaticVolume(Transform3::Identity(),
                               std::make_unique<CylinderVolumeBounds>(0_mm, 20_mm, 100_mm),
                               "Beampipe");
      tracker3.addCylinderContainer("Tracker2", AxisZ, [&](auto& tracker2) {
        tracker2.setAttachmentStrategy(AttachmentStrategy::Gap);
        tracker2.addChild(BackwardMPGD);         // r=[65–405], z=[−1462,−1324]
        tracker2.addChild(OuterTrackerEndcapN);  // r=[32–426], z=[−1275,−895]
        tracker2.addChild(MiddleTrackerEndcapN); // r=[32-420], z ~ -450
        tracker2.addCylinderContainer("Tracker1", AxisR, [&](auto& tracker1) {
          tracker1.setAttachmentStrategy(AttachmentStrategy::First);
          tracker1.addCylinderContainer("Tracker0", AxisZ, [&](auto& tracker0) {
            tracker0.setAttachmentStrategy(AttachmentStrategy::Gap);
            tracker0.addChild(InnerTrackerEndcapN); // r=[32-245], z ~ -250
            tracker0.addChild(VertexBarrelPadded);  // r=[33–130], z=[−135,+135]
            tracker0.addChild(InnerTrackerEndcapP); // r=[32-245], z ~ +250
          });
          tracker1.addChild(SagittaSiBarrel); // r=[258-275], z=[-256,+256]
          tracker1.addChild(OuterSiBarrel);   // r=[413–430], z=[−402,+402]
        });
        tracker2.addChild(MiddleTrackerEndcapP); // r=[32-420], z ~ +450
        tracker2.addChild(OuterTrackerEndcapP);  // r=[34–426], z=[+695,+1355]
        tracker2.addChild(ForwardMPGD);          // r=[76–405], z=[+1249,+1387]
      });
      tracker3.addChild(InnerMPGDBarrel); // r=[547–589], z=[−1192,+1192]
      tracker3.addChild(BarrelTOF);       // r=[629–654], z=[−1285,+1285]
      tracker3.addChild(MPGDOuterBarrel); // r=[731–762], z=[−1700,+1700]
    });
    //tracker4.addChild(ForwardTOF);                     // r=[101–602], z ~ +1861
    //tracker4.addChild(B0Tracker);                      // r=[35-150], z=[+5895,+6705] OFF-AXIS
  });

  // @TODO: Add plugin way to take this from xml

  BlueprintOptions options;

  // Keep the geometry mutable until the material is applied
  std::unique_ptr<Acts::TrackingGeometry> trackingGeometry =
      root.construct(options, getActsGeometryContext(), logger());

  if (!trackingGeometry) {
    logger().log(Acts::Logging::ERROR, "Failed to construct tracking geometry from blueprint");
    throw std::runtime_error("Blueprint construction failed");
  }

  // Proto material only marks a surface for material mapping; it carries no material.
  auto isProtoMaterial = [](const Acts::ISurfaceMaterial* material) {
    return dynamic_cast<const Acts::ProtoSurfaceMaterial*>(material) != nullptr ||
           dynamic_cast<const Acts::ProtoGridSurfaceMaterial*>(material) != nullptr;
  };

  // Portals are shared between adjacent volumes, so the visitor can see a surface twice.
  std::unordered_set<const Acts::Surface*> designatedSurfaces;
  trackingGeometry->apply([&](Acts::Surface& surface) {
    if (isProtoMaterial(surface.surfaceMaterial()) && designatedSurfaces.insert(&surface).second &&
        logger().doPrint(Acts::Logging::DEBUG)) {
      std::ostringstream os;
      os << "Material surface " << surface.geometryId() << " " << surface.name()
         << " z=" << surface.center(getActsGeometryContext()).z() << " bounds=[";
      for (double value : surface.bounds().values()) {
        os << " " << value;
      }
      os << " ]";
      logger().log(Acts::Logging::DEBUG, os.str());
    }
  });
  logger().log(Acts::Logging::INFO, "Gen3 geometry designates " +
                                        std::to_string(designatedSurfaces.size()) +
                                        " surfaces for material mapping");

  if (m_cfg.materialDecorator) {
    logger().log(Acts::Logging::INFO, "Applying material decorator to Gen3 tracking geometry");
    std::size_t decoratedSurfaces    = 0;
    std::size_t surfacesWithMaterial = 0;
    try {
      // All volumes, and all surfaces including portals and passive surfaces
      trackingGeometry->apply(
          [&](Acts::TrackingVolume& volume) { m_cfg.materialDecorator->decorate(volume); });

      std::unordered_set<const Acts::Surface*> visitedSurfaces;
      trackingGeometry->apply([&](Acts::Surface& surface) {
        if (!visitedSurfaces.insert(&surface).second) {
          return;
        }
        m_cfg.materialDecorator->decorate(surface);
        ++decoratedSurfaces;
        if (const auto* material = surface.surfaceMaterial();
            material != nullptr && !isProtoMaterial(material)) {
          ++surfacesWithMaterial;
        }
      });
    } catch (const std::exception& e) {
      logger().log(Acts::Logging::ERROR,
                   std::string("Failed while applying material decorator in Gen3: ") + e.what());
      throw;
    }
    if (decoratedSurfaces == 0) {
      logger().log(
          Acts::Logging::ERROR,
          "Material map configured for Gen3, but no surfaces were available for decoration");
      throw std::runtime_error("Gen3 material decoration failed: no surfaces decorated");
    }
    logger().log(Acts::Logging::INFO, "Gen3 material decoration applied to " +
                                          std::to_string(decoratedSurfaces) + " surfaces (" +
                                          std::to_string(surfacesWithMaterial) +
                                          " with assigned material)");
    if (surfacesWithMaterial == 0) {
      // Material maps are keyed by GeometryIdentifier, so a map made for the Gen1 geometry
      // matches no Gen3 surfaces
      logger().log(Acts::Logging::WARNING,
                   "Material map assigned material to none of the Gen3 surfaces; tracking will "
                   "run without material. The map was likely produced for a different "
                   "geometry (e.g. Gen1); set acts:MaterialMap to a Gen3 map, or to an empty "
                   "value to disable material explicitly.");
    }
  }

  m_trackingGeometry = std::move(trackingGeometry);

  logger().log(Acts::Logging::INFO, "Blueprint tracking geometry constructed successfully");

  // Build the surface map
  buildSurfaceMap(m_trackingGeometry);

  // Optional: Visualization (can be enabled via config)
  // Acts::ObjVisualization3D vis{};
  // m_trackingGeometry->visualize(vis, getActsGeometryContext());
  // vis.write("acts_gen3_geometry.obj");
}

} // namespace eicrecon

#endif // HAS_GEN3_SUPPORT
