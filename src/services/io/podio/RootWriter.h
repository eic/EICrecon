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
#include <set>

#include <services/log/Log_service.h>

// forward declarations
class TFile;
class TTree;

namespace eic {
class EventStore;

class ROOTWriter {

    struct CollectionInfo {
        int id;
        std::string name;
        std::string podtype;
        bool write_requested = false;
        bool write_failed = false;
        bool is_subset = false;
        podio::root_utils::CollectionBranches branches;
    };

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
  void createBranches(CollectionInfo& info, podio::CollectionBase* collection);

  // members
  std::string m_filename;
  TFile* m_file;
  TTree* m_datatree;
  TTree* m_metadatatree;
  TTree* m_runMDtree;
  TTree* m_evtMDtree;
  TTree* m_colMDtree;

  std::map<std::string, std::unique_ptr<CollectionInfo>> m_collection_infos;
  std::set<std::string> m_write_requests;
  bool m_firstEvent{true};
  std::shared_ptr<spdlog::logger> m_log;
};

} // namespace eic
#endif
