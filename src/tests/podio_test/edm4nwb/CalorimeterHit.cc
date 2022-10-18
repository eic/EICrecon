// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

// datamodel specific includes
#include "edm4nwb/CalorimeterHit.h"
#include "edm4nwb/MutableCalorimeterHit.h"
#include "edm4nwb/CalorimeterHitObj.h"
#include "edm4nwb/CalorimeterHitData.h"
#include "edm4nwb/CalorimeterHitCollection.h"


#include <ostream>

namespace edm4nwb {


CalorimeterHit::CalorimeterHit() : m_obj(new CalorimeterHitObj()) {
  if (m_obj) m_obj->acquire();
}

CalorimeterHit::CalorimeterHit(std::uint64_t cellID, float energy, float energyError, float time, float timeError, edm4hep::Vector3f position, edm4hep::Vector3f dimension, std::int32_t sector, std::int32_t layer, edm4hep::Vector3f local) : m_obj(new CalorimeterHitObj()) {
  m_obj->acquire();
  m_obj->data.cellID = cellID;
  m_obj->data.energy = energy;
  m_obj->data.energyError = energyError;
  m_obj->data.time = time;
  m_obj->data.timeError = timeError;
  m_obj->data.position = position;
  m_obj->data.dimension = dimension;
  m_obj->data.sector = sector;
  m_obj->data.layer = layer;
  m_obj->data.local = local;
}

CalorimeterHit::CalorimeterHit(const CalorimeterHit& other) : m_obj(other.m_obj) {
  if (m_obj) m_obj->acquire();
}

CalorimeterHit& CalorimeterHit::operator=(CalorimeterHit other) {
  swap(*this, other);
  return *this;
}

CalorimeterHit::CalorimeterHit( CalorimeterHitObj* obj) : m_obj(obj) {
  if (m_obj) {
    m_obj->acquire();
  }
}

MutableCalorimeterHit CalorimeterHit::clone() const {
  return {new CalorimeterHitObj(*m_obj)};
}

CalorimeterHit::~CalorimeterHit() {
  if (m_obj) {
    m_obj->release();
  }
}

const std::uint64_t& CalorimeterHit::getCellID() const { return m_obj->data.cellID; }
const float& CalorimeterHit::getEnergy() const { return m_obj->data.energy; }
const float& CalorimeterHit::getEnergyError() const { return m_obj->data.energyError; }
const float& CalorimeterHit::getTime() const { return m_obj->data.time; }
const float& CalorimeterHit::getTimeError() const { return m_obj->data.timeError; }
const edm4hep::Vector3f& CalorimeterHit::getPosition() const { return m_obj->data.position; }
const edm4hep::Vector3f& CalorimeterHit::getDimension() const { return m_obj->data.dimension; }
const std::int32_t& CalorimeterHit::getSector() const { return m_obj->data.sector; }
const std::int32_t& CalorimeterHit::getLayer() const { return m_obj->data.layer; }
const edm4hep::Vector3f& CalorimeterHit::getLocal() const { return m_obj->data.local; }






bool CalorimeterHit::isAvailable() const {
  if (m_obj) {
    return true;
  }
  return false;
}

const podio::ObjectID CalorimeterHit::getObjectID() const {
  if (m_obj) {
    return m_obj->id;
  }
  return podio::ObjectID{podio::ObjectID::invalid, podio::ObjectID::invalid};
}

bool CalorimeterHit::operator==(const MutableCalorimeterHit& other) const {
  return m_obj == other.m_obj;
}

std::ostream& operator<<(std::ostream& o, const CalorimeterHit& value) {
  o << " id: " << value.id() << '\n';
  o << " cellID : " << value.getCellID() << '\n';
  o << " energy : " << value.getEnergy() << '\n';
  o << " energyError : " << value.getEnergyError() << '\n';
  o << " time : " << value.getTime() << '\n';
  o << " timeError : " << value.getTimeError() << '\n';
  o << " position : " << value.getPosition() << '\n';
  o << " dimension : " << value.getDimension() << '\n';
  o << " sector : " << value.getSector() << '\n';
  o << " layer : " << value.getLayer() << '\n';
  o << " local : " << value.getLocal() << '\n';



  return o;
}

#ifdef PODIO_JSON_OUTPUT
void to_json(nlohmann::json& j, const CalorimeterHit& value) {
  j = nlohmann::json{
    {"cellID", value.getCellID()}
    ,{"energy", value.getEnergy()}
    ,{"energyError", value.getEnergyError()}
    ,{"time", value.getTime()}
    ,{"timeError", value.getTimeError()}
    ,{"position", value.getPosition()}
    ,{"dimension", value.getDimension()}
    ,{"sector", value.getSector()}
    ,{"layer", value.getLayer()}
    ,{"local", value.getLocal()}
  };


}
#endif


} // namespace edm4nwb

