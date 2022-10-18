// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#ifndef EDM4EIC_MutableCluster_H
#define EDM4EIC_MutableCluster_H

#include "edm4nwb/ClusterObj.h"
// Make the immutable class available from its mutable version but not vice versa
#include "edm4nwb/Cluster.h"

#include "edm4nwb/CalorimeterHit.h"
#include "edm4eic/Cov2f.h"
#include "edm4eic/Cov3f.h"
#include "edm4hep/ParticleID.h"
#include "edm4hep/Vector3f.h"
#include "podio/RelationRange.h"
#include <cstdint>
#include <vector>
#include "podio/ObjectID.h"
#include <ostream>

#ifdef PODIO_JSON_OUTPUT
#include "nlohmann/json.hpp"
#endif



namespace edm4nwb {


/** @class MutableCluster
 *  EIC hit cluster, reworked to more closely resemble EDM4hep
 *  @author: W. Armstrong, S. Joosten, C.Peng
 */
class MutableCluster {

  friend class ClusterCollection;
  friend class ClusterMutableCollectionIterator;
  friend class Cluster;

public:

  /// default constructor
  MutableCluster();
  MutableCluster(std::int32_t type, float energy, float energyError, float time, float timeError, std::uint32_t nhits, edm4hep::Vector3f position, edm4eic::Cov3f positionError, float intrinsicTheta, float intrinsicPhi, edm4eic::Cov2f intrinsicDirectionError);

  /// constructor from existing ClusterObj
  MutableCluster(ClusterObj* obj);

  /// copy constructor
  MutableCluster(const MutableCluster& other);

  /// copy-assignment operator
  MutableCluster& operator=(MutableCluster other);

  /// create a mutable deep-copy of the object with identical relations
  MutableCluster clone() const;

  /// destructor
  ~MutableCluster();

  /// conversion to const object
  operator Cluster() const;

public:

  /// Access the Flagword that defines the type of the cluster
  const std::int32_t& getType() const;

  /// Access the Reconstructed energy of the cluster [GeV].
  const float& getEnergy() const;

  /// Access the Error on the cluster energy [GeV]
  const float& getEnergyError() const;

  /// Access the [ns]
  const float& getTime() const;

  /// Access the Error on the cluster time
  const float& getTimeError() const;

  /// Access the Number of hits in the cluster.
  const std::uint32_t& getNhits() const;

  /// Access the Global position of the cluster [mm].
  const edm4hep::Vector3f& getPosition() const;

  /// Access the Covariance matrix of the position (6 Parameters).
  const edm4eic::Cov3f& getPositionError() const;

  /// Access the Intrinsic cluster propagation direction polar angle [rad]
  const float& getIntrinsicTheta() const;

  /// Access the Intrinsic cluster propagation direction azimuthal angle [rad]
  const float& getIntrinsicPhi() const;

  /// Access the Error on the intrinsic cluster propagation direction
  const edm4eic::Cov2f& getIntrinsicDirectionError() const;



  /// Set the Flagword that defines the type of the cluster
  void setType(std::int32_t value);

  /// Set the Reconstructed energy of the cluster [GeV].
  void setEnergy(float value);

  /// Set the Error on the cluster energy [GeV]
  void setEnergyError(float value);

  /// Set the [ns]
  void setTime(float value);

  /// Set the Error on the cluster time
  void setTimeError(float value);

  /// Set the Number of hits in the cluster.
  void setNhits(std::uint32_t value);

  /// Set the Global position of the cluster [mm].
  void setPosition(edm4hep::Vector3f value);
  /// Get reference to Global position of the cluster [mm].
  edm4hep::Vector3f& position();

  /// Set the Covariance matrix of the position (6 Parameters).
  void setPositionError(edm4eic::Cov3f value);
  /// Get reference to Covariance matrix of the position (6 Parameters).
  edm4eic::Cov3f& positionError();

  /// Set the Intrinsic cluster propagation direction polar angle [rad]
  void setIntrinsicTheta(float value);

  /// Set the Intrinsic cluster propagation direction azimuthal angle [rad]
  void setIntrinsicPhi(float value);

  /// Set the Error on the intrinsic cluster propagation direction
  void setIntrinsicDirectionError(edm4eic::Cov2f value);
  /// Get reference to Error on the intrinsic cluster propagation direction
  edm4eic::Cov2f& intrinsicDirectionError();



  void addToClusters(edm4nwb::Cluster);
  unsigned int clusters_size() const;
  edm4nwb::Cluster getClusters(unsigned int) const;
  std::vector<edm4nwb::Cluster>::const_iterator clusters_begin() const;
  std::vector<edm4nwb::Cluster>::const_iterator clusters_end() const;
  podio::RelationRange<edm4nwb::Cluster> getClusters() const;
  void addToHits(edm4nwb::CalorimeterHit);
  unsigned int hits_size() const;
  edm4nwb::CalorimeterHit getHits(unsigned int) const;
  std::vector<edm4nwb::CalorimeterHit>::const_iterator hits_begin() const;
  std::vector<edm4nwb::CalorimeterHit>::const_iterator hits_end() const;
  podio::RelationRange<edm4nwb::CalorimeterHit> getHits() const;
  void addToParticleIDs(edm4hep::ParticleID);
  unsigned int particleIDs_size() const;
  edm4hep::ParticleID getParticleIDs(unsigned int) const;
  std::vector<edm4hep::ParticleID>::const_iterator particleIDs_begin() const;
  std::vector<edm4hep::ParticleID>::const_iterator particleIDs_end() const;
  podio::RelationRange<edm4hep::ParticleID> getParticleIDs() const;
  void addToShapeParameters(float);
  unsigned int shapeParameters_size() const;
  float getShapeParameters(unsigned int) const;
  std::vector<float>::const_iterator shapeParameters_begin() const;
  std::vector<float>::const_iterator shapeParameters_end() const;
  podio::RelationRange<float> getShapeParameters() const;
  void addToHitContributions(float);
  unsigned int hitContributions_size() const;
  float getHitContributions(unsigned int) const;
  std::vector<float>::const_iterator hitContributions_begin() const;
  std::vector<float>::const_iterator hitContributions_end() const;
  podio::RelationRange<float> getHitContributions() const;
  void addToSubdetectorEnergies(float);
  unsigned int subdetectorEnergies_size() const;
  float getSubdetectorEnergies(unsigned int) const;
  std::vector<float>::const_iterator subdetectorEnergies_begin() const;
  std::vector<float>::const_iterator subdetectorEnergies_end() const;
  podio::RelationRange<float> getSubdetectorEnergies() const;



  /// check whether the object is actually available
  bool isAvailable() const;
  /// disconnect from ClusterObj instance
  void unlink() { m_obj = nullptr; }

  bool operator==(const MutableCluster& other) const { return m_obj == other.m_obj; }
  bool operator==(const Cluster& other) const;

  // less comparison operator, so that objects can be e.g. stored in sets.
  bool operator<(const MutableCluster& other) const { return m_obj < other.m_obj; }

  unsigned int id() const { return getObjectID().collectionID * 10000000 + getObjectID().index; }

  const podio::ObjectID getObjectID() const;

  friend void swap(MutableCluster& a, MutableCluster& b) {
    using std::swap;
    swap(a.m_obj, b.m_obj); // swap out the internal pointers
  }

private:
  ClusterObj* m_obj;
};

#ifdef PODIO_JSON_OUTPUT
void to_json(nlohmann::json& j, const MutableCluster& value);
#endif


} // namespace edm4nwb


#endif
