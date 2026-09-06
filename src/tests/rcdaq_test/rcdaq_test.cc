// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

/// Unit tests for the rcdaq event source components:
///   - RCDAQFileReader: parsing a synthetic ONCS-format binary
///   - RCDAQFrameData:  lazy decoding via the podio::FrameDataType interface

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "services/io/rcdaq/RCDAQDecoder.h"
#include "services/io/rcdaq/RCDAQFileReader.h"
#include "services/io/rcdaq/RCDAQFrameData.h"
#include "services/io/rcdaq/RCDAQSubevent.h"

// ---------------------------------------------------------------------------
// Minimal in-memory structs that mirror the rcdaq binary layout.
// Using our own definitions avoids depending on rcdaq headers in the test
// translation unit while still producing correct byte patterns on
// little-endian (x86_64) machines.
// ---------------------------------------------------------------------------

namespace {

static constexpr uint32_t ONCS_MARKER   = 0xffffc0c0U;
static constexpr int32_t BLOCK_SIZE     = 8192;
static constexpr int32_t BUF_HDR_WORDS  = 4;
static constexpr int32_t EVT_HDR_WORDS  = 8;
static constexpr int32_t SEVT_HDR_WORDS = 4;
static constexpr int32_t DATAEVENT      = 1;
static constexpr int32_t BEGRUNEVENT    = 9;
static constexpr int16_t ID4EVT         = 6;

// Buffer header layout: Length, Marker, Bufseq, Runnr
struct BufHdr {
  int32_t length;
  uint32_t marker;
  int32_t bufseq;
  int32_t runnr;
};
static_assert(sizeof(BufHdr) == 16);

// Event header layout (8 int32 words)
struct EvtHdr {
  int32_t evt_length;
  int32_t evt_type;
  int32_t evt_sequence;
  int32_t run_number;
  int32_t date;
  int32_t time_;
  int32_t reserved[2];
};
static_assert(sizeof(EvtHdr) == 32);

// Sub-event header layout (4 int32 words = 16 bytes)
// On little-endian: word1 = (sub_type<<16)|sub_id, word2 = (padding<<16)|sub_decoding
struct SubHdr {
  int32_t sub_length;
  int16_t sub_id;
  int16_t sub_type;
  int16_t sub_decoding;
  int16_t sub_padding;
  int16_t reserved[2];
};
static_assert(sizeof(SubHdr) == 16);

/// Build a one-block (8192-byte) rcdaq binary containing:
///   - One BEGRUNEVENT (header only)
///   - One DATAEVENT with a single sub-event carrying @p payload int32 words
///   - An EOB marker
std::vector<uint8_t> buildSyntheticBlock(int32_t run_number, int32_t evt_sequence, int16_t sub_id,
                                         int16_t sub_type, int16_t sub_decoding,
                                         const std::vector<int32_t>& payload) {
  const int sub_words     = SEVT_HDR_WORDS + static_cast<int>(payload.size());
  const int dat_words     = EVT_HDR_WORDS + sub_words;
  const int content_words = BUF_HDR_WORDS + EVT_HDR_WORDS + dat_words + 2 /*EOB*/;
  const int content_bytes = content_words * 4;

  std::vector<uint8_t> buf(BLOCK_SIZE, 0);
  uint8_t* p = buf.data();

  // Buffer header
  BufHdr bh{content_bytes, ONCS_MARKER, 1, run_number};
  std::memcpy(p, &bh, sizeof(bh));
  p += sizeof(bh);

  // BEGRUNEVENT (header only, no sub-events)
  EvtHdr begin{};
  begin.evt_length   = EVT_HDR_WORDS;
  begin.evt_type     = BEGRUNEVENT;
  begin.evt_sequence = 0;
  begin.run_number   = run_number;
  std::memcpy(p, &begin, sizeof(begin));
  p += sizeof(begin);

  // DATAEVENT
  EvtHdr dat{};
  dat.evt_length   = dat_words;
  dat.evt_type     = DATAEVENT;
  dat.evt_sequence = evt_sequence;
  dat.run_number   = run_number;
  std::memcpy(p, &dat, sizeof(dat));
  p += sizeof(dat);

  // Sub-event header
  SubHdr sh{};
  sh.sub_length   = sub_words;
  sh.sub_id       = sub_id;
  sh.sub_type     = sub_type;
  sh.sub_decoding = sub_decoding;
  std::memcpy(p, &sh, sizeof(sh));
  p += sizeof(sh);

  // Sub-event payload
  for (auto v : payload) {
    std::memcpy(p, &v, 4);
    p += 4;
  }

  // EOB marker: {2, 0}
  int32_t eob[2] = {2, 0};
  std::memcpy(p, eob, sizeof(eob));

  return buf;
}

/// Write a binary blob to a uniquely-named temp file and return the path.
std::string writeTempFile(const std::vector<uint8_t>& data) {
  auto path = (std::filesystem::temp_directory_path() / "rcdaq_unit_test.rcdaq").string();
  std::ofstream f(path, std::ios::binary);
  f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
  return path;
}

// ---------------------------------------------------------------------------
// Spy decoder — records what decode() was called with
// ---------------------------------------------------------------------------
class SpyDecoder : public RCDAQDecoder {
public:
  explicit SpyDecoder(int32_t id) : m_id(id) {}

  int32_t packetID() const override { return m_id; }
  std::string collectionName() const override { return "TestColl_" + std::to_string(m_id); }
  std::string collectionType() const override { return "test::DummyCollection"; }

  std::optional<podio::CollectionReadBuffers> decode(int16_t sub_type, int16_t sub_decoding,
                                                     const int32_t* /*data*/, int nwords) override {
    m_called           = true;
    m_got_sub_type     = sub_type;
    m_got_sub_decoding = sub_decoding;
    m_got_nwords       = nwords;
    return std::nullopt; // real collections need factory registration
  }

  bool m_called{false};
  int16_t m_got_sub_type{-1};
  int16_t m_got_sub_decoding{-1};
  int m_got_nwords{-1};

private:
  int32_t m_id;
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// Tests: RCDAQFileReader
// ---------------------------------------------------------------------------

TEST_CASE("RCDAQFileReader parses BEGRUNEVENT and DATAEVENT", "[rcdaq]") {
  const int32_t run_number           = 42;
  const int32_t evt_sequence         = 7;
  const int16_t sub_id               = 5;
  const int16_t sub_type             = 1;
  const int16_t sub_decoding         = ID4EVT;
  const std::vector<int32_t> payload = {0x00000001, 0x000001A4};

  const auto data =
      buildSyntheticBlock(run_number, evt_sequence, sub_id, sub_type, sub_decoding, payload);
  const auto path = writeTempFile(data);

  RCDAQFileReader reader;
  reader.open(path);
  REQUIRE(reader.isOpen());

  RCDAQFileReader::Event evt;
  REQUIRE(reader.nextEvent(evt));

  // Run metadata
  CHECK(evt.run_number == run_number);
  CHECK(evt.evt_sequence == evt_sequence);

  // Sub-event count and header fields
  REQUIRE(evt.subevents.size() == 1u);
  const auto& se = evt.subevents[0];
  CHECK(se.sub_id == sub_id);
  CHECK(se.sub_type == sub_type);
  CHECK(se.sub_decoding == sub_decoding);

  // Payload words preserved verbatim
  REQUIRE(se.data.size() == payload.size());
  CHECK(se.data[0] == payload[0]);
  CHECK(se.data[1] == payload[1]);

  // No additional DATA events in the file
  CHECK_FALSE(reader.nextEvent(evt));

  reader.close();
  std::filesystem::remove(path);
}

TEST_CASE("RCDAQFileReader reports EOF on empty file", "[rcdaq]") {
  auto path = (std::filesystem::temp_directory_path() / "rcdaq_empty.rcdaq").string();
  {
    std::ofstream f(path);
  } // create empty file

  RCDAQFileReader reader;
  reader.open(path);
  REQUIRE(reader.isOpen());

  RCDAQFileReader::Event evt;
  CHECK_FALSE(reader.nextEvent(evt));
  reader.close();
  std::filesystem::remove(path);
}

// ---------------------------------------------------------------------------
// Tests: RCDAQFrameData
// ---------------------------------------------------------------------------

TEST_CASE("RCDAQFrameData.getAvailableCollections lists decoder collection", "[rcdaq]") {
  const int16_t sub_id               = 5;
  const std::vector<int32_t> payload = {1, 2, 3, 4};

  const auto data = buildSyntheticBlock(42, 1, sub_id, 1, ID4EVT, payload);
  const auto path = writeTempFile(data);

  RCDAQFileReader reader;
  reader.open(path);
  RCDAQFileReader::Event evt;
  reader.nextEvent(evt);
  reader.close();
  std::filesystem::remove(path);

  SpyDecoder spy(sub_id);
  RCDAQFrameData::DecoderMap dm{{sub_id, &spy}};
  RCDAQFrameData frame_data(std::move(evt), dm);

  const auto colls = frame_data.getAvailableCollections();
  REQUIRE(colls.size() == 1u);
  CHECK(colls[0] == spy.collectionName());
}

TEST_CASE("RCDAQFrameData.getCollectionBuffers calls decode with correct args", "[rcdaq]") {
  const int16_t sub_id               = 5;
  const int16_t sub_type             = 1;
  const int16_t sub_decoding         = ID4EVT;
  const std::vector<int32_t> payload = {10, 20, 30, 40};

  const auto data = buildSyntheticBlock(42, 1, sub_id, sub_type, sub_decoding, payload);
  const auto path = writeTempFile(data);

  RCDAQFileReader reader;
  reader.open(path);
  RCDAQFileReader::Event evt;
  reader.nextEvent(evt);
  reader.close();
  std::filesystem::remove(path);

  SpyDecoder spy(sub_id);
  RCDAQFrameData::DecoderMap dm{{sub_id, &spy}};
  RCDAQFrameData frame_data(std::move(evt), dm);

  // Unknown collection name → nullopt, decoder NOT called
  auto none = frame_data.getCollectionBuffers("nonexistent");
  CHECK_FALSE(none.has_value());
  CHECK_FALSE(spy.m_called);

  // Known collection name → decoder invoked with correct header fields
  auto result = frame_data.getCollectionBuffers(spy.collectionName());
  CHECK_FALSE(result.has_value()); // spy returns nullopt
  REQUIRE(spy.m_called);
  CHECK(spy.m_got_sub_type == sub_type);
  CHECK(spy.m_got_sub_decoding == sub_decoding);
  CHECK(spy.m_got_nwords == static_cast<int>(payload.size()));
}

TEST_CASE("RCDAQFrameData.getParameters contains run and event numbers", "[rcdaq]") {
  const int32_t run_number   = 99;
  const int32_t evt_sequence = 13;

  const auto data = buildSyntheticBlock(run_number, evt_sequence, 5, 1, ID4EVT, {1});
  const auto path = writeTempFile(data);

  RCDAQFileReader reader;
  reader.open(path);
  RCDAQFileReader::Event evt;
  reader.nextEvent(evt);
  reader.close();
  std::filesystem::remove(path);

  SpyDecoder spy(5);
  RCDAQFrameData::DecoderMap dm{{5, &spy}};
  RCDAQFrameData frame_data(std::move(evt), dm);

  const auto params = frame_data.getParameters();
  REQUIRE(params != nullptr);
  const auto run_opt = params->get<int>("run_number");
  const auto seq_opt = params->get<int>("event_sequence");
  REQUIRE(run_opt.has_value());
  REQUIRE(seq_opt.has_value());
  CHECK(run_opt.value() == run_number);
  CHECK(seq_opt.value() == evt_sequence);
}

// ---------------------------------------------------------------------------
// Integration test: PRDF format (real test file, if available)
// ---------------------------------------------------------------------------

TEST_CASE("RCDAQFileReader parses PRDF format (first_100_run375_KCU0.evt)",
          "[rcdaq][integration]") {
  // This test requires the real test file which is not committed to the repo.
  // It is skipped automatically when the file does not exist.
  const std::string path = std::string{SRCDIR} + "/first_100_run375_KCU0.evt";
  if (!std::filesystem::exists(path)) {
    SKIP("Test file not found: " + path);
  }

  RCDAQFileReader reader;
  reader.open(path);
  REQUIRE(reader.isOpen());

  // File is PRDF format
  RCDAQFileReader::Event evt;
  REQUIRE(reader.nextEvent(evt));
  CHECK(reader.format() == RCDAQFileReader::Format::PRDF);

  // Known properties of first_100_run375_KCU0.evt
  CHECK(evt.run_number == 375);
  REQUIRE(evt.subevents.size() >= 1u);

  // Every event should contain packet ID 12001 (0x2EE1)
  bool found_12001 = false;
  for (const auto& se : evt.subevents) {
    if (se.packet_id == 12001) {
      found_12001 = true;
      CHECK(se.data.size() > 0u);
      break;
    }
  }
  CHECK(found_12001);

  // Read remaining events (file contains 100 events)
  int count = 1;
  while (reader.nextEvent(evt)) {
    ++count;
  }
  reader.close();
  CHECK(count == 100);
}
