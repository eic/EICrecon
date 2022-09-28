#ifndef EIC_PODIO_ROOTWRITER_H
#define EIC_PODIO_ROOTWRITER_H

#include <podio/CollectionBase.h>
#include <podio/CollectionBranches.h>

#include "TBranch.h"

#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <map>

#include <services/log/Log_service.h>

// forward declarations
class TFile;
class TTree;

namespace eic {
class EventStore;

class ROOTWriter {

public:
  ROOTWriter(const std::string& filename, std::shared_ptr<spdlog::logger> &logger);
  ~ROOTWriter();

  // non-copyable
  ROOTWriter(const ROOTWriter&) = delete;
  ROOTWriter& operator=(const ROOTWriter&) = delete;

  bool registerForWrite(const std::string& name);
  void writeEvent(eic::EventStore* store);
  void finish();

private:
  using StoreCollection = std::pair<const std::string&, podio::CollectionBase*>;
  void createBranches(const std::vector<StoreCollection>& collections);
  void setBranches(const std::vector<StoreCollection>& collections);

  // members
  std::string m_filename;
  TFile* m_file;
  TTree* m_datatree;
  TTree* m_metadatatree;
  TTree* m_runMDtree;
  TTree* m_evtMDtree;
  TTree* m_colMDtree;

  std::map<std::string, podio::root_utils::CollectionBranches> m_collectionBranchesToWrite{};
  bool m_firstEvent{true};
  std::set<std::string> unwritable_collections; // keep list of collections that threw exception during prepareForWrite

  std::shared_ptr<spdlog::logger> m_log;
};

} // namespace eic
#endif
