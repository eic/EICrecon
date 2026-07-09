// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Sebouh Paul, Baptiste Fraisse

#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <stddef.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <string>

#include "FarForwardNeutralsReconstruction.h"

namespace eicrecon {

void FarForwardNeutralsReconstruction::init() {
  try {
    m_gammaZMax =
        m_cfg.gammaZMaxOffset + m_detector->constant<double>(m_cfg.offsetPositionName) / dd4hep::mm;
  } catch (std::runtime_error&) {
    m_gammaZMax = m_cfg.gammaZMaxOffset + 35800.0;
    trace("Failed to get {} from the detector, using default value of {}", m_cfg.offsetPositionName,
          m_gammaZMax);
  }
  trace("gamma detection params:   max length={},   max width={},   max z={}", m_cfg.gammaMaxLength,
        m_cfg.gammaMaxWidth, m_gammaZMax);
}

/*
     check that the cluster position is within the correct range into ZDC,
     and that the sqrt(largest eigenvalue) is less than gamma_max_length,
     and that the sqrt(second largest eigenvalue) is less than gamma_max_width
*/
bool FarForwardNeutralsReconstruction::isGamma(const edm4eic::Cluster& cluster) const {

  double l1 = sqrt(cluster.getShapeParameters(4)) * dd4hep::mm;
  double l2 = sqrt(cluster.getShapeParameters(5)) * dd4hep::mm;
  double l3 = sqrt(cluster.getShapeParameters(6)) * dd4hep::mm;

  // z in the local coordinates
  double z = (cluster.getPosition().z * cos(m_cfg.globalToProtonRotation) +
              cluster.getPosition().x * sin(m_cfg.globalToProtonRotation)) *
             dd4hep::mm;

  trace("z recon = {}", z);
  trace("l1 = {}, l2 = {}, l3 = {}", l1, l2, l3);

  bool isZMoreThanMax = (z > m_gammaZMax);
  bool isLengthMoreThanMax =
      (l1 > m_cfg.gammaMaxLength || l2 > m_cfg.gammaMaxLength || l3 > m_cfg.gammaMaxLength);
  bool areWidthsMoreThanMax = static_cast<int>(l1 > m_cfg.gammaMaxWidth) +
                                  static_cast<int>(l2 > m_cfg.gammaMaxWidth) +
                                  static_cast<int>(l3 > m_cfg.gammaMaxWidth) >=
                              2;

  return !(isZMoreThanMax || isLengthMoreThanMax || areWidthsMoreThanMax);
}

double FarForwardNeutralsReconstruction::corrPower(double E, const std::vector<double>& coeffs) {

  if (coeffs.size() != 2) {
    throw std::runtime_error(
        "Energy correction for Lambda reconstruction requires 2 coefficients: a * E^b.");
  }

  return coeffs[0] * std::pow(E, coeffs[1]);
}

int FarForwardNeutralsReconstruction::processNeutralCalo(
    const edm4eic::ClusterCollection* clusters,
    edm4eic::ReconstructedParticleCollection* out_neutrals,
    const std::vector<double>& gammaScaleCoeff, const std::vector<double>& neutronScaleCoeff,
    bool canDetectGammas, bool canDetectNeutrons, const CorrFunc& gammaCorr,
    const CorrFunc& neutronCorr, GammaMode gammaMode, double clusterEmin,
    NeutronMode neutronMode, bool associateAllClustersToNeutron) const {

  const double m_neutron = m_particleSvc.particle(2112).mass;

  if ((clusters == nullptr) || clusters->empty()) {
    return 0;
  }
  if (!canDetectGammas) {
    gammaMode = GammaMode::None;
  }

  // gammas from clusters
  auto makeGamma = [&](const edm4eic::Cluster& cl) {
    auto rec = out_neutrals->create();
    rec.setPDG(22);

    const auto pos = cl.getPosition();
    const double E = gammaCorr(cl.getEnergy(), gammaScaleCoeff);

    const double r = edm4hep::utils::magnitude(pos);
    if (r > 0) {
      rec.setMomentum(pos * (E / r));
    }

    rec.setEnergy(E);
    rec.setCharge(0);
    rec.setMass(0);
    rec.addToClusters(cl);
  };

  std::vector<const edm4eic::Cluster*> gamma_used;
  gamma_used.reserve(clusters->size());

  // gammaMode == LeaderOnly
  if (gammaMode == GammaMode::LeaderOnly) {

    std::vector<int> idx;
    idx.reserve(clusters->size());

    double Esum = 0.0;
    for (int i = 0, n = (int)clusters->size(); i < n; ++i) {
      const double E = (*clusters)[i].getEnergy();
      if (E < clusterEmin) {
        continue;
      }
      Esum += E;
      idx.push_back(i);
    }

    if (idx.empty() || Esum <= 0.0) {
      return 0;
    }

    // Sort clusters by decreasing energy before keeping the leading ones.
    std::ranges::sort(
        idx, [&](int a, int b) { return (*clusters)[a].getEnergy() > (*clusters)[b].getEnergy(); });

    // Keep only the leading clusters in order to limit combinatorial
    const size_t Nkeep = 4;
    for (size_t k = 0; k < std::min(Nkeep, idx.size()); ++k) {
      const auto& cl = (*clusters)[idx[k]];
      makeGamma(cl);
      gamma_used.push_back(&cl);
    }

  }

  // gammaMode == AllPassing
  else if (gammaMode == GammaMode::AllPassing) {
    for (const auto& cl : *clusters) {
      const double E = cl.getEnergy();
      if (E < clusterEmin)
        continue;
      if (isGamma(cl)) {
        makeGamma(cl);
        gamma_used.push_back(&cl);
      }
    }
  }

  // gammaMode == None => nothing
  auto is_used_as_gamma = [&](const edm4eic::Cluster& cl) {
    for (auto* p : gamma_used)
      if (p == &cl)
        return true;
    return false;
  };

  // neutrons from clusters
  const edm4eic::Cluster* leaderN = nullptr;
  double E_leader                 = -1.0;

  double E_sum = 0.0;
  std::vector<const edm4eic::Cluster*> kept;
  kept.reserve(clusters->size());

  for (const auto& cl : *clusters) {
    if (gammaMode != GammaMode::None && is_used_as_gamma(cl))
      continue;

    const double E = cl.getEnergy();
    if (E < clusterEmin)
      continue;

    E_sum += E;
    kept.push_back(&cl);

    if (E > E_leader) {
      E_leader = E;
      leaderN  = &cl;
    }
  }

  if (!canDetectNeutrons)
    return 0;

  double En_raw = 0.0;
  edm4hep::Vector3f n_pos{0, 0, 0};

  if (neutronMode == NeutronMode::LeaderOnly) {
    if (!leaderN || E_leader <= 0.0)
      return 0;
    En_raw = E_leader;
    n_pos  = leaderN->getPosition();

    kept.clear();
    kept.push_back(leaderN);
  } else if (neutronMode == NeutronMode::SumAll) {
    if (E_sum <= 0.0 || kept.empty())
      return 0;
    En_raw = E_sum;
    if (!leaderN || E_leader <= 0.0)
      return 0;
    n_pos = leaderN->getPosition();
  } else {
    return 0;
  }

  // apply calibration laws
  const double En = neutronCorr(En_raw, neutronScaleCoeff);

  auto rec = out_neutrals->create();
  rec.setPDG(2112);
  rec.setEnergy(En);
  rec.setCharge(0);
  rec.setMass(m_neutron);

  const double r = edm4hep::utils::magnitude(n_pos);
  if (r > 0) {
    const double psq = std::max(0.0, En * En - m_neutron * m_neutron);
    rec.setMomentum(n_pos * (std::sqrt(psq) / r));
  }
  rec.setReferencePoint(n_pos);

  if (associateAllClustersToNeutron) {
    for (const auto& cl : *clusters)
      rec.addToClusters(cl);
  } else {
    for (auto* clp : kept)
      rec.addToClusters(*clp);
  }

  return 1;
}

void FarForwardNeutralsReconstruction::process(
    const FarForwardNeutralsReconstruction::Input& input,
    const FarForwardNeutralsReconstruction::Output& output) const {

  // Unpacking
  const auto [clusters] = input;
  auto [out_neutrals]   = output;

  GammaMode gammaMode = GammaMode::None;
  if (m_cfg.gammaMode == "LeaderOnly") {
    gammaMode = GammaMode::LeaderOnly;
  } else if (m_cfg.gammaMode == "AllPassing") {
    gammaMode = GammaMode::AllPassing;
  }

  NeutronMode neutronMode = NeutronMode::None;
  if (m_cfg.neutronMode == "SumAll") {
    neutronMode = NeutronMode::SumAll;
  } else if (m_cfg.neutronMode == "LeaderOnly") {
    neutronMode = NeutronMode::LeaderOnly;
  }

  const int n_neutrons =
      processNeutralCalo(clusters, out_neutrals,
                         /*gammaScaleCoeff=*/m_cfg.gammaScaleCorrCoeff,
                         /*neutronScaleCoeff=*/m_cfg.neutronScaleCorrCoeff,
                         /*canDetectGammas=*/m_cfg.canDetectGammas,
                         /*canDetectNeutrons=*/m_cfg.canDetectNeutrons,
                         /*gammaCorr=*/corrPower,
                         /*neutronCorr=*/corrPower,
                         /*gammaMode=*/gammaMode,
                         /*clusterEmin=*/m_cfg.clusterEmin,
                         /*neutronMode=*/neutronMode,
                         /*associateAllClustersToNeutron=*/m_cfg.associateAllClustersToNeutron);

  debug("Found {} neutron candidates", n_neutrons);
}

} // namespace eicrecon
