// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "SecondaryVertexFinder.h"
#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <JANA/Utils/JTypeInfo.h>
#include <JANA/Services/JGlobalRootLock.h>
#include <Acts/Definitions/Common.hpp>
#include <Acts/Definitions/Direction.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/GenericParticleHypothesis.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/TrackParameters.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#if Acts_VERSION_MAJOR >= 32
#include <Acts/Propagator/VoidNavigator.hpp>
#else
#include <Acts/Propagator/detail/VoidPropagatorComponents.hpp>
#endif
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/Result.hpp>
#include <Acts/Utilities/VectorHelpers.hpp>
#include <Acts/Vertexing/FullBilloirVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
#include "Acts/Vertexing/TrackDensityVertexFinder.hpp"
#include <Acts/Vertexing/ImpactPointEstimator.hpp>
#include <Acts/Vertexing/IterativeVertexFinder.hpp>
#include "Acts/Vertexing/AdaptiveMultiVertexFinder.hpp"
#include <Acts/Vertexing/AdaptiveMultiVertexFitter.hpp>
#include <Acts/Vertexing/Vertex.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include <Acts/Vertexing/ZScanVertexFinder.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <Acts/Vertexing/TrackAtVertex.hpp>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <algorithm>
#include <boost/container/vector.hpp>
#include <edm4eic/Cov4f.h>
#include <math.h>
#include <optional>
#include <tuple>
#include <utility>

#include "edm4eic/ReconstructedParticle.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/log/Log_service.h"
#include "services/rootfile/RootFile_service.h"
#include "extensions/spdlog/SpdlogToActs.h"

void eicrecon::SecondaryVertexFinder::init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
                                           std::shared_ptr<spdlog::logger> log) {

  m_log = log;

  m_geoSvc = geo_svc;

  m_BField =
      std::dynamic_pointer_cast<const eicrecon::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
  m_fieldctx = eicrecon::BField::BFieldVariant(m_BField);

/*
  std::string plugin_name = ("secvtx_studies");
  // ===============================================================================================
  // Get JANA application and seup general variables
  // ===============================================================================================
  auto app = GetApplication();
  //m_log = app->GetService<Log_service>()->logger(plugin_name);

  // Ask service locator for the DD4hep geometry
  auto dd4hep_service = app->GetService<DD4hep_service>();

  // Ask service locator a file to write histograms to
  auto root_file_service = app->GetService<RootFile_service>();

  // Get TDirectory for histograms root file
  auto globalRootLock = app->GetService<JGlobalRootLock>();
  globalRootLock->acquire_write_lock();
  auto *file = root_file_service->GetHistFile();
  globalRootLock->release_lock();

  // ===============================================================================================
  // Create a directory for this plugin. And subdirectories for series of histograms
  // ===============================================================================================
  m_dir_main = file->mkdir(plugin_name.c_str());

  // ===============================================================================================
  //  hists
  // ===============================================================================================
    m_hb_massPiPi1  = new TH1D("massPiPi1"," mass PiPi",100,200., 2000.);
    m_hb_massEE     = new TH1D("massEE"," massEE", 100,0., 200.);
    m_hb_nvrt2      = new TH1D("nvrt2"," vertices2", 50,0., 50.);
    m_hb_ratio      = new TH1D("ratio"," ratio", 51,0., 1.02);
    m_hb_totmass    = new TH1D("totmass"," totmass", 250,0., 10000.);
    m_hb_totmassEE  = new TH1D("massEEcomvrt"," totmass EE common vertex", 100,0., 1000.);
    m_hb_totmass2T0 = new TH1D("mass2trcomvrt0"," totmass 2tr common vertex", 800,0., 4000.);
    m_hb_totmass2T1 = new TH1D("mass2trcomvrt1"," totmass 2tr common vertex", 200,0., 10000.);
    m_hb_totmass2T2 = new TH1D("mass2trcomvrt2"," totmass 2tr common vertex", 200,0., 10000.);
    m_hb_impact     = new TH1D("impact", " impact", 100,0., 20.);
    m_hb_impactR    = new TH1D("impactR"," impactR", 400,-30., 70.);
    m_hb_impactZ    = new TH1D("impactZ"," impactZ", 100,-30., 70.);
*/
}

std::unique_ptr<edm4eic::VertexCollection> eicrecon::SecondaryVertexFinder::produce(
    const edm4eic::ReconstructedParticleCollection* recotracks,
    std::vector<const ActsExamples::Trajectories*> trajectories) {
/*
void eicrecon::SecondaryVertexFinder::produce(const Input& input,const Output& output)const{
  const auto [primvertex,tracks,recotracks,trajectories]=input;
  auto[amvprimvtx,amvsecvtx]=output;
*/

  auto primaryVertices = std::make_unique<edm4eic::VertexCollection>();
  auto outputVertices  = std::make_unique<edm4eic::VertexCollection>();

  ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("SVF", m_log));

  Acts::EigenStepper<> stepperSec(m_BField);

  // Set-up the propagator
  using PropagatorSec        = Acts::Propagator<Acts::EigenStepper<>>;
  using PropagatorOptionsSec = Acts::PropagatorOptions<>;

  // Set up propagator with void navigator
#if Acts_VERSION_MAJOR >= 32
  auto propagatorSec = std::make_shared<PropagatorSec>(stepperSec, Acts::VoidNavigator{},
                                                 logger().cloneWithSuffix("PropSec"));
#else
  auto propagatorSec = std::make_shared<PropagatorSec>(stepperSec, Acts::detail::VoidNavigator{},
                                                 logger().cloneWithSuffix("PropSec"));
#endif

  //set up Impact estimator
#if Acts_VERSION_MAJOR >= 33
  using ImpactPointEstimator = Acts::ImpactPointEstimator;
  using LinearizerSec = Acts::HelicalTrackLinearizer;
  using VertexFitterSec = Acts::AdaptiveMultiVertexFitter;
  using VertexSeederSec = Acts::TrackDensityVertexFinder;
  // The vertex finder type
  using VertexFinderSec = Acts::AdaptiveMultiVertexFinder;
  using VertexFinderOptionsSec = Acts::VertexingOptions;
#else
  using ImpactPointEstimator = Acts::ImpactPointEstimator<Acts::BoundTrackParameters, PropagatorSec>;
  using LinearizerSec = Acts::HelicalTrackLinearizer<PropagatorSec>;
  using VertexFitterSec = Acts::AdaptiveMultiVertexFitter<Acts::BoundTrackParameters, LinearizerSec>;
  using VertexSeederSec = Acts::Acts::ZScanVertexFinder<VertexFitterSec>;
  // The vertex finder type
  using VertexFinderSec = Acts::AdaptiveMultiVertexFinder<VertexFitterSec, VertexSeederSec>;
  using VertexFinderOptionsSec = Acts::VertexingOptions<Acts::BoundTrackParameters>;
#endif

  // using LinearizerSec = Acts::HelicalTrackLinearizer<PropagatorSec>;
  // Setup the track linearizer
  LinearizerSec::Config linearizerCfgSec(m_BField, propagatorSec);
  LinearizerSec linearizerSec(linearizerCfgSec, logger().cloneWithSuffix("HelLinSec"));

  //Trying multivertex fitter here...
  // Setup the vertex fitter
  ImpactPointEstimator::Config ipEstCfg(m_BField, propagatorSec);
  ImpactPointEstimator ipEst(ipEstCfg);
  VertexFitterSec::Config vertexFitterCfgSec(ipEst);
#if Acts_VERSION_MAJOR >= 33
  vertexFitterCfgSec.extractParameters
    .connect<&Acts::InputTrack::extractParameters>();
  vertexFitterCfgSec.trackLinearizer.connect<&LinearizerSec::linearizeTrack>(
    &linearizerSec);
#endif
  VertexFitterSec vertexFitterSec(vertexFitterCfgSec);

#if Acts_VERSION_MAJOR >= 33
  // Set up the vertex seed finder
  //VertexSeederSec::Config seedercfg(ipEst);
  std::shared_ptr<const Acts::IVertexFinder> seeder;
  Acts::GaussianTrackDensity::Config seederCfg;
  seederCfg.extractParameters
    .connect<&Acts::InputTrack::extractParameters>();
  seeder = std::make_shared<VertexSeederSec>(VertexSeederSec::Config{seederCfg});
#else
  // Set up the vertex seed finder
  VertexSeederSec::Config seedercfg(ipEst);
  VertexSeederSec seeder(seederCfg);
#endif


  VertexFinderSec::Config vertexfinderCfgSec(std::move(vertexFitterSec),
                              std::move(seeder), 
                              std::move(ipEst),
                              m_BField);

  // The vertex finder state
#if Acts_VERSION_MAJOR >= 31
  #if Acts_VERSION_MAJOR >= 33
  // Set the initial variance of the 4D vertex position. Since time is on a
  // numerical scale, we have to provide a greater value in the corresponding
  // dimension.
  vertexfinderCfgSec.initialVariances<<1e+2, 1e+2, 1e+2, 1e+8;
  // We do not want to use a beamspot constraint here
  //vertexfinderCfgSec.useBeamSpotConstraint = false;
  vertexfinderCfgSec.tracksMaxZinterval=35. * Acts::UnitConstants::mm;
  vertexfinderCfgSec.maxIterations=200;
  vertexfinderCfgSec.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  vertexFitterCfgSec.trackLinearizer.connect<&LinearizerSec::linearizeTrack>(&linearizerSec);
  #if Acts_VERSION_MAJOR >= 36
  vertexfinderCfgSec.field = m_BField;
  #else
  vertexfinderCfgSec.bField = std::dynamic_pointer_cast<Acts::MagneticFieldProvider>(
    std::const_pointer_cast<eicrecon::BField::DD4hepBField>(m_BField));
  #endif
  #endif
  VertexFinderSec vertexfinderSec(std::move(vertexfinderCfgSec)); //,logger().clone());
#else
  VertexFinder finder(std::move(vertexfinderCfgSec));
  typename VertexFinderSec::State stateSec(*m_BField, m_fieldctx);
#endif
  // Instantiate the finder
#if Acts_VERSION_MAJOR >= 33
  auto stateSec = vertexfinderSec.makeState(m_fieldctx);
#else
  typename Acts::IVertexFinder::State stateSec;
#endif

  VertexFinderOptionsSec vfOptions(m_geoctx, m_fieldctx);

#if Acts_VERSION_MAJOR >= 33
  std::vector<Acts::InputTrack> inputTracks;
#else
  std::vector<const Acts::BoundTrackParameters*> inputTrackPointersSecondary;
#endif
  //std::unique_ptr<eicrecon::TrackingSecUtilityTool> utilityTool(new eicrecon::TrackingSecUtilityTool());
  //utilityTool->calcPrimaryVtx(recotracks,trajectories,vertexfinderSec,vfOptions,vertexfinderCfgSec,stateSec);
  //Two Track Vertex fitting
  calcPrimaryVtx(recotracks,trajectories,vertexfinderSec,vfOptions,vertexfinderCfgSec,stateSec);
  for (std::vector<Acts::BoundTrackParameters>::size_type i = 0; i != trajectories.size(); i++) {
    auto tips = trajectories[i]->tips();
    if (tips.empty()) {
      continue;
    }
    std::cout << "Trajectory i: " << i << " has " << tips.size() << " tips" << std::endl;
    for(std::vector<Acts::BoundTrackParameters>::size_type j = i + 1;
        j != trajectories.size(); j++){
      auto tips2 = trajectories[j]->tips();
      if (tips2.empty()) {
        continue;
      }
      std::cout << "Trajectory j: " << j << " has " << tips2.size() << " tips" << std::endl;
      // Checking for default DCA cut-condition
      //secvtxGood=computeVtxcandidate(primvertex,tracks[i],tracks[j]);
      //if(!secvtxGood) continue;
      for (auto& tip : tips) {
        /// CKF can provide multiple track trajectories for a single input seed
#if Acts_VERSION_MAJOR >= 33
        inputTracks.emplace_back(&(trajectories[i]->trackParameters(tip)));
#else
        inputTrackPointersSecondary.push_back(&(trajectories[i]->trackParameters(tip)));
#endif
      }
      for(auto& tip : tips2){
#if Acts_VERSION_MAJOR >= 33
        inputTracks.emplace_back(&(trajectories[j]->trackParameters(tip)));
#else
        inputTrackPointersSecondary.push_back(&(trajectories[j]->trackParameters(tip)));
#endif
      }
      // run the vertex finder for both tracks
      std::cout << "Fitting secondary vertex" << std::endl;
#if Acts_VERSION_MAJOR >= 33
      std::vector<Acts::Vertex> verticesSec;
      auto resultSecondary = vertexfinderSec.find(inputTracks, vfOptions, stateSec);
#else
      auto resultSecondary =
          vertexfinderSec.find(inputTrackPointersSecondary, vfOptions, stateSec);
      std::cout << "Secondary vertex fit done" << std::endl;

      std::vector<Acts::Vertex<Acts::BoundTrackParameters>> verticesSec;
#endif
      if (resultSecondary.ok()) {
        std::cout << "Secondary vertex fit succeeded" << std::endl;
        verticesSec = std::move(resultSecondary.value());
      }else {
        std::cout << "Secondary vertex fit failed" << std::endl;
      }

  //utilityTool->write2screen();
      for (const auto& secvertex : verticesSec) {
        //std::cout<<"This is really from the secondary vertex tracking...\n"; std::abort();
        edm4eic::Cov4f cov(secvertex.fullCovariance()(0, 0), secvertex.fullCovariance()(1, 1),
                           secvertex.fullCovariance()(2, 2), secvertex.fullCovariance()(3, 3),
                           secvertex.fullCovariance()(0, 1), secvertex.fullCovariance()(0, 2),
                           secvertex.fullCovariance()(0, 3), secvertex.fullCovariance()(1, 2),
                           secvertex.fullCovariance()(1, 3), secvertex.fullCovariance()(2, 3));
        auto eicvertex = outputVertices->create();
        eicvertex.setType(0); // boolean flag if vertex is primary vertex of event
        eicvertex.setChi2((float)secvertex.fitQuality().first); // chi2
        eicvertex.setNdf((float)secvertex.fitQuality().second); // ndf
        eicvertex.setPosition({
            (float)secvertex.position().x(),
            (float)secvertex.position().y(),
            (float)secvertex.position().z(),
            (float)secvertex.time(),
        });                              // vtxposition
        eicvertex.setPositionError(cov); // covariance

        for(const auto& trk: secvertex.tracks()){
#if Acts_VERSION_MAJOR >= 33
          const auto& par=vertexfinderCfgSec.extractParameters(trk.originalParams);
#else 
          const auto& par=trk.originalParams;
#endif
        }//end for trk-loop
      }
#if Acts_VERSION_MAJOR >= 33
      // empty the vector for the next set of tracks
      inputTracks.clear();
#else
      inputTrackPointersSecondary.clear();
#endif
    }
  }

  return std::move(outputVertices);
}
