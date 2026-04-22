// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "RCDAQSubevent.h"

/// Sequential reader for rcdaq binary data files.
///
/// Supports uncompressed ONCS (buffer marker 0xffffc0c0) and PRDF
/// (buffer marker 0xffffffc0) format files.
/// LZO-compressed buffers are detected and a descriptive exception is thrown.
/// Each call to nextEvent() fills one Event struct with the header metadata and
/// all sub-events belonging to that event.  BEGIN/END run records update the
/// stored run number but are not returned to the caller; only DATAEVENT records
/// are emitted.
class RCDAQFileReader {
public:
  /// Underlying binary format of the currently-open file.
  enum class Format { Unknown, ONCS, PRDF };

  struct Event {
    int run_number{0};
    int evt_type{0};
    int evt_sequence{0};
    std::vector<RCDAQSubevent> subevents;
  };

  RCDAQFileReader()  = default;
  ~RCDAQFileReader() = default;

  // Non-copyable, movable
  RCDAQFileReader(const RCDAQFileReader&)            = delete;
  RCDAQFileReader& operator=(const RCDAQFileReader&) = delete;
  RCDAQFileReader(RCDAQFileReader&&)                 = default;
  RCDAQFileReader& operator=(RCDAQFileReader&&)      = default;

  /// Open a file by path.  Throws std::runtime_error on failure.
  void open(const std::string& path);

  /// Read the next DATA event into \p out.
  /// @return true if an event was read, false at end of file.
  bool nextEvent(Event& out);

  /// Close the file.
  void close();

  bool isOpen() const { return m_file.is_open(); }

  /// Returns the detected binary format (set after the first buffer is read).
  Format format() const { return m_format; }

private:
  /// Read the next buffer from disk into m_buf. Returns false at EOF.
  bool readNextBuffer();

  /// Parse events sequentially from the current buffer starting at m_buf_offset.
  /// Returns true and fills \p out when a DATA event is found.
  bool nextEventInBuffer(Event& out);

  std::ifstream m_file;

  // Current buffer contents (in int32 units)
  std::vector<int32_t> m_buf;

  // Index (in int32 units) of the next event within m_buf (past the buffer header)
  int m_buf_offset{0};

  // Total valid words in m_buf (= bptr->Length / 4)
  int m_buf_words{0};

  int m_run_number{0};

  Format m_format{Format::Unknown};
};
