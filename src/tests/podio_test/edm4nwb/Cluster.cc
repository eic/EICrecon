// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

// datamodel specific includes
#include "edm4nwb/Cluster.h"
#include "edm4nwb/MutableCluster.h"
#include "edm4nwb/ClusterObj.h"
#include "edm4nwb/ClusterData.h"
#include "edm4nwb/ClusterCollection.h"


#include <ostream>

namespace edm4nwb {


Cluster::Cluster() : m_obj(new ClusterObj()) {
  if (m_obj) m_obj->acquire();
}

Cluster::Cluster(std::int32_t type, float energy, float energyError, float time, float timeError, std::uint32_t nhits, edm4hep::Vector3f position, edm4eic::Cov3f positionError, float intrinsicTheta, float intrinsicPhi, edm4eic::Cov2f intrinsicDirectionError) : m_obj(new ClusterObj()) {
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

Cluster::Cluster(const Cluster& other) : m_obj(other.m_obj) {
  if (m_obj) m_obj->acquire();
}

Cluster& Cluster::operator=(Cluster other) {
  swap(*this, other);
  return *this;
}

Cluster::Cluster( ClusterObj* obj) : m_obj(obj) {
  if (m_obj) {
    m_obj->acquire();
  }
}

MutableCluster Cluster::clone() const {
  return {new ClusterObj(*m_obj)};
}

Cluster::~Cluster() {
  if (m_obj) {
    m_obj->release();
  }
}

const std::int32_t& Cluster::getType() const { return m_obj->data.type; }
const float& Cluster::getEnergy() const { return m_obj->data.energy; }
const float& Cluster::getEnergyError() const { return m_obj->data.energyError; }
const float& Cluster::getTime() const { return m_obj->data.time; }
const float& Cluster::getTimeError() const { return m_obj->data.timeError; }
const std::uint32_t& Cluster::getNhits() const { return m_obj->data.nhits; }
const edm4hep::Vector3f& Cluster::getPosition() const { return m_obj->data.position; }
const edm4eic::Cov3f& Cluster::getPositionError() const { return m_obj->data.positionError; }
const float& Cluster::getIntrinsicTheta() const { return m_obj->data.intrinsicTheta; }
const float& Cluster::getIntrinsicPhi() const { return m_obj->data.intrinsicPhi; }
const edm4eic::Cov2f& Cluster::getIntrinsicDirectionError() const { return m_obj->data.intrinsicDirectionError; }



std::vector<edm4nwb::Cluster>::const_iterator Cluster::clusters_begin() const {
  auto ret_value = m_obj->m_clusters->begin();
  std::advance(ret_value, m_obj->data.clusters_begin);
  return ret_value;
}

std::vector<edm4nwb::Cluster>::const_iterator Cluster::clusters_end() const {
  auto ret_value = m_obj->m_clusters->begin();
  std::advance(ret_value, m_obj->data.clusters_end);
  return ret_value;
}

unsigned int Cluster::clusters_size() const {
  return m_obj->data.clusters_end - m_obj->data.clusters_begin;
}

edm4nwb::Cluster Cluster::getClusters(unsigned int index) const {
  if (clusters_size() > index) {
    return m_obj->m_clusters->at(m_obj->data.clusters_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<edm4nwb::Cluster> Cluster::getClusters() const {
  auto begin = m_obj->m_clusters->begin();
  std::advance(begin, m_obj->data.clusters_begin);
  auto end = m_obj->m_clusters->begin();
  std::advance(end, m_obj->data.clusters_end);
  return {begin, end};
}


std::vector<edm4nwb::CalorimeterHit>::const_iterator Cluster::hits_begin() const {
  auto ret_value = m_obj->m_hits->begin();
  std::advance(ret_value, m_obj->data.hits_begin);
  return ret_value;
}

std::vector<edm4nwb::CalorimeterHit>::const_iterator Cluster::hits_end() const {
  auto ret_value = m_obj->m_hits->begin();
  std::advance(ret_value, m_obj->data.hits_end);
  return ret_value;
}

unsigned int Cluster::hits_size() const {
  return m_obj->data.hits_end - m_obj->data.hits_begin;
}

edm4nwb::CalorimeterHit Cluster::getHits(unsigned int index) const {
  if (hits_size() > index) {
    return m_obj->m_hits->at(m_obj->data.hits_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<edm4nwb::CalorimeterHit> Cluster::getHits() const {
  auto begin = m_obj->m_hits->begin();
  std::advance(begin, m_obj->data.hits_begin);
  auto end = m_obj->m_hits->begin();
  std::advance(end, m_obj->data.hits_end);
  return {begin, end};
}


std::vector<edm4hep::ParticleID>::const_iterator Cluster::particleIDs_begin() const {
  auto ret_value = m_obj->m_particleIDs->begin();
  std::advance(ret_value, m_obj->data.particleIDs_begin);
  return ret_value;
}

std::vector<edm4hep::ParticleID>::const_iterator Cluster::particleIDs_end() const {
  auto ret_value = m_obj->m_particleIDs->begin();
  std::advance(ret_value, m_obj->data.particleIDs_end);
  return ret_value;
}

unsigned int Cluster::particleIDs_size() const {
  return m_obj->data.particleIDs_end - m_obj->data.particleIDs_begin;
}

edm4hep::ParticleID Cluster::getParticleIDs(unsigned int index) const {
  if (particleIDs_size() > index) {
    return m_obj->m_particleIDs->at(m_obj->data.particleIDs_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<edm4hep::ParticleID> Cluster::getParticleIDs() const {
  auto begin = m_obj->m_particleIDs->begin();
  std::advance(begin, m_obj->data.particleIDs_begin);
  auto end = m_obj->m_particleIDs->begin();
  std::advance(end, m_obj->data.particleIDs_end);
  return {begin, end};
}


std::vector<float>::const_iterator Cluster::shapeParameters_begin() const {
  auto ret_value = m_obj->m_shapeParameters->begin();
  std::advance(ret_value, m_obj->data.shapeParameters_begin);
  return ret_value;
}

std::vector<float>::const_iterator Cluster::shapeParameters_end() const {
  auto ret_value = m_obj->m_shapeParameters->begin();
  std::advance(ret_value, m_obj->data.shapeParameters_end);
  return ret_value;
}

unsigned int Cluster::shapeParameters_size() const {
  return m_obj->data.shapeParameters_end - m_obj->data.shapeParameters_begin;
}

float Cluster::getShapeParameters(unsigned int index) const {
  if (shapeParameters_size() > index) {
    return m_obj->m_shapeParameters->at(m_obj->data.shapeParameters_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<float> Cluster::getShapeParameters() const {
  auto begin = m_obj->m_shapeParameters->begin();
  std::advance(begin, m_obj->data.shapeParameters_begin);
  auto end = m_obj->m_shapeParameters->begin();
  std::advance(end, m_obj->data.shapeParameters_end);
  return {begin, end};
}


std::vector<float>::const_iterator Cluster::hitContributions_begin() const {
  auto ret_value = m_obj->m_hitContributions->begin();
  std::advance(ret_value, m_obj->data.hitContributions_begin);
  return ret_value;
}

std::vector<float>::const_iterator Cluster::hitContributions_end() const {
  auto ret_value = m_obj->m_hitContributions->begin();
  std::advance(ret_value, m_obj->data.hitContributions_end);
  return ret_value;
}

unsigned int Cluster::hitContributions_size() const {
  return m_obj->data.hitContributions_end - m_obj->data.hitContributions_begin;
}

float Cluster::getHitContributions(unsigned int index) const {
  if (hitContributions_size() > index) {
    return m_obj->m_hitContributions->at(m_obj->data.hitContributions_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<float> Cluster::getHitContributions() const {
  auto begin = m_obj->m_hitContributions->begin();
  std::advance(begin, m_obj->data.hitContributions_begin);
  auto end = m_obj->m_hitContributions->begin();
  std::advance(end, m_obj->data.hitContributions_end);
  return {begin, end};
}


std::vector<float>::const_iterator Cluster::subdetectorEnergies_begin() const {
  auto ret_value = m_obj->m_subdetectorEnergies->begin();
  std::advance(ret_value, m_obj->data.subdetectorEnergies_begin);
  return ret_value;
}

std::vector<float>::const_iterator Cluster::subdetectorEnergies_end() const {
  auto ret_value = m_obj->m_subdetectorEnergies->begin();
  std::advance(ret_value, m_obj->data.subdetectorEnergies_end);
  return ret_value;
}

unsigned int Cluster::subdetectorEnergies_size() const {
  return m_obj->data.subdetectorEnergies_end - m_obj->data.subdetectorEnergies_begin;
}

float Cluster::getSubdetectorEnergies(unsigned int index) const {
  if (subdetectorEnergies_size() > index) {
    return m_obj->m_subdetectorEnergies->at(m_obj->data.subdetectorEnergies_begin + index);
  }
  throw std::out_of_range("index out of bounds for existing references");
}

podio::RelationRange<float> Cluster::getSubdetectorEnergies() const {
  auto begin = m_obj->m_subdetectorEnergies->begin();
  std::advance(begin, m_obj->data.subdetectorEnergies_begin);
  auto end = m_obj->m_subdetectorEnergies->begin();
  std::advance(end, m_obj->data.subdetectorEnergies_end);
  return {begin, end};
}





bool Cluster::isAvailable() const {
  if (m_obj) {
    return true;
  }
  return false;
}

const podio::ObjectID Cluster::getObjectID() const {
  if (m_obj) {
    return m_obj->id;
  }
  return podio::ObjectID{podio::ObjectID::invalid, podio::ObjectID::invalid};
}

bool Cluster::operator==(const MutableCluster& other) const {
  return m_obj == other.m_obj;
}

std::ostream& operator<<(std::ostream& o, const Cluster& value) {
  o << " id: " << value.id() << '\n';
  o << " type : " << value.getType() << '\n';
  o << " energy : " << value.getEnergy() << '\n';
  o << " energyError : " << value.getEnergyError() << '\n';
  o << " time : " << value.getTime() << '\n';
  o << " timeError : " << value.getTimeError() << '\n';
  o << " nhits : " << value.getNhits() << '\n';
  o << " position : " << value.getPosition() << '\n';
  o << " positionError : " << value.getPositionError() << '\n';
  o << " intrinsicTheta : " << value.getIntrinsicTheta() << '\n';
  o << " intrinsicPhi : " << value.getIntrinsicPhi() << '\n';
  o << " intrinsicDirectionError : " << value.getIntrinsicDirectionError() << '\n';


  o << " clusters : ";
  for (unsigned i = 0; i < value.clusters_size(); ++i) {
    o << value.getClusters(i).id() << " ";
  }
  o << '\n';
  o << " hits : ";
  for (unsigned i = 0; i < value.hits_size(); ++i) {
    o << value.getHits(i) << " ";
  }
  o << '\n';
  o << " particleIDs : ";
  for (unsigned i = 0; i < value.particleIDs_size(); ++i) {
    o << value.getParticleIDs(i) << " ";
  }
  o << '\n';
  o << " shapeParameters : ";
  for (unsigned i = 0; i < value.shapeParameters_size(); ++i) {
    o << value.getShapeParameters(i) << " ";
  }
  o << '\n';
  o << " hitContributions : ";
  for (unsigned i = 0; i < value.hitContributions_size(); ++i) {
    o << value.getHitContributions(i) << " ";
  }
  o << '\n';
  o << " subdetectorEnergies : ";
  for (unsigned i = 0; i < value.subdetectorEnergies_size(); ++i) {
    o << value.getSubdetectorEnergies(i) << " ";
  }
  o << '\n';

  return o;
}

#ifdef PODIO_JSON_OUTPUT
void to_json(nlohmann::json& j, const Cluster& value) {
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

