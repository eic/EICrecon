// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023-2025, Simon Gardner

#pragma once

#include <TMVA/MethodBase.h>
#include <TMVA/Reader.h>
#include <algorithms/algorithm.h>
// Event Model related classes
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <mutex>
#include <string>
#include <string_view>

#include "FarDetectorMLReconstructionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

enum FarDetectorMLNNIndexIn { PosY, PosZ, DirX, DirY };
enum FarDetectorMLNNIndexOut { MomX, MomY, MomZ };

using FarDetectorMLReconstructionAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackParametersCollection, edm4hep::MCParticleCollection,
                      edm4eic::TrackCollection, edm4eic::MCRecoTrackParticleAssociationCollection>,
    algorithms::Output<edm4eic::TrajectoryCollection, edm4eic::TrackParametersCollection,
                       edm4eic::TrackCollection,
                       edm4eic::MCRecoTrackParticleAssociationCollection>>;

class FarDetectorMLReconstruction : public FarDetectorMLReconstructionAlgorithm,
                                    public WithPodConfig<FarDetectorMLReconstructionConfig> {

public:
  FarDetectorMLReconstruction(std::string_view name)
      : FarDetectorMLReconstructionAlgorithm{
            name,
            {"TrackParameters", "BeamElectrons", "FittedTracks", "FittedTrackAssociations"},
            {"Trajectory", "TrackParameters", "Track", "PropagatedTrackAssociations"},
            "Reconstruct track parameters using ML method."} {}

  /** One time initialization **/
  void init();

  /** Event by event processing **/
  void process(const Input&, const Output&) const final;

  //----- Define constants here ------

private:
  TMVA::Reader* m_reader{nullptr};
  TMVA::MethodBase* m_method{nullptr};

  mutable float m_beamE{10.0};
  mutable std::once_flag m_initBeamE;

  mutable float nnInput[4] = {0.0, 0.0, 0.0, 0.0};
};

} // namespace eicrecon
