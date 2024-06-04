// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#pragma once

#include <memory>
#include <algorithms/service.h>

namespace algorithms {

class ParticleSvc : public Service<ParticleSvc> {
public:
  struct ParticleData {
    int         pdgCode;
    int         charge;
    double      mass;
    std::string name;
  };
  using Particle    = ParticleData;
  using ParticleMap = std::map<int, Particle>;

public:
  ParticleSvc();

  virtual void init(std::shared_ptr<ParticleMap> map = nullptr) {
    if (map != nullptr) {
      m_particleMap = map;
    }
  }

  virtual std::shared_ptr<ParticleMap> particleMap() const {
    return m_particleMap;
  };

  virtual Particle& particle(int pdg) const {
    if (m_particleMap->count(pdg) == 0) {
      return m_particleMap->at(0);
    }
    return m_particleMap->at(pdg);
  };

protected:
  std::shared_ptr<ParticleMap> m_particleMap{nullptr};
};

} // namespace algorithms
