// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei

#pragma once

#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>

#include "algorithms/pid_lut/PIDLookup.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class PIDLookup_factory
    : public JOmniAlgorithmFactory<PIDLookup_factory, eicrecon::PIDLookup, PIDLookupConfig> {
private:
  ParameterRef<std::string> m_filename{this, "filename", config().filename,
                                       "Relative to current working directory"};
  ParameterRef<std::string> m_system{this, "system", config().system, "For the ParticleID record"};
};

} // namespace eicrecon
