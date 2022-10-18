// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#include "edm4nwb/ClusterObj.h"
#include "edm4nwb/Cluster.h"
namespace edm4nwb {

ClusterObj::ClusterObj() :
  ObjBase{{podio::ObjectID::untracked, podio::ObjectID::untracked}, 0},
  data(),
  m_clusters(new std::vector<edm4nwb::Cluster>()),
  m_hits(new std::vector<edm4nwb::CalorimeterHit>()),
  m_particleIDs(new std::vector<edm4hep::ParticleID>()),
  m_shapeParameters(new std::vector<float>()),
  m_hitContributions(new std::vector<float>()),
  m_subdetectorEnergies(new std::vector<float>())
{  }

ClusterObj::ClusterObj(const podio::ObjectID id_, ClusterData data_) :
  ObjBase{id_, 0}, data(data_)
{  }

ClusterObj::ClusterObj(const ClusterObj& other) :
  ObjBase{{podio::ObjectID::untracked, podio::ObjectID::untracked}, 0},
  data(other.data),
  m_clusters(new std::vector<edm4nwb::Cluster>(*(other.m_clusters))),
  m_hits(new std::vector<edm4nwb::CalorimeterHit>(*(other.m_hits))),
  m_particleIDs(new std::vector<edm4hep::ParticleID>(*(other.m_particleIDs))),
  m_shapeParameters(new std::vector<float>(*(other.m_shapeParameters))),
  m_hitContributions(new std::vector<float>(*(other.m_hitContributions))),
  m_subdetectorEnergies(new std::vector<float>(*(other.m_subdetectorEnergies)))
{
}

ClusterObj::~ClusterObj() {
  if (id.index == podio::ObjectID::untracked) {
    delete m_clusters;
    delete m_hits;
    delete m_particleIDs;
    delete m_shapeParameters;
    delete m_hitContributions;
    delete m_subdetectorEnergies;
  }

}
} // namespace edm4nwb

