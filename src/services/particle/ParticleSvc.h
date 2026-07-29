// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#pragma once

#include <algorithms/service.h>
#include <map>
#include <memory>
#include <string>

namespace algorithms {

class ParticleSvc : public Service<ParticleSvc> {
public:
  struct ParticleData {
    int pdgCode;
    int charge;
    double mass;
    std::string name;
  };
  using Particle    = ParticleData;
  using ParticleMap = std::map<int, Particle>;

public:
  virtual void init(std::shared_ptr<ParticleMap> map = nullptr);

  virtual std::shared_ptr<ParticleMap> particleMap() const { return m_particleMap; };

  virtual Particle& particle(int pdg) const;

protected:
  std::shared_ptr<ParticleMap> m_particleMap{nullptr};

  ALGORITHMS_DEFINE_SERVICE(ParticleSvc)

private:
  /// Get the default particle map (defined in ParticleSvc.cc to avoid including large map in header)
  static const ParticleMap& getDefaultParticleMap();
};

} // namespace algorithms
