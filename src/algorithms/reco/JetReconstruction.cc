// SPDX-License-Identifier: LGPL-3.0-or-later

#include "JetReconstructionConfig.h"

#include <cmath>
#include <vector>

// Event Model related classes
#include <edm4hep/Vector3f.h>
#include <edm4eic/vector_utils.h>

using namespace fastjet;


namespace eicrecon {

  void JetReconstruction::init(std::shared_ptr<spdlog::logger> logger, JetReconstructionConfig jet_config) {
    m_config = jet_config;
    m_log = logger;
    m_log->trace("Initialized");
  }

  std::vector<edm4eic::Jet>* JetReconstruction::execute(const std::vector<edm4hep::Vector3f> momenta) {

    // Skip empty
    if (momenta.empty()) {
      m_log->trace("  Empty particle list.");
      return nullptr;
    }

    m_log->trace("  Number of particles: {}", momenta.size());

    // Particles for jet reconstrution
    std::vector<PseudoJet> particles;
    for (const auto &mom : momenta)
      particles.push_back( PseudoJet(mom.x, mom.y, mom.z, edm4eic::magnitude(mom)) );

    // Choose jet and area definitions
    JetDefinition jet_def(m_config.m_jetAlgo, m_config.m_rJet);
    AreaDefinition area_def(m_config.m_areaType, GhostedAreaSpec(m_config.m_ghostMaxRap, m_config.m_numGhostRepeat, m_config.m_ghostArea));

    // Run the clustering, extract the jets
    ClusterSequenceArea clus_seq(particles, jet_def, area_def);
    std::vector<PseudoJet> jets = sorted_by_pt(clus_seq.inclusive_jets());

    // Print out some infos
    m_log->trace("  Clustering with : {}", jet_def.description());

    // Store the jets
    auto jets_edm = new std::vector<edm4eic::Jet>;

    for (unsigned i = 0; i < jets.size(); i++) {

      m_log->trace("  jet {}: pt = {}, y = {}, phi = {}", i, jets[i].pt(), jets[i].rap(), jets[i].phi());
      edm4eic::Jet jet_edm({
          0,  // fill number of constituents later
          jets[i].area(),  // jet area
          jets[i].E(),  // jet energy
          edm4hep::Vector3f(jets[i].px(), jets[i].py(), jets[i].pz())  // jet momentum
          });

      uint32_t ncsts = 0;  // count the number of constituents
      std::vector<PseudoJet> csts = jets[i].constituents();
      for (unsigned j = 0; j < csts.size(); j++) {
        const double cst_pt = csts[j].pt();
        m_log->trace("    constituent {}'s pt: {}", j, cst_pt);
        // Only consider constituents in the momentum range
        if (cst_pt > m_config.m_minCstPt && cst_pt < m_config.m_maxCstPt) {
          jet_edm.AddtoCstEnergy(csts[j].E());  // constituent energy
          jet_edm.AddtoCstMomentum(edm4hep::Vector3f(csts[j].px(), csts[j].py(), csts[j].pz()));  // constituent momentum
          ncsts++;
        }
      } // for constituent j

      jet_edm.nCsts = ncsts;  // number of constituents
      jets_edm->push_back(jet_edm);
    } // for jet i

    // return the jets
    return jets_edm;
  }

} // namespace eicrecon
