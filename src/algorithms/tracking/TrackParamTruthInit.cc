// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#include "TrackParamTruthInit.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Result.hpp>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/logger.h>
#include <edm4eic/Cov6f.h>
#include <edm4hep/Vector3d.h>
#include <fmt/core.h>
#include <Eigen/Core>
#include <cmath>
#include <cstdlib>
#include <gsl/pointers>
#include <limits>
#include <memory>

#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep

namespace eicrecon {

void TrackParamTruthInit::process(const Input& input, const Output& output) const {
  // MCParticles uses numerical values in its specified units,
  // while m_cfg is in the DD4hep unit system

  const auto [headers, mcparticles] = input;
  auto [track_parameters]           = output;

  // local random generator
  auto seed = m_uid.getUniqueID(*headers, "TrackParamTruthInit");
  std::default_random_engine generator;
  std::normal_distribution<double> gaussian;

  // Loop over input particles
  for (const auto& mcparticle : *mcparticles) {

    // require generatorStatus == 1 for stable generated particles in HepMC3 and DDSim gun
    if (mcparticle.getGeneratorStatus() != 1) {
      trace("ignoring particle with generatorStatus = {}", mcparticle.getGeneratorStatus());
      continue;
    }

    // require close to interaction vertex
    auto v = mcparticle.getVertex();
    if (std::abs(v.x) * dd4hep::mm > m_cfg.maxVertexX ||
        std::abs(v.y) * dd4hep::mm > m_cfg.maxVertexY ||
        std::abs(v.z) * dd4hep::mm > m_cfg.maxVertexZ) {
      trace("ignoring particle with vs = {} [mm]", v);
      continue;
    }

    // require minimum momentum
    const auto& p   = mcparticle.getMomentum();
    const auto pmag = std::hypot(p.x, p.y, p.z);
    if (pmag * dd4hep::GeV < m_cfg.minMomentum) {
      trace("ignoring particle with p = {} GeV ", pmag);
      continue;
    }

    // require minimum pseudorapidity
    const auto phi   = std::atan2(p.y, p.x);
    const auto theta = std::atan2(std::hypot(p.x, p.y), p.z);
    const auto eta   = -std::log(std::tan(theta / 2));
    if (eta > m_cfg.maxEtaForward || eta < -std::abs(m_cfg.maxEtaBackward)) {
      trace("ignoring particle with Eta = {}", eta);
      continue;
    }

    // get the particle charge
    // note that we cannot trust the mcparticles charge, as DD4hep
    // sets this value to zero! let's lookup by PDGID instead
    const auto pdg      = mcparticle.getPDG();
    const auto particle = m_particleSvc.particle(pdg);
    double charge       = std::copysign(1.0, particle.charge);
    if (std::abs(particle.charge) < std::numeric_limits<double>::epsilon()) {
      trace("ignoring neutral particle");
      continue;
    }

    // modify initial momentum to avoid bleeding truth to results when fit fails
    const auto pinit = pmag * (1.0 + m_cfg.momentumSmear * gaussian(generator));

    // define line surface for local position values
    auto perigee = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0, 0, 0));

    // track particle back to transverse point-of-closest approach
    // with respect to the defined line surface
    auto linesurface_parameter = -(v.x * p.x + v.y * p.y) / (p.x * p.x + p.y * p.y);

    auto xpca = v.x + linesurface_parameter * p.x;
    auto ypca = v.y + linesurface_parameter * p.y;
    auto zpca = v.z + linesurface_parameter * p.z;

    Acts::Vector3 global(xpca, ypca, zpca);
    Acts::Vector3 direction(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

    // convert from global to local coordinates using the defined line surface
    auto local = perigee->globalToLocal(m_geoSvc->getActsGeometryContext(), global, direction);

    if (!local.ok()) {
      error("skipping the track because globaltoLocal function failed");
      continue;
    }

    Acts::Vector2 localpos = local.value();

    // Insert into edm4eic::TrackParameters, which uses numerical values in its specified units
    auto track_parameter = track_parameters->create();
    track_parameter.setType(-1); // type --> seed(-1)
    track_parameter.setLoc({static_cast<float>(localpos(0)),
                            static_cast<float>(localpos(1))}); // 2d location on surface [mm]
    track_parameter.setPhi(phi);                               // phi [rad]
    track_parameter.setTheta(theta);                           // theta [rad]
    track_parameter.setQOverP(charge / (pinit / dd4hep::GeV)); // Q/p [e/GeV]
    track_parameter.setTime(mcparticle.getTime());             // time [ns]
    edm4eic::Cov6f cov;
    cov(0, 0) = 1.0;  // loc0
    cov(1, 1) = 1.0;  // loc1
    cov(2, 2) = 0.05; // phi
    cov(3, 3) = 0.01; // theta
    cov(4, 4) = 0.1;  // qOverP
    cov(5, 5) = 10e9; // time
    track_parameter.setCovariance(cov);

    // Debug output
    if (level() <= algorithms::LogLevel::kDebug) {
      debug("Invoke track finding seeded by truth particle with:");
      debug("   p     = {} GeV (smeared to {} GeV)", pmag / dd4hep::GeV, pinit / dd4hep::GeV);
      debug("   q     = {}", charge);
      debug("   q/p   = {} e/GeV (smeared to {} e/GeV)", charge / (pmag / dd4hep::GeV),
            charge / (pinit / dd4hep::GeV));
      debug("   theta = {}", theta);
      debug("   phi   = {}", phi);
    }
  }
}

} // namespace eicrecon
