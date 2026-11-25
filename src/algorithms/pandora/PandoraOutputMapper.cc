// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#include "PandoraOutputMapper.h"

#include <Objects/CartesianVector.h>
#include <Objects/ParticleFlowObject.h>
#include <Pandora/PandoraInternal.h>
#include <edm4hep/Vector3f.h>

namespace eicrecon {

void PandoraOutputMapper::retrievePFOs(const pandora::Pandora& pandora,
                                       edm4eic::ReconstructedParticleCollection& outputParticles) {

  const pandora::PfoList* pPfoList = nullptr;
  if (PandoraApi::GetCurrentPfoList(pandora, pPfoList) != pandora::STATUS_CODE_SUCCESS ||
      pPfoList == nullptr) {
    return;
  }

  for (const pandora::ParticleFlowObject* pfo : *pPfoList) {
    auto particle = outputParticles.create();

    // PDG code and charge
    particle.setPDG(pfo->GetParticleId());
    particle.setCharge(static_cast<float>(pfo->GetCharge()));

    // Energy
    particle.setEnergy(pfo->GetEnergy());

    // Momentum
    const pandora::CartesianVector& mom = pfo->GetMomentum();
    particle.setMomentum(edm4hep::Vector3f(mom.GetX(), mom.GetY(), mom.GetZ()));

    // Mass: derive from energy-momentum relation
    const float e     = pfo->GetEnergy();
    const float p2    = mom.GetX() * mom.GetX() + mom.GetY() * mom.GetY() + mom.GetZ() * mom.GetZ();
    const float mass2 = e * e - p2;
    particle.setMass(mass2 > 0.f ? std::sqrt(mass2) : 0.f);

    // Goodnness-of-fit / type: use number of associated clusters as a proxy
    // (a proper chi2 would require a full refit not available at this stage)
    particle.setType(static_cast<int16_t>(pfo->GetClusterList().size()));
  }
}

} // namespace eicrecon
