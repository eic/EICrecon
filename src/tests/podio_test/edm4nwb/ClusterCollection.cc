// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#include "edm4nwb/ClusterCollection.h"

#include "edm4nwb/CalorimeterHitCollection.h"
#include "edm4hep/ParticleIDCollection.h"
#include <numeric>

// standard includes
#include <stdexcept>
#include <iomanip>

namespace edm4nwb {


ClusterCollection::ClusterCollection() :
  m_isValid(false), m_isPrepared(false), m_isSubsetColl(false), m_collectionID(0), m_storage() {}

ClusterCollection::~ClusterCollection() {
  // Need to tell the storage how to clean-up
  m_storage.clear(m_isSubsetColl);
}

Cluster ClusterCollection::operator[](unsigned int index) const {
  return Cluster(m_storage.entries[index]);
}

Cluster ClusterCollection::at(unsigned int index) const {
  return Cluster(m_storage.entries.at(index));
}

MutableCluster ClusterCollection::operator[](unsigned int index) {
  return MutableCluster(m_storage.entries[index]);
}

MutableCluster ClusterCollection::at(unsigned int index) {
  return MutableCluster(m_storage.entries.at(index));
}

size_t ClusterCollection::size() const {
  return m_storage.entries.size();
}

void ClusterCollection::setSubsetCollection(bool setSubset) {
  if (m_isSubsetColl != setSubset && !m_storage.entries.empty()) {
    throw std::logic_error("Cannot change the character of a collection that already contains elements");
  }

  if (setSubset) {
    m_storage.makeSubsetCollection();
  }
  m_isSubsetColl = setSubset;
}

MutableCluster ClusterCollection::create() {
  if (m_isSubsetColl) {
    throw std::logic_error("Cannot create new elements on a subset collection");
  }

  auto obj = m_storage.entries.emplace_back(new ClusterObj());
  m_storage.createRelations(obj);

  obj->id = {int(m_storage.entries.size() - 1), m_collectionID};
  return MutableCluster(obj);
}

void ClusterCollection::clear() {
  m_storage.clear(m_isSubsetColl);
  m_isPrepared = false;
}

void ClusterCollection::prepareForWrite() const {
  // TODO: Replace this double locking pattern here with an atomic and only one
  // lock. Problem: std::atomic is not default movable.
  {
    std::lock_guard lock{*m_storageMtx};
    // If the collection has been prepared already there is nothing to do
    if (m_isPrepared) {
      return;
    }
  }

  std::lock_guard lock{*m_storageMtx};
  // by the time we acquire the lock another thread might have already
  // succeeded, so we need to check again now
  if (m_isPrepared) {
    return;
  }
  m_storage.prepareForWrite(m_isSubsetColl);
  m_isPrepared = true;
}

void ClusterCollection::prepareAfterRead() {
  // No need to go through this again if we have already done it
  if (m_isPrepared) {
    return;
  }

  if (!m_isSubsetColl) {
    // Subset collections do not store any data that would require post-processing
    m_storage.prepareAfterRead(m_collectionID);
  }
  // Preparing a collection doesn't affect the underlying I/O buffers, so this
  // collection is still prepared
  m_isPrepared = true;
}

bool ClusterCollection::setReferences(const podio::ICollectionProvider* collectionProvider) {
  return m_storage.setReferences(collectionProvider, m_isSubsetColl);
}

void ClusterCollection::push_back(Cluster object) {
  // We have to do different things here depending on whether this is a
  // subset collection or not. A normal collection cannot collect objects
  // that are already part of another collection, while a subset collection
  // can only collect such objects
  if (!m_isSubsetColl) {
    auto obj = object.m_obj;
    if (obj->id.index == podio::ObjectID::untracked) {
      const auto size = m_storage.entries.size();
      obj->id = {(int)size, m_collectionID};
      m_storage.entries.push_back(obj);
      m_storage.createRelations(obj);
    } else {
      throw std::invalid_argument("Object already in a collection. Cannot add it to a second collection");
    }
  } else {
    const auto obj = object.m_obj;
    if (obj->id.index < 0) {
      throw std::invalid_argument("Objects can only be stored in a subset collection if they are already elements of a collection");
    }
    m_storage.entries.push_back(obj);
    // No need to handle any relations here, since this is already done by the
    // "owning" collection
  }
}

podio::CollectionBuffers ClusterCollection::getBuffers() {
  return m_storage.getCollectionBuffers(m_isSubsetColl);
}

#ifdef PODIO_JSON_OUTPUT
void to_json(nlohmann::json& j, const ClusterCollection& collection) {
  j = nlohmann::json::array();
  for (auto&& elem : collection) {
    j.emplace_back(elem);
  }
}
#endif


Cluster ClusterCollectionIterator::operator*() {
  m_object.m_obj = (*m_collection)[m_index];
  return m_object;
}

Cluster* ClusterCollectionIterator::operator->() {
  m_object.m_obj = (*m_collection)[m_index];
  return &m_object;
}

ClusterCollectionIterator& ClusterCollectionIterator::operator++() {
  ++m_index;
  return *this;
}



MutableCluster ClusterMutableCollectionIterator::operator*() {
  m_object.m_obj = (*m_collection)[m_index];
  return m_object;
}

MutableCluster* ClusterMutableCollectionIterator::operator->() {
  m_object.m_obj = (*m_collection)[m_index];
  return &m_object;
}

ClusterMutableCollectionIterator& ClusterMutableCollectionIterator::operator++() {
  ++m_index;
  return *this;
}



std::ostream& operator<<(std::ostream& o, const ClusterCollection& v) {
  const auto old_flags = o.flags();
  o << "          id:        type:      energy: energyError:        time:   timeError:       nhits:    position:                                 positionError [ xx, yy, zz, xy, xz, yz]:intrinsicTheta:intrinsicPhi:intrinsicDirectionError [ xx, yy, xy]:" << '\n';

  for (const auto&& el : v) {
    o << std::scientific << std::showpos << std::setw(12) << el.id() << " "
      << std::setw(12) << el.getType() << " "
      << std::setw(12) << el.getEnergy() << " "
      << std::setw(12) << el.getEnergyError() << " "
      << std::setw(12) << el.getTime() << " "
      << std::setw(12) << el.getTimeError() << " "
      << std::setw(12) << el.getNhits() << " "
      << std::setw(12) << el.getPosition() << " "
      << std::setw(12) << el.getPositionError() << " "
      << std::setw(12) << el.getIntrinsicTheta() << " "
      << std::setw(12) << el.getIntrinsicPhi() << " "
      << std::setw(12) << el.getIntrinsicDirectionError() << " "
      << std::endl;

    o << "      clusters : ";
    for (unsigned j = 0, N = el.clusters_size(); j < N; ++j) {
      o << el.getClusters(j).id() << " ";
    }
    o << std::endl;
    o << "      hits : ";
    for (unsigned j = 0, N = el.hits_size(); j < N; ++j) {
      o << el.getHits(j).id() << " ";
    }
    o << std::endl;
    o << "      particleIDs : ";
    for (unsigned j = 0, N = el.particleIDs_size(); j < N; ++j) {
      o << el.getParticleIDs(j).id() << " ";
    }
    o << std::endl;


    o << "      shapeParameters : ";
    for (unsigned j = 0, N = el.shapeParameters_size(); j < N; ++j) {
      o << el.getShapeParameters(j) << " ";
    }
    o << std::endl;
    o << "      hitContributions : ";
    for (unsigned j = 0, N = el.hitContributions_size(); j < N; ++j) {
      o << el.getHitContributions(j) << " ";
    }
    o << std::endl;
    o << "      subdetectorEnergies : ";
    for (unsigned j = 0, N = el.subdetectorEnergies_size(); j < N; ++j) {
      o << el.getSubdetectorEnergies(j) << " ";
    }
    o << std::endl;

  }

  o.flags(old_flags);
  return o;
}

} // namespace edm4nwb

