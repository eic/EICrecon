// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <vector>
#include <memory>
#include <algorithm>
//ROOT headers
#include <TH1D.h>
#include <TH2D.h>
#include <TLorentzVector.h>
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
#include <edm4eic/Vertex.h>
#include <edm4eic/TrackParametersCollection.h>
#include <spdlog/logger.h>

#include "ActsExamples/EventData/Trajectories.hpp"
#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "SecondaryVertexFinderConfig.h"
#include "IterativeVertexFinder.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {
class SecondaryVertexFinder
    : public eicrecon::WithPodConfig<eicrecon::SecondaryVertexFinderConfig> {
public:
  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> log);
  std::unique_ptr<edm4eic::VertexCollection>
  produce(std::vector<const edm4eic::Vertex*>,
          const edm4eic::TrackParametersCollection*,
          const edm4eic::ReconstructedParticleCollection*,
          std::vector<const ActsExamples::Trajectories*> trajectories);

  // Calculate an initial Primary Vertex
  std::unique_ptr<edm4eic::VertexCollection>
  calcPrimaryVtx(const edm4eic::ReconstructedParticleCollection*,
          std::vector<const ActsExamples::Trajectories*> trajectories);

  //Calculate secondary vertex and store secVtx container
  std::unique_ptr<edm4eic::VertexCollection>
  calcSecVtx(const edm4eic::ReconstructedParticleCollection*,
          std::vector<const ActsExamples::Trajectories*> trajectries);
                                                                   // Functions to be used to check efficacy of sec. vertex         bool computeVtxcandidate(const edm4eic::Vertex*,                                          const edm4eic::TrackParameters*,                                 const edm4eic::TrackParameters*,bool);  //Calculate multi-secvertex and store secVtx container
  std::unique_ptr<edm4eic::VertexCollection>
  getSecVtx(const edm4eic::Vertex*,const TLorentzVector&,
            const edm4eic::TrackParameters,
            const edm4eic::TrackParameters,std::vector<double>&);

private:
  //Histos
  struct histos{
    TH1D* m_hb_massKPi{};
    TH1D* m_hb_massPiPi1{};
    TH1D* m_hb_massEE{};
    TH1D* m_hb_totmassEE{};
    TH1D* m_hb_totmass2T0{};
    TH1D* m_hb_totmass2T1{};
    TH1D* m_hb_totmass2T2{};
    TH1D* m_hb_nvrt2{};
    TH1D* m_hb_ratio{};
    TH1D* m_hb_totmass{};
    TH1D* m_hb_impact{};
    TH1D* m_hb_impactR{};
    TH2D* m_hb_impactRZ{};
    TH1D* m_hb_trkD0{};
    TH1D* m_hb_ntrk{};
    TH1D* m_hb_impactZ{};
    TH1D* m_hb_r2d{};
    TH1D* m_hb_r1dc{};
    TH1D* m_hb_r2dc{};
    TH1D* m_hb_r3dc{};
    TH1D* m_hb_rNdc{};
    TH1D* m_hb_dstToMat{};
    TH1D* m_hb_jmom{};
    TH1D* m_hb_mom{};
    TH1D* m_hb_signif3D{};
    TH1D* m_hb_impV0{};
    TH1D* m_hb_sig3DTot{};
    TH1D* m_hb_goodvrtN{};
    TH1D* m_hb_distVV{};
    TH1D* m_hb_diffPS{};
    TH1D* m_hb_sig3D1tr{};
    TH1D* m_hb_sig3D2tr{};
    TH1D* m_hb_sig3DNtr{};
    TH1D* m_hb_trkPtMax{};
    TH1D* m_hb_rawVrtN{};
    TH1D* m_hb_lifetime{};
    TH1D* m_hb_trkPErr{};
    TH1D* m_hb_deltaRSVPV{};
//--
    TProfile* m_pr_NSelTrkMean{};
    TProfile* m_pr_effVrt2tr{};
    TProfile* m_pr_effVrt2trEta{};
    TProfile* m_pr_effVrt{};
    TProfile* m_pr_effVrtEta{};
  };
  std::unique_ptr<histos> m_h;

  // two-track vertex container (D0 and B0)
  // two-body decay modes
  struct vrt2Tr{
    int i=0, j=0;
    int badVrt=0;
    Acts::Vector3 fitVertex;
    TLorentzVector momentum;
    long int vertexCharge;
    std::vector<double> errorMatrix;
    std::vector<double> chi2PerTrk;
    std::vector< std::vector<double> > trkAtVrt;
    double signif3D=0.;
    double signif3DProj=0.;
    double signif2D=0.;
    double chi2=0.;
    double dRSVPV=-1.;
  };

  //For multivertex' container
  struct multiVrt{
    bool Good=true;
    std::deque<long int> selTrk;
    Acts::Vector3 vertex;
    TLorentzVector vertexMom;
    long int vertexCharge{};
    std::vector<double> vertexCov;
    std::vector<double> chi2PerTrk;
    std::vector< std::vector<double> > trkAtVrt;
    double chi2{};
    int nCloseVrt=0;
    double dCloseVrt=1000000.;
    double projectedVrt=0.;
    int detachedTrack=-1;
  };

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

/*
//Create template to remove double counted tracks
template <class Trk>
  void InDetVKalVxInJetTool::removeDoubleEntries(std::vector<const Trk*>& ListTracks) const
  {
    typename std::vector<const Trk*>::iterator   TransfEnd;
    sort(ListTracks.begin(),ListTracks.end());
    TransfEnd =  unique(ListTracks.begin(),ListTracks.end());
    ListTracks.erase( TransfEnd, ListTracks.end());
  }

*/
} // namespace eicrecon
