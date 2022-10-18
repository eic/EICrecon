// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#ifndef EDM4NWB_CalorimeterHit_CollectionData_H
#define EDM4NWB_CalorimeterHit_CollectionData_H

// datamodel specific includes
#include "edm4nwb/CalorimeterHitData.h"
#include "edm4nwb/CalorimeterHitObj.h"

// podio specific includes
#include "podio/CollectionBuffers.h"
#include "podio/ICollectionProvider.h"

#include <deque>
#include <memory>

namespace edm4nwb {


using CalorimeterHitObjPointerContainer = std::deque<CalorimeterHitObj*>;
using CalorimeterHitDataContainer = std::vector<CalorimeterHitData>;


/**
 * Class encapsulating everything related to storage of data that is needed by a
 * collection.
 */
class CalorimeterHitCollectionData {
public:
  /**
   * The Objs of this collection
   */
  CalorimeterHitObjPointerContainer entries{};

  /**
   * Default constructor setting up the necessary buffers
   */
  CalorimeterHitCollectionData();

  /**
   * Non copy-able, move-only class
   */
  CalorimeterHitCollectionData(const CalorimeterHitCollectionData&) = delete;
  CalorimeterHitCollectionData& operator=(const CalorimeterHitCollectionData&) = delete;
  CalorimeterHitCollectionData(CalorimeterHitCollectionData&& other) = default;
  CalorimeterHitCollectionData& operator=(CalorimeterHitCollectionData&& other) = default;

  /**
   * Deconstructor
   */
  ~CalorimeterHitCollectionData() = default;

  void clear(bool isSubsetColl);

  podio::CollectionBuffers getCollectionBuffers(bool isSubsetColl);

  void prepareForWrite(bool isSubsetColl);

  void prepareAfterRead(int collectionID);

  void makeSubsetCollection();


  bool setReferences(const podio::ICollectionProvider* collectionProvider, bool isSubsetColl);

private:
  // members to handle 1-to-N-relations

  // members to handle vector members

  // I/O related buffers
  podio::CollRefCollection m_refCollections{};
  podio::VectorMembersInfo m_vecmem_info{};
  std::unique_ptr<CalorimeterHitDataContainer> m_data{nullptr};
};


} // namespace edm4nwb


#endif
