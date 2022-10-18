// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

// datamodel specific includes
#include "edm4nwb/CalorimeterHit.h"
#include "edm4nwb/MutableCalorimeterHit.h"
#include "edm4nwb/CalorimeterHitObj.h"
#include "edm4nwb/CalorimeterHitData.h"
#include "edm4nwb/CalorimeterHitCollection.h"


#include <ostream>

namespace edm4nwb {


MutableCalorimeterHit::MutableCalorimeterHit() : m_obj(new CalorimeterHitObj()) {
  if (m_obj) m_obj->acquire();
}

MutableCalorimeterHit::MutableCalorimeterHit(std::uint64_t cellID, float energy, float energyError, float time, float timeError, edm4hep::Vector3f position, edm4hep::Vector3f dimension, std::int32_t sector, std::int32_t layer, edm4hep::Vector3f local) : m_obj(new CalorimeterHitObj()) {
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

MutableCalorimeterHit::MutableCalorimeterHit(const MutableCalorimeterHit& other) : m_obj(other.m_obj) {
  if (m_obj) m_obj->acquire();
}

MutableCalorimeterHit& MutableCalorimeterHit::operator=(MutableCalorimeterHit other) {
  swap(*this, other);
  return *this;
}

MutableCalorimeterHit::MutableCalorimeterHit( CalorimeterHitObj* obj) : m_obj(obj) {
  if (m_obj) {
    m_obj->acquire();
  }
}

MutableCalorimeterHit MutableCalorimeterHit::clone() const {
  return {new CalorimeterHitObj(*m_obj)};
}

MutableCalorimeterHit::~MutableCalorimeterHit() {
  if (m_obj) {
    m_obj->release();
  }
}
MutableCalorimeterHit::operator CalorimeterHit() const { return CalorimeterHit(m_obj); }

const std::uint64_t& MutableCalorimeterHit::getCellID() const { return m_obj->data.cellID; }
const float& MutableCalorimeterHit::getEnergy() const { return m_obj->data.energy; }
const float& MutableCalorimeterHit::getEnergyError() const { return m_obj->data.energyError; }
const float& MutableCalorimeterHit::getTime() const { return m_obj->data.time; }
const float& MutableCalorimeterHit::getTimeError() const { return m_obj->data.timeError; }
const edm4hep::Vector3f& MutableCalorimeterHit::getPosition() const { return m_obj->data.position; }
const edm4hep::Vector3f& MutableCalorimeterHit::getDimension() const { return m_obj->data.dimension; }
const std::int32_t& MutableCalorimeterHit::getSector() const { return m_obj->data.sector; }
const std::int32_t& MutableCalorimeterHit::getLayer() const { return m_obj->data.layer; }
const edm4hep::Vector3f& MutableCalorimeterHit::getLocal() const { return m_obj->data.local; }


void MutableCalorimeterHit::setCellID(std::uint64_t value) { m_obj->data.cellID = value; }
void MutableCalorimeterHit::setEnergy(float value) { m_obj->data.energy = value; }
void MutableCalorimeterHit::setEnergyError(float value) { m_obj->data.energyError = value; }
void MutableCalorimeterHit::setTime(float value) { m_obj->data.time = value; }
void MutableCalorimeterHit::setTimeError(float value) { m_obj->data.timeError = value; }
void MutableCalorimeterHit::setPosition(edm4hep::Vector3f value) { m_obj->data.position = value; }
edm4hep::Vector3f& MutableCalorimeterHit::position() { return m_obj->data.position; }
void MutableCalorimeterHit::setDimension(edm4hep::Vector3f value) { m_obj->data.dimension = value; }
edm4hep::Vector3f& MutableCalorimeterHit::dimension() { return m_obj->data.dimension; }
void MutableCalorimeterHit::setSector(std::int32_t value) { m_obj->data.sector = value; }
void MutableCalorimeterHit::setLayer(std::int32_t value) { m_obj->data.layer = value; }
void MutableCalorimeterHit::setLocal(edm4hep::Vector3f value) { m_obj->data.local = value; }
edm4hep::Vector3f& MutableCalorimeterHit::local() { return m_obj->data.local; }







bool MutableCalorimeterHit::isAvailable() const {
  if (m_obj) {
    return true;
  }
  return false;
}

const podio::ObjectID MutableCalorimeterHit::getObjectID() const {
  if (m_obj) {
    return m_obj->id;
  }
  return podio::ObjectID{podio::ObjectID::invalid, podio::ObjectID::invalid};
}

bool MutableCalorimeterHit::operator==(const CalorimeterHit& other) const {
  return m_obj == other.m_obj;
}

#ifdef PODIO_JSON_OUTPUT
void to_json(nlohmann::json& j, const MutableCalorimeterHit& value) {
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

