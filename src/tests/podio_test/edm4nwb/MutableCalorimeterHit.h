// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#ifndef EDM4EIC_MutableCalorimeterHit_H
#define EDM4EIC_MutableCalorimeterHit_H

#include "edm4nwb/CalorimeterHitObj.h"
// Make the immutable class available from its mutable version but not vice versa
#include "edm4nwb/CalorimeterHit.h"

#include "edm4hep/Vector3f.h"
#include <cstdint>
#include "podio/ObjectID.h"
#include <ostream>

#ifdef PODIO_JSON_OUTPUT
#include "nlohmann/json.hpp"
#endif



namespace edm4nwb {


/** @class MutableCalorimeterHit
 *  Calorimeter hit
 *  @author: W. Armstrong, S. Joosten
 */
class MutableCalorimeterHit {

  friend class CalorimeterHitCollection;
  friend class CalorimeterHitMutableCollectionIterator;
  friend class CalorimeterHit;

public:

  /// default constructor
  MutableCalorimeterHit();
  MutableCalorimeterHit(std::uint64_t cellID, float energy, float energyError, float time, float timeError, edm4hep::Vector3f position, edm4hep::Vector3f dimension, std::int32_t sector, std::int32_t layer, edm4hep::Vector3f local);

  /// constructor from existing CalorimeterHitObj
  MutableCalorimeterHit(CalorimeterHitObj* obj);

  /// copy constructor
  MutableCalorimeterHit(const MutableCalorimeterHit& other);

  /// copy-assignment operator
  MutableCalorimeterHit& operator=(MutableCalorimeterHit other);

  /// create a mutable deep-copy of the object with identical relations
  MutableCalorimeterHit clone() const;

  /// destructor
  ~MutableCalorimeterHit();

  /// conversion to const object
  operator CalorimeterHit() const;

public:

  /// Access the The detector specific (geometrical) cell id.
  const std::uint64_t& getCellID() const;

  /// Access the The energy for this hit in [GeV].
  const float& getEnergy() const;

  /// Access the Error on energy [GeV].
  const float& getEnergyError() const;

  /// Access the The time of the hit in [ns].
  const float& getTime() const;

  /// Access the Error on the time
  const float& getTimeError() const;

  /// Access the The global position of the hit in world coordinates [mm].
  const edm4hep::Vector3f& getPosition() const;

  /// Access the The dimension information of the cell [mm].
  const edm4hep::Vector3f& getDimension() const;

  /// Access the Sector that this hit occured in
  const std::int32_t& getSector() const;

  /// Access the Layer that the hit occured in
  const std::int32_t& getLayer() const;

  /// Access the The local coordinates of the hit in the detector segment [mm].
  const edm4hep::Vector3f& getLocal() const;



  /// Set the The detector specific (geometrical) cell id.
  void setCellID(std::uint64_t value);

  /// Set the The energy for this hit in [GeV].
  void setEnergy(float value);

  /// Set the Error on energy [GeV].
  void setEnergyError(float value);

  /// Set the The time of the hit in [ns].
  void setTime(float value);

  /// Set the Error on the time
  void setTimeError(float value);

  /// Set the The global position of the hit in world coordinates [mm].
  void setPosition(edm4hep::Vector3f value);
  /// Get reference to The global position of the hit in world coordinates [mm].
  edm4hep::Vector3f& position();

  /// Set the The dimension information of the cell [mm].
  void setDimension(edm4hep::Vector3f value);
  /// Get reference to The dimension information of the cell [mm].
  edm4hep::Vector3f& dimension();

  /// Set the Sector that this hit occured in
  void setSector(std::int32_t value);

  /// Set the Layer that the hit occured in
  void setLayer(std::int32_t value);

  /// Set the The local coordinates of the hit in the detector segment [mm].
  void setLocal(edm4hep::Vector3f value);
  /// Get reference to The local coordinates of the hit in the detector segment [mm].
  edm4hep::Vector3f& local();






  /// check whether the object is actually available
  bool isAvailable() const;
  /// disconnect from CalorimeterHitObj instance
  void unlink() { m_obj = nullptr; }

  bool operator==(const MutableCalorimeterHit& other) const { return m_obj == other.m_obj; }
  bool operator==(const CalorimeterHit& other) const;

  // less comparison operator, so that objects can be e.g. stored in sets.
  bool operator<(const MutableCalorimeterHit& other) const { return m_obj < other.m_obj; }

  unsigned int id() const { return getObjectID().collectionID * 10000000 + getObjectID().index; }

  const podio::ObjectID getObjectID() const;

  friend void swap(MutableCalorimeterHit& a, MutableCalorimeterHit& b) {
    using std::swap;
    swap(a.m_obj, b.m_obj); // swap out the internal pointers
  }

private:
  CalorimeterHitObj* m_obj;
};

#ifdef PODIO_JSON_OUTPUT
void to_json(nlohmann::json& j, const MutableCalorimeterHit& value);
#endif


} // namespace edm4nwb


#endif
