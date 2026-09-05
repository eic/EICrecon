// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

// This class is only available if ActsPlugins with DD4hep support is available,
// and has blueprint builder support (gen3).
#if __has_include(<ActsPlugins/DD4hep/BlueprintBuilder.hpp>)

#include "ActsDD4hepDetectorGen3.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/Blueprint.hpp>
#include <Acts/Geometry/BlueprintBuilder.hpp>
#include <Acts/Geometry/BlueprintOptions.hpp>
#include <Acts/Geometry/ContainerBlueprintNode.hpp>
#include <Acts/Geometry/CylinderVolumeBounds.hpp>
#include <Acts/Geometry/Extent.hpp>
#include <Acts/Geometry/LayerBlueprintNode.hpp>
#include <Acts/Geometry/NavigationPolicyFactory.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Geometry/TrackingVolume.hpp>
#include <Acts/Geometry/VolumeAttachmentStrategy.hpp>
#include <Acts/Material/IMaterialDecorator.hpp>
#include <Acts/Navigation/CylinderNavigationPolicy.hpp>
#include <Acts/Navigation/TryAllNavigationPolicy.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <ActsPlugins/DD4hep/BlueprintBuilder.hpp>
#include <ActsPlugins/DD4hep/DD4hepDetectorElement.hpp>
#include <DD4hep/DetElement.h>
#include <algorithm>
#include <cstddef>
#include <exception>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
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

    template <typename CustomizerT> LayerHelperCompat&& customize(CustomizerT customizer) && {
      m_layers = std::move(m_layers).onLayer(
          [c = std::move(customizer)](const std::optional<dd4hep::DetElement>& element,
                                      Acts::Experimental::LayerBlueprintNode& layer) mutable {
            if (!element.has_value()) {
              throw std::runtime_error("Layer customizer requires a source dd4hep::DetElement");
            }
            auto nonOwning = std::shared_ptr<Acts::Experimental::LayerBlueprintNode>(
                &layer, [](Acts::Experimental::LayerBlueprintNode*) {});
            if constexpr (std::is_void_v<decltype(c(*element, nonOwning))>) {
              c(*element, nonOwning);
            } else {
              (void)c(*element, nonOwning);
            }
          });
      return std::move(*this);
    }

    std::shared_ptr<Acts::Experimental::ContainerBlueprintNode> build() && {
      return std::move(m_layers).build();
    }

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
  };

} // namespace

void ActsDD4hepDetectorGen3::construct() {
  using namespace Acts::Experimental;
  using namespace Acts;
  using namespace Acts::UnitLiterals;
  using enum AxisDirection;

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

  Acts::Experimental::Blueprint::Config cfg;
  cfg.envelope[AxisZ] = {20_mm, 20_mm};
  cfg.envelope[AxisR] = {0_mm, 20_mm};
  Acts::Experimental::Blueprint root{cfg};

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
          .customize(std::forward<decltype(customizer)>(customizer))
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
      buildLayer({.kind       = LayerKind::Barrel,
                  .sensorAxes = "XYZ",
                  .layerAxes  = std::nullopt,
                  .pattern    = "VertexBarrel_layer\\d",
                  .container  = "VertexBarrel",
                  .label      = "VertexBarrel"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                   return layer;
                 });
  VertexBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // SagittaSiBarrel: stave-based barrel, sensor axes only (no layer axes to
  // avoid extracting off-axis layer transforms from individual module placements).
  // Use center-of-gravity only in Z so x/y are forced to 0 for CylinderVolumeStack.
  auto SagittaSiBarrel =
      buildLayer({.kind       = LayerKind::Barrel,
                  .sensorAxes = "XYZ",
                  .layerAxes  = std::nullopt,
                  .pattern    = "SagittaSiBarrel_layer\\d",
                  .container  = "SagittaSiBarrel",
                  .label      = "SagittaSiBarrel"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setUseCenterOfGravity(false, false, true);
                   layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                   return layer;
                 });
  SagittaSiBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // OuterSiBarrel: stave-based barrel, sensor axes only (no layer axes).
  // Use center-of-gravity only in Z so x/y are forced to 0 for CylinderVolumeStack.
  auto OuterSiBarrel =
      buildLayer({.kind       = LayerKind::Barrel,
                  .sensorAxes = "XYZ",
                  .layerAxes  = std::nullopt,
                  .pattern    = "OuterSiBarrel_layer\\d",
                  .container  = "OuterSiBarrel",
                  .label      = "OuterSiBarrel"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setUseCenterOfGravity(false, false, true);
                   layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                   return layer;
                 });
  OuterSiBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // endcapPolicyFactory
  std::shared_ptr SiTrackerEndcapPolicyFactory = NavigationPolicyFactory{}
                                                     .add<CylinderNavigationPolicy>()
                                                     .add<TryAllNavigationPolicy>()
                                                     .asUniquePtr();

  // InnerTrackerEndcapP
  auto InnerTrackerEndcapP =
      buildLayer({.kind       = LayerKind::Endcap,
                  .sensorAxes = "XZY",
                  .layerAxes  = std::nullopt,
                  .pattern    = "InnerTrackerEndcapP_layer\\d_P",
                  .container  = "InnerTrackerEndcapP",
                  .label      = "InnerTrackerEndcapP"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  InnerTrackerEndcapP->setAttachmentStrategy(AttachmentStrategy::First);

  // InnerTrackerEndcapN
  auto InnerTrackerEndcapN =
      buildLayer({.kind       = LayerKind::Endcap,
                  .sensorAxes = "XZY",
                  .layerAxes  = std::nullopt,
                  .pattern    = "InnerTrackerEndcapN_layer\\d_N",
                  .container  = "InnerTrackerEndcapN",
                  .label      = "InnerTrackerEndcapN"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  InnerTrackerEndcapN->setAttachmentStrategy(AttachmentStrategy::First);

  // MiddleTrackerEndcapP
  auto MiddleTrackerEndcapP =
      buildLayer({.kind       = LayerKind::Endcap,
                  .sensorAxes = "XZY",
                  .layerAxes  = std::nullopt,
                  .pattern    = "MiddleTrackerEndcapP_layer\\d_P",
                  .container  = "MiddleTrackerEndcapP",
                  .label      = "MiddleTrackerEndcapP"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  MiddleTrackerEndcapP->setAttachmentStrategy(AttachmentStrategy::First);

  // MiddleTrackerEndcapN
  auto MiddleTrackerEndcapN =
      buildLayer({.kind       = LayerKind::Endcap,
                  .sensorAxes = "XZY",
                  .layerAxes  = std::nullopt,
                  .pattern    = "MiddleTrackerEndcapN_layer\\d_N",
                  .container  = "MiddleTrackerEndcapN",
                  .label      = "MiddleTrackerEndcapN"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  MiddleTrackerEndcapN->setAttachmentStrategy(AttachmentStrategy::First);

  // OuterTrackerEndcapP
  auto OuterTrackerEndcapP =
      buildLayer({.kind       = LayerKind::Endcap,
                  .sensorAxes = "XZY",
                  .layerAxes  = std::nullopt,
                  .pattern    = "OuterTrackerEndcapP_layer\\d_P",
                  .container  = "OuterTrackerEndcapP",
                  .label      = "OuterTrackerEndcapP"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  OuterTrackerEndcapP->setAttachmentStrategy(AttachmentStrategy::First);

  // OuterTrackerEndcapN
  auto OuterTrackerEndcapN =
      buildLayer({.kind       = LayerKind::Endcap,
                  .sensorAxes = "XZY",
                  .layerAxes  = std::nullopt,
                  .pattern    = "OuterTrackerEndcapN_layer\\d_N",
                  .container  = "OuterTrackerEndcapN",
                  .label      = "OuterTrackerEndcapN"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
                   return layer;
                 });
  OuterTrackerEndcapN->setAttachmentStrategy(AttachmentStrategy::First);

  // ForwardMPGD
  std::shared_ptr ForwardMPGDPolicyFactory = NavigationPolicyFactory{}
                                                 .add<CylinderNavigationPolicy>()
                                                 .add<TryAllNavigationPolicy>()
                                                 .asUniquePtr();
  auto ForwardMPGD = buildLayer({.kind       = LayerKind::Endcap,
                                 .sensorAxes = "XZY",
                                 .layerAxes  = std::nullopt,
                                 .pattern    = "ForwardMPGD_layer\\d_P",
                                 .container  = "ForwardMPGD",
                                 .label      = "ForwardMPGD"},
                                [&](const dd4hep::DetElement&,
                                    std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                                  layer->setNavigationPolicyFactory(ForwardMPGDPolicyFactory);
                                  return layer;
                                });
  ForwardMPGD->setAttachmentStrategy(AttachmentStrategy::First);

  // BackwardMPGD
  std::shared_ptr BackwardMPGDPolicyFactory = NavigationPolicyFactory{}
                                                  .add<CylinderNavigationPolicy>()
                                                  .add<TryAllNavigationPolicy>()
                                                  .asUniquePtr();
  auto BackwardMPGD =
      buildLayer({.kind       = LayerKind::Endcap,
                  .sensorAxes = "XZY",
                  .layerAxes  = std::nullopt,
                  .pattern    = "BackwardMPGD_layer\\d_N",
                  .container  = "BackwardMPGD",
                  .label      = "BackwardMPGD"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(BackwardMPGDPolicyFactory);
                   return layer;
                 });
  BackwardMPGD->setAttachmentStrategy(AttachmentStrategy::First);

  // InnerMPGDBarrel
  auto InnerMPGDBarrel =
      buildLayer({.kind       = LayerKind::Barrel,
                  .sensorAxes = "XYZ",
                  .layerAxes  = std::nullopt,
                  .pattern    = "InnerMPGDBarrel_layer\\d",
                  .container  = "InnerMPGDBarrel",
                  .label      = "InnerMPGDBarrel"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                   return layer;
                 });
  InnerMPGDBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // BarrelTOF
  auto BarrelTOF = buildLayer({.kind       = LayerKind::Barrel,
                               .sensorAxes = "XYZ",
                               .layerAxes  = std::nullopt,
                               .pattern    = "BarrelTOF_layer\\d",
                               .container  = "BarrelTOF",
                               .label      = "BarrelTOF"},
                              [&](const dd4hep::DetElement&,
                                  std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                                layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                                return layer;
                              });
  BarrelTOF->setAttachmentStrategy(AttachmentStrategy::First);

  // MPGDOuterBarrel
  auto MPGDOuterBarrel =
      buildLayer({.kind       = LayerKind::Barrel,
                  .sensorAxes = "XYZ",
                  .layerAxes  = std::nullopt,
                  .pattern    = "MPGDOuterBarrel_layer\\d",
                  .container  = "MPGDOuterBarrel",
                  .label      = "MPGDOuterBarrel"},
                 [&](const dd4hep::DetElement&,
                     std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
                   layer->setNavigationPolicyFactory(makeBarrelPolicyFactory());
                   return layer;
                 });
  MPGDOuterBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // ForwardTOF: use emptyOk so non-sensitive structural layers (support, gap) are
  // silently skipped instead of causing "no surfaces provided" failures.
  // ForwardTOF has layers 1-6; only layers 1 and 2 carry sensitive surfaces.
  std::shared_ptr ForwardTOFPolicyFactory = NavigationPolicyFactory{}
                                                .add<CylinderNavigationPolicy>()
                                                .add<TryAllNavigationPolicy>()
                                                .asUniquePtr();
  auto ForwardTOF = buildLayer({.kind       = LayerKind::Endcap,
                                .sensorAxes = "XZY",
                                .layerAxes  = std::nullopt,
                                .pattern    = "ForwardTOF_layer\\d",
                                .container  = "ForwardTOF",
                                .label      = "ForwardTOF",
                                .emptyOk    = true},
                               [&](const dd4hep::DetElement&,
                                   std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
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
                                  std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
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
  root.addCylinderContainer("Tracker4", AxisZ, [&](auto& tracker4) {
    tracker4.addCylinderContainer("Tracker3", AxisR, [&](auto& tracker3) {
      tracker3.addStaticVolume(Transform3::Identity(),
                               std::make_unique<CylinderVolumeBounds>(0_mm, 20_mm, 100_mm),
                               "Beampipe");
      tracker3.addCylinderContainer("Tracker2", AxisZ, [&](auto& tracker2) {
        tracker2.addChild(BackwardMPGD);         // r=[65–405], z=[−1462,−1324]
        tracker2.addChild(OuterTrackerEndcapN);  // r=[32–426], z=[−1275,−895]
        tracker2.addChild(MiddleTrackerEndcapN); // r=[32-420], z ~ -450
        tracker2.addCylinderContainer("Tracker1", AxisR, [&](auto& tracker1) {
          tracker1.addCylinderContainer("Tracker0", AxisZ, [&](auto& tracker0) {
            tracker0.addChild(InnerTrackerEndcapN); // r=[32-245], z ~ -250
            tracker0.addChild(VertexBarrel);        // r=[33–130], z=[−135,+135]
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

  m_trackingGeometry = root.construct(options, getActsGeometryContext(), logger());

  if (!m_trackingGeometry) {
    logger().log(Acts::Logging::ERROR, "Failed to construct tracking geometry from blueprint");
    throw std::runtime_error("Blueprint construction failed");
  }

  if (m_cfg.materialDecorator) {
    logger().log(Acts::Logging::INFO, "Applying material decorator to Gen3 tracking geometry");
    std::size_t decoratedSurfaces    = 0;
    std::size_t surfacesWithMaterial = 0;
    try {
      if (auto* world =
              const_cast<Acts::TrackingVolume*>(m_trackingGeometry->highestTrackingVolume());
          world != nullptr) {
        m_cfg.materialDecorator->decorate(*world);
      }

      m_trackingGeometry->visitSurfaces([&](const Acts::Surface* surface) {
        if (surface == nullptr) {
          return;
        }
        auto* mutableSurface = const_cast<Acts::Surface*>(surface);
        m_cfg.materialDecorator->decorate(*mutableSurface);
        ++decoratedSurfaces;
        if (mutableSurface->surfaceMaterial() != nullptr) {
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
  }

  logger().log(Acts::Logging::INFO, "Blueprint tracking geometry constructed successfully");

  // Build the surface map
  buildSurfaceMap(m_trackingGeometry);

  // Optional: Visualization (can be enabled via config)
  // Acts::ObjVisualization3D vis{};
  // m_trackingGeometry->visualize(vis, getActsGeometryContext());
  // vis.write("acts_gen3_geometry.obj");
}

} // namespace eicrecon

#endif // __has_include(<ActsPlugins/DD4hep/BlueprintBuilder.hpp>)
