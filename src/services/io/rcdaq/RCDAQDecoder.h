// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <podio/CollectionBuffers.h>
#include <cstdint>
#include <string>

/// Abstract interface for decoding a single rcdaq sub-event into a podio
/// CollectionReadBuffers object.
///
/// Implement this interface for each sub-event ID that should be exposed as an
/// EDM4hep (or other podio) collection. The implementer is responsible for:
///   1. Calling podio::CollectionBufferFactory::instance().createBuffers() to
///      allocate properly-typed buffers.
///   2. Filling the data vector (via buffers.dataAsVector<DataT>()) from the
///      raw rcdaq words.
///   3. Returning the filled buffers.
///
/// Example usage:
/// @code
///   auto src = std::make_shared<JEventSourceRCDAQ>("myfile.prdf", app);
///   src->addDecoder(std::make_unique<MyCaloDecoder>());
/// @endcode
class RCDAQDecoder {
public:
  virtual ~RCDAQDecoder() = default;

  /// The sub-event ID this decoder handles (matches RCDAQSubevent::sub_id).
  virtual int16_t subeventID() const = 0;

  /// The name of the collection that will appear in the podio Frame.
  virtual std::string collectionName() const = 0;

  /// The fully-qualified collection type name, e.g.
  ///   "edm4hep::RawCalorimeterHitCollection"
  /// This must match the string registered in podio::CollectionBufferFactory.
  virtual std::string collectionType() const = 0;

  /// Decode \p nwords int32_t words starting at \p data into CollectionReadBuffers.
  ///
  /// \p sub_type    mirrors the trigger event type (DATA1EVENT=1, etc.).
  /// \p sub_decoding identifies the payload encoding scheme (see
  ///                SubevtConstants.h: IDCRAW=0, ID4EVT=6, IDDCFEM=51, …).
  ///
  /// The implementation should:
  ///   1. Validate \p sub_decoding against the expected format.
  ///   2. Obtain properly-initialised buffers from CollectionBufferFactory.
  ///   3. Fill them with decoded data.
  ///   4. Return the buffers (or an empty optional on failure / unknown encoding).
  ///
  /// Called lazily by RCDAQFrameData::getCollectionBuffers() when the podio
  /// Frame is asked for this collection for the first time.
  virtual std::optional<podio::CollectionReadBuffers> decode(int16_t sub_type, int16_t sub_decoding,
                                                             const int32_t* data, int nwords) = 0;
};
