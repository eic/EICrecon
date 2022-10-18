// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#ifndef EDM4NWB_CalorimeterHitDATA_H
#define EDM4NWB_CalorimeterHitDATA_H

#include "edm4hep/Vector3f.h"
#include <cstdint>

namespace edm4nwb {


/** @class CalorimeterHitData
 *  Calorimeter hit
 *  @author: W. Armstrong, S. Joosten
 */
class CalorimeterHitData {
public:
  std::uint64_t cellID{}; ///< The detector specific (geometrical) cell id.
  float energy{}; ///< The energy for this hit in [GeV].
  float energyError{}; ///< Error on energy [GeV].
  float time{}; ///< The time of the hit in [ns].
  float timeError{}; ///< Error on the time
  ::edm4hep::Vector3f position{}; ///< The global position of the hit in world coordinates [mm].
  ::edm4hep::Vector3f dimension{}; ///< The dimension information of the cell [mm].
  std::int32_t sector{}; ///< Sector that this hit occured in
  std::int32_t layer{}; ///< Layer that the hit occured in
  ::edm4hep::Vector3f local{}; ///< The local coordinates of the hit in the detector segment [mm].

};

} // namespace edm4nwb


#endif
