// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#ifndef EDM4NWB_Cluster_CollectionData_H
#define EDM4NWB_Cluster_CollectionData_H

// datamodel specific includes
#include "edm4nwb/ClusterData.h"
#include "edm4nwb/ClusterObj.h"
#include "edm4nwb/CalorimeterHit.h"
#include "edm4hep/ParticleID.h"

// podio specific includes
#include "podio/CollectionBuffers.h"
#include "podio/ICollectionProvider.h"

#include <deque>
#include <memory>

namespace edm4nwb {


using ClusterObjPointerContainer = std::deque<ClusterObj*>;
using ClusterDataContainer = std::vector<ClusterData>;


/**
 * Class encapsulating everything related to storage of data that is needed by a
 * collection.
 */
class ClusterCollectionData {
public:
  /**
   * The Objs of this collection
   */
  ClusterObjPointerContainer entries{};

  /**
   * Default constructor setting up the necessary buffers
   */
  ClusterCollectionData();

  /**
   * Non copy-able, move-only class
   */
  ClusterCollectionData(const ClusterCollectionData&) = delete;
  ClusterCollectionData& operator=(const ClusterCollectionData&) = delete;
  ClusterCollectionData(ClusterCollectionData&& other) = default;
  ClusterCollectionData& operator=(ClusterCollectionData&& other) = default;

  /**
   * Deconstructor
   */
  ~ClusterCollectionData() = default;

  void clear(bool isSubsetColl);

  podio::CollectionBuffers getCollectionBuffers(bool isSubsetColl);

  void prepareForWrite(bool isSubsetColl);

  void prepareAfterRead(int collectionID);

  void makeSubsetCollection();

  void createRelations(ClusterObj* obj);

  bool setReferences(const podio::ICollectionProvider* collectionProvider, bool isSubsetColl);

private:
  // members to handle 1-to-N-relations
  podio::UVecPtr<edm4nwb::Cluster> m_rel_clusters;  ///< Relation buffer for read / write
  std::vector<podio::UVecPtr<edm4nwb::Cluster>> m_rel_clusters_tmp{}; ///< Relation buffer for internal book-keeping
  podio::UVecPtr<edm4nwb::CalorimeterHit> m_rel_hits;  ///< Relation buffer for read / write
  std::vector<podio::UVecPtr<edm4nwb::CalorimeterHit>> m_rel_hits_tmp{}; ///< Relation buffer for internal book-keeping
  podio::UVecPtr<edm4hep::ParticleID> m_rel_particleIDs;  ///< Relation buffer for read / write
  std::vector<podio::UVecPtr<edm4hep::ParticleID>> m_rel_particleIDs_tmp{}; ///< Relation buffer for internal book-keeping

  // members to handle vector members
  podio::UVecPtr<float> m_vec_shapeParameters; /// combined vector of all objects in collection
  std::vector<podio::UVecPtr<float>> m_vecs_shapeParameters{}; /// pointers to individual member vectors
  podio::UVecPtr<float> m_vec_hitContributions; /// combined vector of all objects in collection
  std::vector<podio::UVecPtr<float>> m_vecs_hitContributions{}; /// pointers to individual member vectors
  podio::UVecPtr<float> m_vec_subdetectorEnergies; /// combined vector of all objects in collection
  std::vector<podio::UVecPtr<float>> m_vecs_subdetectorEnergies{}; /// pointers to individual member vectors

  // I/O related buffers
  podio::CollRefCollection m_refCollections{};
  podio::VectorMembersInfo m_vecmem_info{};
  std::unique_ptr<ClusterDataContainer> m_data{nullptr};
};


} // namespace edm4nwb


#endif
