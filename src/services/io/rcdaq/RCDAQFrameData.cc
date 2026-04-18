// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "RCDAQFrameData.h"

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
RCDAQFrameData::RCDAQFrameData(RCDAQFileReader::Event event, const DecoderMap& decoders)
    : m_event(std::move(event))
    , m_decoders(decoders)
    , m_params(std::make_unique<podio::GenericParameters>()) {

  m_params->set("run_number", static_cast<int>(m_event.run_number));
  m_params->set("event_sequence", static_cast<int>(m_event.evt_sequence));

  // Build the ID table and the name→subevent index in one pass.
  for (const auto& se : m_event.subevents) {
    auto it = m_decoders.find(se.sub_id);
    if (it == m_decoders.end()) {
      continue;
    }
    const std::string& coll_name = it->second->collectionName();
    m_idTable.add(coll_name);
    m_nameToSubevent[coll_name] = &se;
  }
}

// ---------------------------------------------------------------------------
// getIDTable
// ---------------------------------------------------------------------------
podio::CollectionIDTable RCDAQFrameData::getIDTable() const {
  // Return a copy so Frame can take ownership.
  return podio::CollectionIDTable(m_idTable.ids(), m_idTable.names());
}

// ---------------------------------------------------------------------------
// getCollectionBuffers  (called lazily by podio::Frame on first access)
// ---------------------------------------------------------------------------
std::optional<podio::CollectionReadBuffers>
RCDAQFrameData::getCollectionBuffers(const std::string& name) {
  auto se_it = m_nameToSubevent.find(name);
  if (se_it == m_nameToSubevent.end()) {
    return std::nullopt;
  }
  const RCDAQSubevent* se = se_it->second;

  auto dec_it = m_decoders.find(se->sub_id);
  if (dec_it == m_decoders.end()) {
    return std::nullopt;
  }

  return dec_it->second->decode(se->sub_type, se->sub_decoding, se->data.data(),
                                static_cast<int>(se->data.size()));
}

// ---------------------------------------------------------------------------
// getAvailableCollections
// ---------------------------------------------------------------------------
std::vector<std::string> RCDAQFrameData::getAvailableCollections() const {
  return m_idTable.names();
}

// ---------------------------------------------------------------------------
// getParameters  (consumed once by the Frame constructor)
// ---------------------------------------------------------------------------
std::unique_ptr<podio::GenericParameters> RCDAQFrameData::getParameters() {
  return std::move(m_params);
}
