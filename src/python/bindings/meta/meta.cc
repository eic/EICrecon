// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python extension module eicrecon._meta: the generic (meta) factory types
// (src/factories/meta) for eicrecon.factories.meta, and C++ helpers for config values that
// Python cannot write. Add one bind_factory line per factory type that a wiring uses.

#include <pybind11/pybind11.h>

#include "FactoryType.h"
#include "SubDivideFunctorBindings.h"
#include "algorithms/reco/HadronicFinalState.h"
#include "algorithms/reco/InclusiveKinematicsDA.h"
#include "algorithms/reco/InclusiveKinematicsESigma.h"
#include "algorithms/reco/InclusiveKinematicsElectron.h"
#include "algorithms/reco/InclusiveKinematicsJB.h"
#include "algorithms/reco/InclusiveKinematicsSigma.h"
#include "factories/meta/Cloner_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/CopyLinks_factory.h"
#include "factories/meta/FilterMatching_factory.h"
#include "factories/meta/ONNXInference_factory.h"
#include "factories/meta/ObjectIDAccessors.h"
#include "factories/meta/SortSubsetCollection_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"
#include "factories/reco/ReconstructedParticleSortKeys.h"
#include <cstdint>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleLinkCollection.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/MCRecoTrackParticleLinkCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <format> // IWYU pragma: keep
#include <podio/detail/Link.h>
#include <string>

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

namespace {

// DIRC wiring
void add_dirc_helpers(pybind11::module_& m) {
  eicrecon::python::bind_geometry_split<edm4hep::SimTrackerHit>(m, "GeometrySplit_SimTrackerHit");
}

// LOWQ2 wiring
void add_lowq2_helpers(pybind11::module_& m) {
  eicrecon::python::bind_geometry_split<edm4eic::TrackerHit>(m, "GeometrySplit_TrackerHit");
}

// beam wiring
void add_beam_helpers(pybind11::module_& m) {
  using Fn = decltype(eicrecon::SubDivideCollectionConfig<edm4hep::MCParticle>::function);
  m.def(
      "ValueSplit_GeneratorStatus_PDG",
      [](std::vector<std::vector<int>> values) {
        return eicrecon::python::make_cpp_function<Fn>(
            Fn{eicrecon::ValueSplit<&edm4hep::MCParticle::getGeneratorStatus,
                                    &edm4hep::MCParticle::getPDG>{values}});
      },
      pybind11::arg("values"),
      "SubDivideCollection function: split MCParticles by (generator status, PDG) pairs");
}

// particle_flow wiring
void add_particle_flow_helpers(pybind11::module_& m) {
  using Fn = decltype(eicrecon::SubDivideCollectionConfig<edm4eic::TrackSegment>::function);
  m.def(
      "TrackPointSystemSplit",
      [](std::vector<std::uint32_t> system_ids) {
        return eicrecon::python::make_cpp_function<Fn>(
            Fn{eicrecon::TrackPointSystemSplit{system_ids}});
      },
      pybind11::arg("system_ids"),
      "SubDivideCollection function: split track projections by the systems of their points");
}

} // namespace

PYBIND11_MODULE(_meta, m) {
  m.doc() = "EICrecon generic (meta) factory types";
  py::module_::import("eicrecon._core");
  bind_factory<Cloner_factory<edm4hep::MCParticle>>(m, "Cloner_MCParticle");
  bind_factory<Cloner_factory<edm4eic::ReconstructedParticle>>(m, "Cloner_ReconstructedParticle");
  bind_factory<CollectionCollector_factory<edm4eic::Cluster, false>>(
      m, "CollectionCollector_Cluster_false");
  bind_factory<CollectionCollector_factory<edm4eic::Cluster, true>>(
      m, "CollectionCollector_Cluster_true");
  bind_factory<CollectionCollector_factory<edm4eic::InclusiveKinematics>>(
      m, "CollectionCollector_InclusiveKinematics");
  bind_factory<CollectionCollector_factory<edm4hep::MCParticle, true>>(
      m, "CollectionCollector_MCParticle_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoClusterParticleAssociation, true>>(
      m, "CollectionCollector_MCRecoClusterParticleAssociation_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoParticleAssociation, true>>(
      m, "CollectionCollector_MCRecoParticleAssociation_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoParticleLink, false>>(
      m, "CollectionCollector_MCRecoParticleLink_false");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoParticleLink, true>>(
      m, "CollectionCollector_MCRecoParticleLink_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoTrackParticleAssociation, true>>(
      m, "CollectionCollector_MCRecoTrackParticleAssociation_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoTrackParticleLink, true>>(
      m, "CollectionCollector_MCRecoTrackParticleLink_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoTrackerHitAssociation, true>>(
      m, "CollectionCollector_MCRecoTrackerHitAssociation_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoTrackerHitLink, true>>(
      m, "CollectionCollector_MCRecoTrackerHitLink_true");
  bind_factory<CollectionCollector_factory<edm4eic::Measurement2D, true>>(
      m, "CollectionCollector_Measurement2D_true");
  bind_factory<CollectionCollector_factory<edm4eic::RawTrackerHit>>(
      m, "CollectionCollector_RawTrackerHit");
  bind_factory<CollectionCollector_factory<edm4eic::ReconstructedParticle, false>>(
      m, "CollectionCollector_ReconstructedParticle_false");
  bind_factory<CollectionCollector_factory<edm4eic::ReconstructedParticle, true>>(
      m, "CollectionCollector_ReconstructedParticle_true");
  bind_factory<CollectionCollector_factory<edm4hep::SimTrackerHit, true>>(
      m, "CollectionCollector_SimTrackerHit_true");
  bind_factory<CollectionCollector_factory<edm4eic::TrackClusterMatch, false>>(
      m, "CollectionCollector_TrackClusterMatch_false");
  bind_factory<CollectionCollector_factory<edm4eic::TrackClusterMatch, true>>(
      m, "CollectionCollector_TrackClusterMatch_true");
  bind_factory<CollectionCollector_factory<edm4eic::Track, true>>(m,
                                                                  "CollectionCollector_Track_true");
  bind_factory<CollectionCollector_factory<edm4eic::TrackerHit, true>>(
      m, "CollectionCollector_TrackerHit_true");
  bind_factory<CopyLinks_factory<edm4eic::ReconstructedParticle, edm4eic::MCRecoParticleLink>>(
      m, "CopyLinks_ReconstructedParticle_MCRecoParticleLink");
  bind_factory<FilterMatching_factory<edm4eic::Cluster, eicrecon::object_id::self,
                                      edm4eic::TrackClusterMatch, eicrecon::object_id::cluster>>(
      m, "FilterMatching_Cluster_self_TrackClusterMatch_cluster");
  bind_factory<
      FilterMatching_factory<edm4eic::MCRecoClusterParticleAssociation, eicrecon::object_id::rec,
                             edm4eic::TrackClusterMatch, eicrecon::object_id::cluster>>(
      m, "FilterMatching_MCRecoClusterParticleAssociation_rec_TrackClusterMatch_cluster");
  bind_factory<FilterMatching_factory<edm4eic::MCRecoParticleAssociation, eicrecon::object_id::sim,
                                      edm4hep::MCParticle, eicrecon::object_id::self>>(
      m, "FilterMatching_MCRecoParticleAssociation_sim_MCParticle_self");
  bind_factory<FilterMatching_factory<edm4eic::MCRecoParticleLink, eicrecon::object_id::to,
                                      edm4hep::MCParticle, eicrecon::object_id::self>>(
      m, "FilterMatching_MCRecoParticleLink_to_MCParticle_self");
  bind_factory<ONNXInference_factory>(m, "ONNXInference");
  bind_factory<SortSubsetCollection_factory<edm4eic::ReconstructedParticle, reco_particle_pt>>(
      m, "SortSubsetCollection_ReconstructedParticle_reco_particle_pt");
  bind_factory<SubDivideCollection_factory<edm4hep::MCParticle>>(m,
                                                                 "SubDivideCollection_MCParticle");
  bind_factory<SubDivideCollection_factory<edm4hep::SimTrackerHit, true>>(
      m, "SubDivideCollection_SimTrackerHit_true");
  bind_factory<SubDivideCollection_factory<edm4eic::TrackSegment>>(
      m, "SubDivideCollection_TrackSegment");
  bind_factory<SubDivideCollection_factory<edm4eic::TrackerHit>>(m,
                                                                 "SubDivideCollection_TrackerHit");
  add_dirc_helpers(m);
  add_lowq2_helpers(m);
  add_beam_helpers(m);
  add_particle_flow_helpers(m);
}
