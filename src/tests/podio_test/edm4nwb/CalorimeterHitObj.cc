// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#include "edm4nwb/CalorimeterHitObj.h"
namespace edm4nwb {

CalorimeterHitObj::CalorimeterHitObj() :
  ObjBase{{podio::ObjectID::untracked, podio::ObjectID::untracked}, 0},
  data()
{  }

CalorimeterHitObj::CalorimeterHitObj(const podio::ObjectID id_, CalorimeterHitData data_) :
  ObjBase{id_, 0}, data(data_)
{  }

CalorimeterHitObj::CalorimeterHitObj(const CalorimeterHitObj& other) :
  ObjBase{{podio::ObjectID::untracked, podio::ObjectID::untracked}, 0},
  data(other.data)
{
}


} // namespace edm4nwb

