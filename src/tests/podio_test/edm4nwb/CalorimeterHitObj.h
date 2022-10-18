// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#ifndef EDM4NWB_CalorimeterHitOBJ_H
#define EDM4NWB_CalorimeterHitOBJ_H

// data model specific includes
#include "edm4nwb/CalorimeterHitData.h"

#include "podio/ObjBase.h"



namespace edm4nwb {

class CalorimeterHit;

class CalorimeterHitObj : public podio::ObjBase {
public:
  /// constructor
  CalorimeterHitObj();
  /// copy constructor (does a deep-copy of relation containers)
  CalorimeterHitObj(const CalorimeterHitObj&);
  /// constructor from ObjectID and CalorimeterHitData
  /// does not initialize the internal relation containers
  CalorimeterHitObj(const podio::ObjectID id, CalorimeterHitData data);
  /// No assignment operator
  CalorimeterHitObj& operator=(const CalorimeterHitObj&) = delete;
  virtual ~CalorimeterHitObj() = default;

public:
  CalorimeterHitData data;
};

} // namespace edm4nwb


#endif
