// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#ifndef EDM4NWB_CalorimeterHit_H
#define EDM4NWB_CalorimeterHit_H

#include "edm4nwb/CalorimeterHitObj.h"

#include "edm4hep/Vector3f.h"
#include <cstdint>
#include "podio/ObjectID.h"
#include <ostream>

#ifdef PODIO_JSON_OUTPUT
#include "nlohmann/json.hpp"
#endif



namespace edm4nwb {

class MutableCalorimeterHit;

/** @class CalorimeterHit
 *  Calorimeter hit
 *  @author: W. Armstrong, S. Joosten
 */
class CalorimeterHit {

  friend class MutableCalorimeterHit;
  friend class CalorimeterHitCollection;
  friend class CalorimeterHitCollectionIterator;

public:
  /// default constructor
  CalorimeterHit();
  CalorimeterHit(std::uint64_t cellID, float energy, float energyError, float time, float timeError, edm4hep::Vector3f position, edm4hep::Vector3f dimension, std::int32_t sector, std::int32_t layer, edm4hep::Vector3f local);

  /// constructor from existing CalorimeterHitObj
  CalorimeterHit(CalorimeterHitObj* obj);

  /// copy constructor
  CalorimeterHit(const CalorimeterHit& other);

  /// copy-assignment operator
  CalorimeterHit& operator=(CalorimeterHit other);

  /// create a mutable deep-copy of the object with identical relations
  MutableCalorimeterHit clone() const;

  /// destructor
  ~CalorimeterHit();


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





  /// check whether the object is actually available
  bool isAvailable() const;
  /// disconnect from CalorimeterHitObj instance
  void unlink() { m_obj = nullptr; }

  bool operator==(const CalorimeterHit& other) const { return m_obj == other.m_obj; }
  bool operator==(const MutableCalorimeterHit& other) const;

  // less comparison operator, so that objects can be e.g. stored in sets.
  bool operator<(const CalorimeterHit& other) const { return m_obj < other.m_obj; }

  unsigned int id() const { return getObjectID().collectionID * 10000000 + getObjectID().index; }

  const podio::ObjectID getObjectID() const;

  friend void swap(CalorimeterHit& a, CalorimeterHit& b) {
    using std::swap;
    swap(a.m_obj, b.m_obj); // swap out the internal pointers
  }

private:
  CalorimeterHitObj* m_obj;
};

std::ostream& operator<<(std::ostream& o, const CalorimeterHit& value);

#ifdef PODIO_JSON_OUTPUT
void to_json(nlohmann::json& j, const CalorimeterHit& value);
#endif


} // namespace edm4nwb


#endif
