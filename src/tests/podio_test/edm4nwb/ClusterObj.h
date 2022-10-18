// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#ifndef EDM4NWB_ClusterOBJ_H
#define EDM4NWB_ClusterOBJ_H

// data model specific includes
#include "edm4nwb/ClusterData.h"
#include "edm4nwb/CalorimeterHit.h"
#include "edm4hep/ParticleID.h"
#include <vector>

#include "podio/ObjBase.h"
#include <vector>


namespace edm4nwb {

class Cluster;

class ClusterObj : public podio::ObjBase {
public:
  /// constructor
  ClusterObj();
  /// copy constructor (does a deep-copy of relation containers)
  ClusterObj(const ClusterObj&);
  /// constructor from ObjectID and ClusterData
  /// does not initialize the internal relation containers
  ClusterObj(const podio::ObjectID id, ClusterData data);
  /// No assignment operator
  ClusterObj& operator=(const ClusterObj&) = delete;
  virtual ~ClusterObj();

public:
  ClusterData data;
  std::vector<edm4nwb::Cluster>* m_clusters{nullptr};
  std::vector<edm4nwb::CalorimeterHit>* m_hits{nullptr};
  std::vector<edm4hep::ParticleID>* m_particleIDs{nullptr};
  std::vector<float>* m_shapeParameters{nullptr};
  std::vector<float>* m_hitContributions{nullptr};
  std::vector<float>* m_subdetectorEnergies{nullptr};
};

} // namespace edm4nwb


#endif
