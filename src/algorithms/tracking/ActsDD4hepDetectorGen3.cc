// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

// This class is only available if ActsPlugins with DD4hep support is available,
// and has blueprint builder support (gen3).
#if __has_include(<ActsPlugins/DD4hep/BlueprintBuilder.hpp>)

#include "ActsDD4hepDetectorGen3.h"

#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/LayerBlueprintNode.hpp>
#include <Acts/Geometry/Blueprint.hpp>
#include <Acts/Geometry/BlueprintOptions.hpp>
#include <Acts/Geometry/BoundarySurfaceFace.hpp>
#include <Acts/Geometry/ContainerBlueprintNode.hpp>
#include <Acts/Geometry/CylinderVolumeBounds.hpp>
#include <Acts/Geometry/Extent.hpp>
#include <Acts/Geometry/Layer.hpp>
#include <Acts/Navigation/CylinderNavigationPolicy.hpp>
#include <Acts/Navigation/SurfaceArrayNavigationPolicy.hpp>
#include <Acts/Navigation/TryAllNavigationPolicy.hpp>
#include <Acts/Surfaces/SurfaceArray.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Visualization/ObjVisualization3D.hpp>
#include <ActsPlugins/DD4hep/BlueprintBuilder.hpp>
#include <ActsPlugins/DD4hep/DD4hepConversionHelpers.hpp>
#include <ActsPlugins/DD4hep/DD4hepDetectorElement.hpp>

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>

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
  using SrfArrayNavPol     = Acts::SurfaceArrayNavigationPolicy;

  //
  // DEFINE DETECTORS
  //

  // VertexBarrel
  auto VertexBarrel =
      makeLayerHelper()
          .barrel()
          .setAxes("XYZ")
          .setPattern("VertexBarrel_layer\\d")
          .setContainer("VertexBarrel")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(NavigationPolicyFactory{}
                                                  .add<CylinderNavigationPolicy>()
                                                  .add<TryAllNavigationPolicy>()
                                                  .asUniquePtr());
            return layer;
          })
          .build();
  VertexBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // SagittaSiBarrel
  // FIXME Volumes are not aligned: translation in x or y
  // Requires changing number of modules to multiple of 4
  auto SagittaSiBarrel =
      makeLayerHelper()
          .barrel()
          .setAxes("XYZ")
          .setPattern("SagittaSiBarrel_layer\\d")
          .setContainer("SagittaSiBarrel")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(NavigationPolicyFactory{}
                                                  .add<CylinderNavigationPolicy>()
                                                  .add<TryAllNavigationPolicy>()
                                                  .asUniquePtr());
            return layer;
          })
          .build();
  SagittaSiBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // OuterSiBarrel
  // FIXME Volumes are not aligned: translation in x or y
  // Requires changing number of modules to multiple of 4
  auto OuterSiBarrel =
      makeLayerHelper()
          .barrel()
          .setAxes("XYZ")
          .setPattern("OuterSiBarrel_layer\\d")
          .setContainer("OuterSiBarrel")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(NavigationPolicyFactory{}
                                                  .add<CylinderNavigationPolicy>()
                                                  .add<TryAllNavigationPolicy>()
                                                  .asUniquePtr());
            return layer;
          })
          .build();
  OuterSiBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // endcapPolicyFactory
  std::shared_ptr SiTrackerEndcapPolicyFactory = NavigationPolicyFactory{}
                                                     .add<CylinderNavigationPolicy>()
                                                     .add<TryAllNavigationPolicy>()
                                                     .asUniquePtr();

  // InnerTrackerEndcapP
  auto InnerTrackerEndcapP =
      makeLayerHelper()
          .endcap()
          .setAxes("XZY")
          .setPattern("InnerTrackerEndcapP_layer\\d_P")
          .setContainer("InnerTrackerEndcapP")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
            return layer;
          })
          .build();
  InnerTrackerEndcapP->setAttachmentStrategy(AttachmentStrategy::First);

  // InnerTrackerEndcapN
  auto InnerTrackerEndcapN =
      makeLayerHelper()
          .endcap()
          .setAxes("XZY")
          .setPattern("InnerTrackerEndcapN_layer\\d_N")
          .setContainer("InnerTrackerEndcapN")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
            return layer;
          })
          .build();
  InnerTrackerEndcapN->setAttachmentStrategy(AttachmentStrategy::First);

  // MiddleTrackerEndcapP
  auto MiddleTrackerEndcapP =
      makeLayerHelper()
          .endcap()
          .setAxes("XZY")
          .setPattern("MiddleTrackerEndcapP_layer\\d_P")
          .setContainer("MiddleTrackerEndcapP")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
            return layer;
          })
          .build();
  MiddleTrackerEndcapP->setAttachmentStrategy(AttachmentStrategy::First);

  // MiddleTrackerEndcapN
  auto MiddleTrackerEndcapN =
      makeLayerHelper()
          .endcap()
          .setAxes("XZY")
          .setPattern("MiddleTrackerEndcapN_layer\\d_N")
          .setContainer("MiddleTrackerEndcapN")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
            return layer;
          })
          .build();
  MiddleTrackerEndcapN->setAttachmentStrategy(AttachmentStrategy::First);

  // OuterTrackerEndcapP
  auto OuterTrackerEndcapP =
      makeLayerHelper()
          .endcap()
          .setAxes("XZY")
          .setPattern("OuterTrackerEndcapP_layer\\d_P")
          .setContainer("OuterTrackerEndcapP")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
            return layer;
          })
          .build();
  OuterTrackerEndcapP->setAttachmentStrategy(AttachmentStrategy::First);

  // OuterTrackerEndcapN
  auto OuterTrackerEndcapN =
      makeLayerHelper()
          .endcap()
          .setAxes("XZY")
          .setPattern("OuterTrackerEndcapN_layer\\d_N")
          .setContainer("OuterTrackerEndcapN")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(SiTrackerEndcapPolicyFactory);
            return layer;
          })
          .build();
  OuterTrackerEndcapN->setAttachmentStrategy(AttachmentStrategy::First);

  // ForwardMPGD
  std::shared_ptr ForwardMPGDPolicyFactory = NavigationPolicyFactory{}
                                                 .add<CylinderNavigationPolicy>()
                                                 .add<TryAllNavigationPolicy>()
                                                 .asUniquePtr();
  auto ForwardMPGD =
      makeLayerHelper()
          .endcap()
          .setAxes("XZY")
          .setPattern("ForwardMPGD_layer\\d_P")
          .setContainer("ForwardMPGD")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(ForwardMPGDPolicyFactory);
            return layer;
          })
          .build();
  ForwardMPGD->setAttachmentStrategy(AttachmentStrategy::First);

  // BackwardMPGD
  std::shared_ptr BackwardMPGDPolicyFactory = NavigationPolicyFactory{}
                                                  .add<CylinderNavigationPolicy>()
                                                  .add<TryAllNavigationPolicy>()
                                                  .asUniquePtr();
  auto BackwardMPGD =
      makeLayerHelper()
          .endcap()
          .setAxes("XZY")
          .setPattern("BackwardMPGD_layer\\d_N")
          .setContainer("BackwardMPGD")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(BackwardMPGDPolicyFactory);
            return layer;
          })
          .build();
  BackwardMPGD->setAttachmentStrategy(AttachmentStrategy::First);

  // InnerMPGDBarrel
  auto InnerMPGDBarrel =
      makeLayerHelper()
          .barrel()
          .setAxes("XYZ")
          .setPattern("InnerMPGDBarrel_layer\\d")
          .setContainer("InnerMPGDBarrel")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(NavigationPolicyFactory{}
                                                  .add<CylinderNavigationPolicy>()
                                                  .add<TryAllNavigationPolicy>()
                                                  .asUniquePtr());
            return layer;
          })
          .build();
  InnerMPGDBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // BarrelTOF
  auto BarrelTOF =
      makeLayerHelper()
          .barrel()
          .setAxes("XYZ")
          .setPattern("BarrelTOF_layer\\d")
          .setContainer("BarrelTOF")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(NavigationPolicyFactory{}
                                                  .add<CylinderNavigationPolicy>()
                                                  .add<TryAllNavigationPolicy>()
                                                  .asUniquePtr());
            return layer;
          })
          .build();
  BarrelTOF->setAttachmentStrategy(AttachmentStrategy::First);

  // MPGDOuterBarrel
  auto MPGDOuterBarrel =
      makeLayerHelper()
          .barrel()
          .setAxes("XYZ")
          .setPattern("MPGDOuterBarrel_layer\\d")
          .setContainer("MPGDOuterBarrel")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(NavigationPolicyFactory{}
                                                  .add<CylinderNavigationPolicy>()
                                                  .add<TryAllNavigationPolicy>()
                                                  .asUniquePtr());
            return layer;
          })
          .build();
  MPGDOuterBarrel->setAttachmentStrategy(AttachmentStrategy::First);

  // ForwardTOF
  // FIXME Volumes are not aligned: translation in x or y
  std::shared_ptr ForwardTOFPolicyFactory = NavigationPolicyFactory{}
                                                .add<CylinderNavigationPolicy>()
                                                .add<TryAllNavigationPolicy>()
                                                .asUniquePtr();
  auto ForwardTOF =
      makeLayerHelper()
          .endcap()
          .setAxes("XZY")
          .setPattern("ForwardTOF_layer1")
          //.setPattern("ForwardTOF_layer\\d")
          // ^ FIXME LayerBlueprintNode: no surfaces provided for ForwardTOF_layer2
          .setContainer("ForwardTOF")
          .setEnvelope(Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&,
                         std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setUseCenterOfGravity(false, false, true);
            layer->setNavigationPolicyFactory(ForwardTOFPolicyFactory);
            return layer;
          })
          .build();
  ForwardTOF->setAttachmentStrategy(AttachmentStrategy::First);

  // B0Tracker (OFF AXIS)
  // FIXME VolumeStack requires at least one volume
  /*
  std::shared_ptr B0TrackerPolicyFactory =
      NavigationPolicyFactory{}
          .add<CylinderNavigationPolicy>()
          .add<TryAllNavigationPolicy>()
          .asUniquePtr();
  auto B0Tracker =
      makeLayerHelper()
          .endcap()
          .setAxes("XZY")
          .setPattern("B0Tracker_layer\\d")
          .setContainer("B0Tracker")
          .setEnvelope(
            Acts::ExtentEnvelope{}.set(AxisZ, {5_mm, 5_mm}).set(AxisR, {5_mm, 5_mm}))
          .customize([&](const dd4hep::DetElement&, std::shared_ptr<Acts::Experimental::LayerBlueprintNode> layer) {
            layer->setNavigationPolicyFactory(B0TrackerPolicyFactory);
            return layer;
          })
          .build();
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
