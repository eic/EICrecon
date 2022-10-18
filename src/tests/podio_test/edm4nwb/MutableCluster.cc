// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

// datamodel specific includes
#include "edm4nwb/Cluster.h"
#include "edm4nwb/MutableCluster.h"
#include "edm4nwb/ClusterObj.h"
#include "edm4nwb/ClusterData.h"
#include "edm4nwb/ClusterCollection.h"


#include <ostream>

namespace edm4nwb {


MutableCluster::MutableCluster() : m_obj(new ClusterObj()) {
  if (m_obj) m_obj->acquire();
}

MutableCluster::MutableCluster(std::int32_t type, float energy, float energyError, float time, float timeError, std::uint32_t nhits, edm4hep::Vector3f position, edm4eic::Cov3f positionError, float intrinsicTheta, float intrinsicPhi, edm4eic::Cov2f intrinsicDirectionError) : m_obj(new ClusterObj()) {
  m_obj->acquire();
  m_obj->data.type = type;
  m_obj->data.energy = energy;
  m_obj->data.energyError = energyError;
  m_obj->data.time = time;
  m_obj->data.timeError = timeError;
  m_obj->data.nhits = nhits;
  m_obj->data.position = position;
  m_obj->data.positionError = positionError;
  m_obj->data.intrinsicTheta = intrinsicTheta;
  m_obj->data.intrinsicPhi = intrinsicPhi;
  m_obj->data.intrinsicDirectionError = intrinsicDirectionError;
}

MutableCluster::MutableCluster(const MutableCluster& other) : m_obj(other.m_obj) {
  if (m_obj) m_obj->acquire();
}

MutableCluster& MutableCluster::operator=(MutableCluster other) {
  swap(*this, other);
  return *this;
}

MutableCluster::MutableCluster( ClusterObj* obj) : m_obj(obj) {
  if (m_obj) {
    m_obj->acquire();
  }
}

MutableCluster MutableCluster::clone() const {
  return {new ClusterObj(*m_obj)};
}

MutableCluster::~MutableCluster() {
  if (m_obj) {
    m_obj->release();
  }
}
MutableCluster::operator Cluster() const { return Cluster(m_obj); }

const std::int32_t& MutableCluster::getType() const { return m_obj->data.type; }
const float& MutableCluster::getEnergy() const { return m_obj->data.energy; }
const float& MutableCluster::getEnergyError() const { return m_obj->data.energyError; }
const float& MutableCluster::getTime() const { return m_obj->data.time; }
const float& MutableCluster::getTimeError() const { return m_obj->data.timeError; }
const std::uint32_t& MutableCluster::getNhits() const { return m_obj->data.nhits; }
const edm4hep::Vector3f& MutableCluster::getPosition() const { return m_obj->data.position; }
const edm4eic::Cov3f& MutableCluster::getPositionError() const { return m_obj->data.positionError; }
const float& MutableCluster::getIntrinsicTheta() const { return m_obj->data.intrinsicTheta; }
const float& MutableCluster::getIntrinsicPhi() const { return m_obj->data.intrinsicPhi; }
const edm4eic::Cov2f& MutableCluster::getIntrinsicDirectionError() const { return m_obj->data.intrinsicDirectionError; }


void MutableCluster::setType(std::int32_t value) { m_obj->data.type = value; }
void MutableCluster::setEnergy(float value) { m_obj->data.energy = value; }
void MutableCluster::setEnergyError(float value) { m_obj->data.energyError = value; }
void MutableCluster::setTime(float value) { m_obj->data.time = value; }
void MutableCluster::setTimeError(float value) { m_obj->data.timeError = value; }
void MutableCluster::setNhits(std::uint32_t value) { m_obj->data.nhits = value; }
void MutableCluster::setPosition(edm4hep::Vector3f value) { m_obj->data.position = value; }
edm4hep::Vector3f& MutableCluster::position() { return m_obj->data.position; }
void MutableCluster::setPositionError(edm4eic::Cov3f value) { m_obj->data.positionError = value; }
edm4eic::Cov3f& MutableCluster::positionError() { return m_obj->data.positionError; }
void MutableCluster::setIntrinsicTheta(float value) { m_obj->data.intrinsicTheta = value; }
void MutableCluster::setIntrinsicPhi(float value) { m_obj->data.intrinsicPhi = value; }
void MutableCluster::setIntrinsicDirectionError(edm4eic::Cov2f value) { m_obj->data.intrinsicDirectionError = value; }
edm4eic::Cov2f& MutableCluster::intrinsicDirectionError() { return m_obj->data.intrinsicDirectionError; }


void MutableCluster::addToClusters(edm4nwb::Cluster component) {
  m_obj->m_clusters->push_back(component);
  m_obj->data.clusters_end++;
}

std::vector<edm4nwb::Cluster>::const_iterator MutableCluster::clusters_begin() const {
  auto ret_value = m_obj->m_clusters->begin();
  std::advance(ret_value, m_obj->data.clusters_begin);
  return ret_value;
}

std::vector<edm4nwb::Cluster>::const_iterator MutableCluster::clusters_end() const {
  auto ret_value = m_obj->m_clusters->begin();
  std::advance(ret_value, m_obj->data.clusters_end);
  return ret_value;
}

unsigned int MutableCluster::clusters_size() const {
  return m_obj->data.clusters_end - m_obj->data.clusters_begin;
}

edm4nwb::Cluster MutableCluster::getClusters(unsigned int index) const {
  if (clusters_size() > index) {
    return m_obj->m_clusters->at(m_obj->data.clusters_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<edm4nwb::Cluster> MutableCluster::getClusters() const {
  auto begin = m_obj->m_clusters->begin();
  std::advance(begin, m_obj->data.clusters_begin);
  auto end = m_obj->m_clusters->begin();
  std::advance(end, m_obj->data.clusters_end);
  return {begin, end};
}

void MutableCluster::addToHits(edm4nwb::CalorimeterHit component) {
  m_obj->m_hits->push_back(component);
  m_obj->data.hits_end++;
}

std::vector<edm4nwb::CalorimeterHit>::const_iterator MutableCluster::hits_begin() const {
  auto ret_value = m_obj->m_hits->begin();
  std::advance(ret_value, m_obj->data.hits_begin);
  return ret_value;
}

std::vector<edm4nwb::CalorimeterHit>::const_iterator MutableCluster::hits_end() const {
  auto ret_value = m_obj->m_hits->begin();
  std::advance(ret_value, m_obj->data.hits_end);
  return ret_value;
}

unsigned int MutableCluster::hits_size() const {
  return m_obj->data.hits_end - m_obj->data.hits_begin;
}

edm4nwb::CalorimeterHit MutableCluster::getHits(unsigned int index) const {
  if (hits_size() > index) {
    return m_obj->m_hits->at(m_obj->data.hits_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<edm4nwb::CalorimeterHit> MutableCluster::getHits() const {
  auto begin = m_obj->m_hits->begin();
  std::advance(begin, m_obj->data.hits_begin);
  auto end = m_obj->m_hits->begin();
  std::advance(end, m_obj->data.hits_end);
  return {begin, end};
}

void MutableCluster::addToParticleIDs(edm4hep::ParticleID component) {
  m_obj->m_particleIDs->push_back(component);
  m_obj->data.particleIDs_end++;
}

std::vector<edm4hep::ParticleID>::const_iterator MutableCluster::particleIDs_begin() const {
  auto ret_value = m_obj->m_particleIDs->begin();
  std::advance(ret_value, m_obj->data.particleIDs_begin);
  return ret_value;
}

std::vector<edm4hep::ParticleID>::const_iterator MutableCluster::particleIDs_end() const {
  auto ret_value = m_obj->m_particleIDs->begin();
  std::advance(ret_value, m_obj->data.particleIDs_end);
  return ret_value;
}

unsigned int MutableCluster::particleIDs_size() const {
  return m_obj->data.particleIDs_end - m_obj->data.particleIDs_begin;
}

edm4hep::ParticleID MutableCluster::getParticleIDs(unsigned int index) const {
  if (particleIDs_size() > index) {
    return m_obj->m_particleIDs->at(m_obj->data.particleIDs_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<edm4hep::ParticleID> MutableCluster::getParticleIDs() const {
  auto begin = m_obj->m_particleIDs->begin();
  std::advance(begin, m_obj->data.particleIDs_begin);
  auto end = m_obj->m_particleIDs->begin();
  std::advance(end, m_obj->data.particleIDs_end);
  return {begin, end};
}

void MutableCluster::addToShapeParameters(float component) {
  m_obj->m_shapeParameters->push_back(component);
  m_obj->data.shapeParameters_end++;
}

std::vector<float>::const_iterator MutableCluster::shapeParameters_begin() const {
  auto ret_value = m_obj->m_shapeParameters->begin();
  std::advance(ret_value, m_obj->data.shapeParameters_begin);
  return ret_value;
}

std::vector<float>::const_iterator MutableCluster::shapeParameters_end() const {
  auto ret_value = m_obj->m_shapeParameters->begin();
  std::advance(ret_value, m_obj->data.shapeParameters_end);
  return ret_value;
}

unsigned int MutableCluster::shapeParameters_size() const {
  return m_obj->data.shapeParameters_end - m_obj->data.shapeParameters_begin;
}

float MutableCluster::getShapeParameters(unsigned int index) const {
  if (shapeParameters_size() > index) {
    return m_obj->m_shapeParameters->at(m_obj->data.shapeParameters_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<float> MutableCluster::getShapeParameters() const {
  auto begin = m_obj->m_shapeParameters->begin();
  std::advance(begin, m_obj->data.shapeParameters_begin);
  auto end = m_obj->m_shapeParameters->begin();
  std::advance(end, m_obj->data.shapeParameters_end);
  return {begin, end};
}

void MutableCluster::addToHitContributions(float component) {
  m_obj->m_hitContributions->push_back(component);
  m_obj->data.hitContributions_end++;
}

std::vector<float>::const_iterator MutableCluster::hitContributions_begin() const {
  auto ret_value = m_obj->m_hitContributions->begin();
  std::advance(ret_value, m_obj->data.hitContributions_begin);
  return ret_value;
}

std::vector<float>::const_iterator MutableCluster::hitContributions_end() const {
  auto ret_value = m_obj->m_hitContributions->begin();
  std::advance(ret_value, m_obj->data.hitContributions_end);
  return ret_value;
}

unsigned int MutableCluster::hitContributions_size() const {
  return m_obj->data.hitContributions_end - m_obj->data.hitContributions_begin;
}

float MutableCluster::getHitContributions(unsigned int index) const {
  if (hitContributions_size() > index) {
    return m_obj->m_hitContributions->at(m_obj->data.hitContributions_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<float> MutableCluster::getHitContributions() const {
  auto begin = m_obj->m_hitContributions->begin();
  std::advance(begin, m_obj->data.hitContributions_begin);
  auto end = m_obj->m_hitContributions->begin();
  std::advance(end, m_obj->data.hitContributions_end);
  return {begin, end};
}

void MutableCluster::addToSubdetectorEnergies(float component) {
  m_obj->m_subdetectorEnergies->push_back(component);
  m_obj->data.subdetectorEnergies_end++;
}

std::vector<float>::const_iterator MutableCluster::subdetectorEnergies_begin() const {
  auto ret_value = m_obj->m_subdetectorEnergies->begin();
  std::advance(ret_value, m_obj->data.subdetectorEnergies_begin);
  return ret_value;
}

std::vector<float>::const_iterator MutableCluster::subdetectorEnergies_end() const {
  auto ret_value = m_obj->m_subdetectorEnergies->begin();
  std::advance(ret_value, m_obj->data.subdetectorEnergies_end);
  return ret_value;
}

unsigned int MutableCluster::subdetectorEnergies_size() const {
  return m_obj->data.subdetectorEnergies_end - m_obj->data.subdetectorEnergies_begin;
}

float MutableCluster::getSubdetectorEnergies(unsigned int index) const {
  if (subdetectorEnergies_size() > index) {
    return m_obj->m_subdetectorEnergies->at(m_obj->data.subdetectorEnergies_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<float> MutableCluster::getSubdetectorEnergies() const {
  auto begin = m_obj->m_subdetectorEnergies->begin();
  std::advance(begin, m_obj->data.subdetectorEnergies_begin);
  auto end = m_obj->m_subdetectorEnergies->begin();
  std::advance(end, m_obj->data.subdetectorEnergies_end);
  return {begin, end};
}






bool MutableCluster::isAvailable() const {
  if (m_obj) {
    return true;
  }
  return false;
}

const podio::ObjectID MutableCluster::getObjectID() const {
  if (m_obj) {
    return m_obj->id;
  }
  return podio::ObjectID{podio::ObjectID::invalid, podio::ObjectID::invalid};
}

bool MutableCluster::operator==(const Cluster& other) const {
  return m_obj == other.m_obj;
}

#ifdef PODIO_JSON_OUTPUT
void to_json(nlohmann::json& j, const MutableCluster& value) {
  j = nlohmann::json{
    {"type", value.getType()}
    ,{"energy", value.getEnergy()}
    ,{"energyError", value.getEnergyError()}
    ,{"time", value.getTime()}
    ,{"timeError", value.getTimeError()}
    ,{"nhits", value.getNhits()}
    ,{"position", value.getPosition()}
    ,{"positionError", value.getPositionError()}
    ,{"intrinsicTheta", value.getIntrinsicTheta()}
    ,{"intrinsicPhi", value.getIntrinsicPhi()}
    ,{"intrinsicDirectionError", value.getIntrinsicDirectionError()}
    ,{"shapeParameters", value.getShapeParameters()}
    ,{"hitContributions", value.getHitContributions()}
    ,{"subdetectorEnergies", value.getSubdetectorEnergies()}
  };


  j["clusters"] = nlohmann::json::array();
  for (const auto& v : value.getClusters()) {
    j["clusters"].emplace_back(nlohmann::json{
      {"collectionID", v.getObjectID().collectionID },
      {"index", v.getObjectID().index }});
  }

  j["hits"] = nlohmann::json::array();
  for (const auto& v : value.getHits()) {
    j["hits"].emplace_back(nlohmann::json{
      {"collectionID", v.getObjectID().collectionID },
      {"index", v.getObjectID().index }});
  }

  j["particleIDs"] = nlohmann::json::array();
  for (const auto& v : value.getParticleIDs()) {
    j["particleIDs"].emplace_back(nlohmann::json{
      {"collectionID", v.getObjectID().collectionID },
      {"index", v.getObjectID().index }});
  }

}
#endif


} // namespace edm4nwb

