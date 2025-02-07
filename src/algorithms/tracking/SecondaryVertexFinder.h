// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <algorithms/algorithm.h>
//ROOT headers
#include <TH1D.h>
#include <TH2D.h>
#include <TLorentzVector.h>
#include <TDirectory.h>
#include <TVector3.h>
#include <TMath.h>
#include <TProfile.h>

#include "Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp"
#include "Acts/Plugins/DD4hep/DD4hepFieldAdapter.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/BinningType.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Vertexing/ImpactPointEstimator.hpp"

#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/Vertex.h>
#include <edm4eic/unit_system.h>
#include <edm4eic/TrackParametersCollection.h>
#include <spdlog/logger.h>

#include "ActsExamples/EventData/Trajectories.hpp"
#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "SecondaryVertexFinderConfig.h"
#include "IterativeVertexFinder.h"
#include "algorithms/interfaces/WithPodConfig.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <JANA/Services/JGlobalRootLock.h>
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/log/Log_service.h"
#include "services/rootfile/RootFile_service.h"
#include "TrackingSecUtilityTool.h"

namespace eicrecon {
class SecondaryVertexFinder
    : public eicrecon::WithPodConfig<eicrecon::SecondaryVertexFinderConfig>{
public:
  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> log);
/*
  void produce(const Input&,const Output&) const;
  produce(std::vector<const edm4eic::Vertex*>,
          const edm4eic::ReconstructedParticleCollection*,
          std::vector<const ActsExamples::Trajectories*> trajectories);
*/
  //std::unique_ptr<edm4eic::VertexCollection>
  //std::tuple<std::unique_ptr<edm4eic::TrackCollection>,std::unique_ptr<edm4eic::VertexCollection>>
  std::tuple<std::unique_ptr<edm4eic::VertexCollection>,std::unique_ptr<edm4eic::VertexCollection>>
  produce(const edm4eic::ReconstructedParticleCollection*,
          std::vector<const ActsExamples::Trajectories*> trajectories);

  // Calculate an initial Primary Vertex
  std::unique_ptr<edm4eic::VertexCollection>
  calcPrimaryVtx(const edm4eic::ReconstructedParticleCollection*,
          std::vector<const ActsExamples::Trajectories*> trajectories,Acts::AdaptiveMultiVertexFinder&,
          Acts::VertexingOptions,Acts::AdaptiveMultiVertexFinder::Config&,Acts::IVertexFinder::State&);

  //Calculate secondary vertex and store secVtx container
  std::unique_ptr<edm4eic::VertexCollection>
  calcSecVtx(const edm4eic::ReconstructedParticleCollection*,
          std::vector<const ActsExamples::Trajectories*> trajectories,Acts::AdaptiveMultiVertexFinder&,
          Acts::VertexingOptions,Acts::AdaptiveMultiVertexFinder::Config&,Acts::IVertexFinder::State&,
          std::vector<Acts::Vertex>);

  // Functions to be used to check efficacy of sec. vertex 
  std::unique_ptr<edm4eic::VertexCollection>
  getSecVtx(const edm4eic::Vertex*,const TLorentzVector&,
            const edm4eic::TrackParameters,
            const edm4eic::TrackParameters,std::vector<double>&);

  // Inline functions to calculate the AMVF primary vertex
  inline void setprmvtx(std::vector<Acts::Vertex> vtx){prmvtx=vtx;};
  inline std::vector<Acts::Vertex> getprmvtx(){return prmvtx;};
private:

  const double m_massPi =139.5702;
  const double m_massP  =938.272;
  const double m_massE  =  0.511;
  const double m_massK0 =497.648;
  const double m_massLam=1115.683;
  const double m_massD0 =1864.84;
  const double m_massB  =5279.400;

  // gives correct mass assigment in case of nonequal masses
  static double massV0(std::vector<std::vector<double>>& trkAtVrt, double massP, double massPi);
  TLorentzVector totalMom(const std::vector<const ActsExamples::Trajectories*> &intrk) const;
  TLorentzVector momAtVrt(const std::vector<double>& inpTrk) const;

  //Track paramters to calculate DCA and PCA
  TVector3 distanceR;
  std::vector<Acts::Vertex>prmvtx;
  bool secvtxGood=false;
  double trackA_a,trackA_b;
  double trackB_a,trackB_b;
  double vtxA_x,vtxA_y;
  double vtxB_x,vtxB_y;
  double deltaxy_A,deltaA_x,deltaA_y;
  double deltaxy_B,deltaB_x,deltaB_y;
  double minR=0.05;

  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::MagneticFieldContext m_fieldctx;
  SecondaryVertexFinderConfig m_cfg;

};

std::unique_ptr<edm4eic::VertexCollection>
SecondaryVertexFinder::calcPrimaryVtx(
    const edm4eic::ReconstructedParticleCollection* reconParticles,
    std::vector<const ActsExamples::Trajectories*> trajectories,Acts::AdaptiveMultiVertexFinder& finder,
    Acts::VertexingOptions finderOpts, Acts::AdaptiveMultiVertexFinder::Config& finderCfg,
    Acts::IVertexFinder::State& state){

  auto prmVertices = std::make_unique<edm4eic::VertexCollection>();
  std::vector<TVector3> tempvtx;
#if Acts_VERSION_MAJOR >= 33
  std::vector<Acts::InputTrack> inputTracks;
#else
  std::vector<const Acts::BoundTrackParameters*> inputTrackPointers;
#endif

  for (const auto& trajectory : trajectories){
    auto tips = trajectory->tips();
    if (tips.empty()){
      continue;
    }
    /// CKF can provide multiple track trajectories for a single input seed
    for (auto& tip : tips) {
      ActsExamples::TrackParameters par = trajectory->trackParameters(tip);

#if Acts_VERSION_MAJOR >= 33
      inputTracks.emplace_back(&(trajectory->trackParameters(tip)));
#else
      inputTrackPointers.push_back(&(trajectory->trackParameters(tip)));
#endif
      m_log->trace("Track local position at input = {} mm, {} mm", par.localPosition().x() / Acts::UnitConstants::mm, par.localPosition().y() / Acts::UnitConstants::mm);
    }
  }

#if Acts_VERSION_MAJOR >= 33
  std::vector<Acts::Vertex> vertices;
  auto result = finder.find(inputTracks, finderOpts, state);
#else
  std::vector<Acts::Vertex<Acts::BoundTrackParameters>> vertices;
  auto result = finder.find(inputTrackPointers, finderOpts, state);
#endif
  if (result.ok()){
    vertices = std::move(result.value());
    setprmvtx(vertices);
  }
  std::cout<<"**** Here we go Primary Vertex Multiplicity: "<<getprmvtx().size()<<" : "<<vertices.size()<<"\n";
  // -----> Fix: Use for later
  for (const auto& vtx : vertices) {
    edm4eic::Cov4f cov(vtx.fullCovariance()(0,0), vtx.fullCovariance()(1,1), vtx.fullCovariance()(2,2), vtx.fullCovariance()(3,3),
                       vtx.fullCovariance()(0,1), vtx.fullCovariance()(0,2), vtx.fullCovariance()(0,3),
                       vtx.fullCovariance()(1,2), vtx.fullCovariance()(1,3),
                       vtx.fullCovariance()(2,3));
    auto eicvertex = prmVertices->create();
    eicvertex.setType(1);                                  // boolean flag if vertex is primary vertex of event
    eicvertex.setChi2((float)vtx.fitQuality().first);      // chi2
    eicvertex.setNdf((float)vtx.fitQuality().second);      // ndf
    eicvertex.setPosition({
         (float)vtx.position().x(),
         (float)vtx.position().y(),
         (float)vtx.position().z(),
         (float)vtx.time(),
    }); // vtxposition
    eicvertex.setPositionError(cov);                          // covariance

/*
    for (const auto& t : vtx.tracks()){
#if Acts_VERSION_MAJOR >= 33
      const auto& par = finderCfg.extractParameters(t.originalParams);
#else
      const auto& par = *t.originalParams;
#endif
      m_log->trace("Track local position from vertex = {} mm, {} mm", par.localPosition().x() / Acts::UnitConstants::mm, par.localPosition().y() / Acts::UnitConstants::mm);
      float loc_a = par.localPosition().x();
      float loc_b = par.localPosition().y();

      for (const auto& part : *reconParticles) {
        const auto& tracks = part.getTracks();
        for (const auto trk : tracks) {
          const auto& traj = trk.getTrajectory();
          const auto& trkPars = traj.getTrackParameters();
          for (const auto par : trkPars) {
            const double EPSILON = 1.0e-4; // mm
            if (fabs((par.getLoc().a / edm4eic::unit::mm) - (loc_a / Acts::UnitConstants::mm)) < EPSILON
              && fabs((par.getLoc().b / edm4eic::unit::mm) - (loc_b / Acts::UnitConstants::mm)) < EPSILON) {
              m_log->trace("From ReconParticles, track local position [Loc a, Loc b] = {} mm, {} mm", par.getLoc().a / edm4eic::unit::mm, par.getLoc().b / edm4eic::unit::mm);
              eicvertex.addToAssociatedParticles(part);
            } // endif
          } // end for par
        } // end for trk
      } // end for part
    } // end for t
    m_log->debug("One vertex found at (x,y,z) = ({}, {}, {}) mm.", vtx.position().x() / Acts::UnitConstants::mm, vtx.position().y() / Acts::UnitConstants::mm, vtx.position().z() / Acts::UnitConstants::mm);

*/
  } // end for vtx
  return std::move(prmVertices);
}

std::unique_ptr<edm4eic::VertexCollection>
SecondaryVertexFinder::calcSecVtx(
    const edm4eic::ReconstructedParticleCollection* reconParticles,
    std::vector<const ActsExamples::Trajectories*> trajectories,Acts::AdaptiveMultiVertexFinder& vertexfinderSec,
    Acts::VertexingOptions vfOptions, Acts::AdaptiveMultiVertexFinder::Config& vertexfinderCfgSec,
    Acts::IVertexFinder::State& stateSec,std::vector<Acts::Vertex> prmvtx){

  std::cout<<"==========> primvtx.size = "<<prmvtx.size()<<"\n";
  auto secVertices = std::make_unique<edm4eic::VertexCollection>();
#if Acts_VERSION_MAJOR >= 33
  std::vector<Acts::InputTrack> inputTracks;
#else
  std::vector<const Acts::BoundTrackParameters*> inputTrackPointersSecondary;
#endif
  for(unsigned int i = 0; i < trajectories.size(); i++) {
    auto tips = trajectories[i]->tips();
    if (tips.empty()) {
      continue;
    }
    std::cout << "Trajectory i: " << i << " has " << tips.size() << " tips" << std::endl;
    for(unsigned int j=i+1; j < trajectories.size(); j++){
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
      for (const auto& secvertex : verticesSec) {
        //std::cout<<"This is really from the secondary vertex tracking...\n"; std::abort();
        edm4eic::Cov4f cov(secvertex.fullCovariance()(0, 0), secvertex.fullCovariance()(1, 1),
                           secvertex.fullCovariance()(2, 2), secvertex.fullCovariance()(3, 3),
                           secvertex.fullCovariance()(0, 1), secvertex.fullCovariance()(0, 2),
                           secvertex.fullCovariance()(0, 3), secvertex.fullCovariance()(1, 2),
                           secvertex.fullCovariance()(1, 3), secvertex.fullCovariance()(2, 3));
        auto eicvertex = secVertices->create();
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

/*
        for(const auto& trk: secvertex.tracks()){
#if Acts_VERSION_MAJOR >= 33
          const auto& par=vertexfinderCfgSec.extractParameters(trk.originalParams);
#else
          const auto& par=trk.originalParams;
#endif
        }//end for trk-loop
*/
      }
#if Acts_VERSION_MAJOR >= 33
      // empty the vector for the next set of tracks
      inputTracks.clear();
#else
      inputTrackPointersSecondary.clear();
#endif
    } //end of int j=i+1
  } // end of int i=0; i<trajectories.size()
  return secVertices;
}
} // namespace eicrecon
