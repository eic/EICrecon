#ifndef TrackingSecUtilityTool_h
#define TrackingSecUtilityTool_h 1

#include <vector>
#include <algorithm>
//ROOT headers
#include <TLorentzVector.h>
#include <TMath.h>

#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/Vertex.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <spdlog/logger.h>

#include "ActsExamples/EventData/Trajectories.hpp"
#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "SecondaryVertexFinderConfig.h"
#include "IterativeVertexFinder.h"
#include "algorithms/interfaces/WithPodConfig.h"

#include "Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp"
#include "Acts/Plugins/DD4hep/DD4hepFieldAdapter.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/BinningType.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Vertexing/ImpactPointEstimator.hpp"
#include <Acts/Vertexing/Vertex.hpp>
#include <Acts/Vertexing/IterativeVertexFinder.hpp>
#include "Acts/Vertexing/AdaptiveMultiVertexFinder.hpp"
#include <Acts/Vertexing/AdaptiveMultiVertexFitter.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>

/*
namespace eicrecon {
class TrackingSecUtilityTool {
public:
  // Constructor
  TrackingSecUtilityTool(){};
  // Destructor
  ~TrackingSecUtilityTool(){};

/ *
  // Calculate an initial Primary Vertex
  std::unique_ptr<edm4eic::VertexCollection>
  calcPrimaryVtx(const edm4eic::ReconstructedParticleCollection*,
                 std::vector<const ActsExamples::Trajectories*>, Acts::AdaptiveMultiVertexFinder&,
                 Acts::VertexingOptions, Acts::AdaptiveMultiVertexFinder::Config&,
                 Acts::IVertexFinder::State&);

  //Calculate secondary vertex and store secVtx container
  std::unique_ptr<edm4eic::VertexCollection>
  calcSecVtx(const edm4eic::ReconstructedParticleCollection*,
             std::vector<const ActsExamples::Trajectories*> trajectories);

  // Functions to be used to check efficacy of sec. vertex
  bool computeVtxcandidate(const edm4eic::Vertex*, const edm4eic::TrackParameters*,
                           const edm4eic::TrackParameters*, bool);
  //Calculate multi-secvertex and store secVtx container
  std::unique_ptr<edm4eic::VertexCollection>
  getSecVtx(const edm4eic::Vertex*, const TLorentzVector&, const edm4eic::TrackParameters,
            const edm4eic::TrackParameters, std::vector<double>&);

  //This is just a test
  void write2screen();
* /
private:
  //Histos
  // two-track vertex container (D0 and B0)
  // two-body decay modes
  struct vrt2Tr {
    int i = 0, j = 0;
    int badVrt = 0;
    Acts::Vector3 fitVertex;
    TLorentzVector momentum;
    long int vertexCharge;
    std::vector<double> errorMatrix;
    std::vector<double> chi2PerTrk;
    std::vector<std::vector<double>> trkAtVrt;
    double signif3D     = 0.;
    double signif3DProj = 0.;
    double signif2D     = 0.;
    double chi2         = 0.;
    double dRSVPV       = -1.;
  };

  //For multivertex' container
  struct multiVrt {
    bool Good = true;
    std::deque<long int> selTrk;
    Acts::Vector3 vertex;
    TLorentzVector vertexMom;
    long int vertexCharge{};
    std::vector<double> vertexCov;
    std::vector<double> chi2PerTrk;
    std::vector<std::vector<double>> trkAtVrt;
    double chi2{};
    int nCloseVrt       = 0;
    double dCloseVrt    = 1000000.;
    double projectedVrt = 0.;
    int detachedTrack   = -1;
  };

  const double m_massPi  = 139.5702;
  const double m_massP   = 938.272;
  const double m_massE   = 0.511;
  const double m_massK0  = 497.648;
  const double m_massLam = 1115.683;
  const double m_massD0  = 1864.84;
  const double m_massB   = 5279.400;

  // gives correct mass assignment in case of nonequal masses
  static double massV0(std::vector<std::vector<double>>& trkAtVrt, double massP, double massPi);
  TLorentzVector totalMom(const std::vector<const ActsExamples::Trajectories*>& intrk) const;
  TLorentzVector momAtVrt(const std::vector<double>& inpTrk) const;
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::MagneticFieldContext m_fieldctx;
  SecondaryVertexFinderConfig m_cfg;

  //Track parameters to calculate DCA and PCA
  TVector3 distanceR;
  bool secvtxGood = false;
  double trackA_a, trackA_b;
  double trackB_a, trackB_b;
  double vtxA_x, vtxA_y;
  double vtxB_x, vtxB_y;
  double deltaxy_A, deltaA_x, deltaA_y;
  double deltaxy_B, deltaB_x, deltaB_y;
  double minR = 0.05;
};

//Create template to remove double counted tracks
template <class Trk>
  void InDetVKalVxInJetTool::removeDoubleEntries(std::vector<const Trk*>& ListTracks) const
  {
    typename std::vector<const Trk*>::iterator   TransfEnd;
    sort(ListTracks.begin(),ListTracks.end());
    TransfEnd =  unique(ListTracks.begin(),ListTracks.end());
    ListTracks.erase( TransfEnd, ListTracks.end());
  }

} // namespace eicrecon
*/
#endif
