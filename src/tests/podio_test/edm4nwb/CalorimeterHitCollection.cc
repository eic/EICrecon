// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#include "edm4nwb/CalorimeterHitCollection.h"


// standard includes
#include <stdexcept>
#include <iomanip>

namespace edm4nwb {


CalorimeterHitCollection::CalorimeterHitCollection() :
  m_isValid(false), m_isPrepared(false), m_isSubsetColl(false), m_collectionID(0), m_storage() {}

CalorimeterHitCollection::~CalorimeterHitCollection() {
  // Need to tell the storage how to clean-up
  m_storage.clear(m_isSubsetColl);
}

CalorimeterHit CalorimeterHitCollection::operator[](unsigned int index) const {
  return CalorimeterHit(m_storage.entries[index]);
}

CalorimeterHit CalorimeterHitCollection::at(unsigned int index) const {
  return CalorimeterHit(m_storage.entries.at(index));
}

MutableCalorimeterHit CalorimeterHitCollection::operator[](unsigned int index) {
  return MutableCalorimeterHit(m_storage.entries[index]);
}

MutableCalorimeterHit CalorimeterHitCollection::at(unsigned int index) {
  return MutableCalorimeterHit(m_storage.entries.at(index));
}

size_t CalorimeterHitCollection::size() const {
  return m_storage.entries.size();
}

void CalorimeterHitCollection::setSubsetCollection(bool setSubset) {
  if (m_isSubsetColl != setSubset && !m_storage.entries.empty()) {
    throw std::logic_error("Cannot change the character of a collection that already contains elements");
  }

  if (setSubset) {
    m_storage.makeSubsetCollection();
  }
  m_isSubsetColl = setSubset;
}

MutableCalorimeterHit CalorimeterHitCollection::create() {
  if (m_isSubsetColl) {
    throw std::logic_error("Cannot create new elements on a subset collection");
  }

  auto obj = m_storage.entries.emplace_back(new CalorimeterHitObj());

  obj->id = {int(m_storage.entries.size() - 1), m_collectionID};
  return MutableCalorimeterHit(obj);
}

void CalorimeterHitCollection::clear() {
  m_storage.clear(m_isSubsetColl);
  m_isPrepared = false;
}

void CalorimeterHitCollection::prepareForWrite() const {
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

void CalorimeterHitCollection::prepareAfterRead() {
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

bool CalorimeterHitCollection::setReferences(const podio::ICollectionProvider* collectionProvider) {
  return m_storage.setReferences(collectionProvider, m_isSubsetColl);
}

void CalorimeterHitCollection::push_back(CalorimeterHit object) {
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

podio::CollectionBuffers CalorimeterHitCollection::getBuffers() {
  return m_storage.getCollectionBuffers(m_isSubsetColl);
}

#ifdef PODIO_JSON_OUTPUT
void to_json(nlohmann::json& j, const CalorimeterHitCollection& collection) {
  j = nlohmann::json::array();
  for (auto&& elem : collection) {
    j.emplace_back(elem);
  }
}
#endif


CalorimeterHit CalorimeterHitCollectionIterator::operator*() {
  m_object.m_obj = (*m_collection)[m_index];
  return m_object;
}

CalorimeterHit* CalorimeterHitCollectionIterator::operator->() {
  m_object.m_obj = (*m_collection)[m_index];
  return &m_object;
}

CalorimeterHitCollectionIterator& CalorimeterHitCollectionIterator::operator++() {
  ++m_index;
  return *this;
}



MutableCalorimeterHit CalorimeterHitMutableCollectionIterator::operator*() {
  m_object.m_obj = (*m_collection)[m_index];
  return m_object;
}

MutableCalorimeterHit* CalorimeterHitMutableCollectionIterator::operator->() {
  m_object.m_obj = (*m_collection)[m_index];
  return &m_object;
}

CalorimeterHitMutableCollectionIterator& CalorimeterHitMutableCollectionIterator::operator++() {
  ++m_index;
  return *this;
}



std::ostream& operator<<(std::ostream& o, const CalorimeterHitCollection& v) {
  const auto old_flags = o.flags();
  o << "          id:      cellID:      energy: energyError:        time:   timeError:    position:   dimension:      sector:       layer:       local:" << '\n';

  for (const auto&& el : v) {
    o << std::scientific << std::showpos << std::setw(12) << el.id() << " "
      << std::setw(12) << el.getCellID() << " "
      << std::setw(12) << el.getEnergy() << " "
      << std::setw(12) << el.getEnergyError() << " "
      << std::setw(12) << el.getTime() << " "
      << std::setw(12) << el.getTimeError() << " "
      << std::setw(12) << el.getPosition() << " "
      << std::setw(12) << el.getDimension() << " "
      << std::setw(12) << el.getSector() << " "
      << std::setw(12) << el.getLayer() << " "
      << std::setw(12) << el.getLocal() << " "
      << std::endl;




  }

  o.flags(old_flags);
  return o;
}

} // namespace edm4nwb

