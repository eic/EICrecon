// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "RCDAQFileReader.h"

// rcdaq format constants (headers live in the source tree at compile time;
// the include path is set by the CMakeLists.txt find_path call)
#include <BufferConstants.h>
#include <EvtConstants.h>
#include <EvtStructures.h>
#include <EventTypes.h>
#include <SubevtConstants.h>
#include <SubevtStructures.h>

#include <cstring>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// open
// ---------------------------------------------------------------------------
void RCDAQFileReader::open(const std::string& path) {
  m_file.open(path, std::ios::binary);
  if (!m_file.is_open()) {
    throw std::runtime_error("RCDAQFileReader: cannot open '" + path + "'");
  }
  m_buf_offset = 0;
  m_buf_words  = 0;
  m_run_number = 0;
}

// ---------------------------------------------------------------------------
// close
// ---------------------------------------------------------------------------
void RCDAQFileReader::close() { m_file.close(); }

// ---------------------------------------------------------------------------
// nextEvent — public entry point
// ---------------------------------------------------------------------------
bool RCDAQFileReader::nextEvent(Event& out) {
  while (true) {
    // Try to find a DATA event in the current buffer
    if (m_buf_words > 0 && nextEventInBuffer(out)) {
      return true;
    }
    // Current buffer exhausted — read the next one
    if (!readNextBuffer()) {
      return false; // end of file
    }
  }
}

// ---------------------------------------------------------------------------
// readNextBuffer
// ---------------------------------------------------------------------------
bool RCDAQFileReader::readNextBuffer() {
  // The buffer header is 4 int32 words (16 bytes).
  // Layout: Length (bytes), ID (format marker), Bufseq, Runnr
  constexpr int HDR_WORDS = BUFFERHEADERLENGTH; // 4

  int32_t hdr[HDR_WORDS];
  if (!m_file.read(reinterpret_cast<char*>(hdr), HDR_WORDS * 4)) {
    return false; // clean EOF
  }

  const auto marker = static_cast<uint32_t>(hdr[1]);
  if (marker != ONCSBUFFERMARKER && marker != BUFFERMARKER) {
    if (marker == LZO1XBUFFERMARKER || marker == ONCSLZO1XBUFFERMARKER) {
      throw std::runtime_error("RCDAQFileReader: LZO-compressed buffers are not yet supported");
    }
    if (marker == GZBUFFERMARKER) {
      throw std::runtime_error("RCDAQFileReader: GZ-compressed buffers are not yet supported");
    }
    throw std::runtime_error("RCDAQFileReader: unrecognised buffer marker 0x" + [&]() {
      char buf[16];
      std::snprintf(buf, sizeof(buf), "%08x", marker);
      return std::string(buf);
    }());
  }

  const unsigned int length_bytes = static_cast<unsigned int>(hdr[0]);
  if (length_bytes < static_cast<unsigned int>(HDR_WORDS * 4)) {
    throw std::runtime_error("RCDAQFileReader: buffer length smaller than header");
  }

  // Total bytes written to disk are rounded up to BUFFERBLOCKSIZE (8192)
  const int block_count = (static_cast<int>(length_bytes) + BUFFERBLOCKSIZE - 1) / BUFFERBLOCKSIZE;
  const int disk_bytes  = block_count * BUFFERBLOCKSIZE;

  // Resize buffer and copy the header we already read
  const int total_words = disk_bytes / 4;
  m_buf.resize(total_words);
  std::memcpy(m_buf.data(), hdr, HDR_WORDS * 4);

  // Read the remainder of the padded block
  const int remaining_bytes = disk_bytes - HDR_WORDS * 4;
  if (!m_file.read(reinterpret_cast<char*>(m_buf.data() + HDR_WORDS), remaining_bytes)) {
    // Partial read at end of file — we can still try to use what we got
    // by limiting m_buf_words to the valid length
  }

  // Valid data ends at length_bytes (rounded up to word boundary)
  m_buf_words  = static_cast<int>((length_bytes + 3) / 4);
  m_buf_offset = HDR_WORDS; // start right after the buffer header

  // Track the run number from the buffer header (word 3 = Runnr)
  if (hdr[3] > 0) {
    m_run_number = hdr[3];
  }

  return true;
}

// ---------------------------------------------------------------------------
// nextEventInBuffer
// ---------------------------------------------------------------------------
bool RCDAQFileReader::nextEventInBuffer(Event& out) {
  // End-of-buffer marker is two int32 words: {2, 0}.  Stop when we hit it
  // or when there is not enough room for a minimal event header.
  while (m_buf_offset + EVTHEADERLENGTH <= m_buf_words) {
    const int32_t* p = m_buf.data() + m_buf_offset;

    // Cast the raw words to the rcdaq event header struct
    const evtdata_ptr evt = reinterpret_cast<const evtdata_ptr>(const_cast<int32_t*>(p));

    const int evt_words = evt->evt_length; // in int32 units, includes header

    // EOB marker: length==2 and the second word is 0
    if (evt_words == 2 && p[1] == 0) {
      m_buf_offset = m_buf_words; // signal buffer exhausted
      return false;
    }

    if (evt_words < EVTHEADERLENGTH || m_buf_offset + evt_words > m_buf_words) {
      // Corrupt or truncated event — skip this buffer
      m_buf_offset = m_buf_words;
      return false;
    }

    const int evt_type = evt->evt_type;

    // Advance past this event regardless of whether we emit it
    m_buf_offset += evt_words;

    if (evt_type == BEGRUNEVENT || evt_type == ENDRUNEVENT) {
      // Update run number but do not emit these as data events
      if (evt->run_number > 0) {
        m_run_number = evt->run_number;
      }
      continue;
    }

    if (evt_type != DATAEVENT && evt_type != DATA2EVENT && evt_type != DATA3EVENT) {
      // SCALE, RUNINFO, etc. — silently skip
      continue;
    }

    // --- DATA event: parse sub-events ---
    out.run_number   = (evt->run_number > 0) ? evt->run_number : m_run_number;
    out.evt_type     = evt_type;
    out.evt_sequence = evt->evt_sequence;
    out.subevents.clear();

    // Sub-events follow immediately after the event header
    // We work on the slice [event_start + EVTHEADERLENGTH, event_start + evt_words)
    const int evt_start = m_buf_offset - evt_words; // index of event header
    int sub_offset      = evt_start + EVTHEADERLENGTH;
    const int evt_end   = evt_start + evt_words;

    while (sub_offset + SEVTHEADERLENGTH <= evt_end) {
      const int32_t* sp        = m_buf.data() + sub_offset;
      const subevtdata_ptr sub = reinterpret_cast<const subevtdata_ptr>(const_cast<int32_t*>(sp));

      const int sub_words = sub->sub_length; // in int32 units, includes header
      if (sub_words < SEVTHEADERLENGTH || sub_offset + sub_words > evt_end) {
        break; // corrupt sub-event
      }

      RCDAQSubevent se;
      se.sub_id       = sub->sub_id;
      se.sub_type     = sub->sub_type;
      se.sub_decoding = sub->sub_decoding;

      // Payload starts at &sub->data; length = sub_words - SEVTHEADERLENGTH words
      const int payload_words = sub_words - SEVTHEADERLENGTH;
      const int32_t* payload  = sp + SEVTHEADERLENGTH;
      se.data.assign(payload, payload + payload_words);

      out.subevents.push_back(std::move(se));
      sub_offset += sub_words;
    }

    return true;
  }

  // No more events in this buffer
  m_buf_offset = m_buf_words;
  return false;
}
