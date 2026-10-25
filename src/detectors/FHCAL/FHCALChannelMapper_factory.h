// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <edm4eic/SimPulseCollection.h>
#include <cstdint>
#include <string>

#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

struct FHCALChannelMapperConfig {
  std::string readout;

  // Last cell-ID field retained in the readout channel ID.
  std::string channel_field;
};

// Map combined pulses to readout channel IDs.
class FHCALChannelMapper_factory
    : public JOmniFactory<FHCALChannelMapper_factory, FHCALChannelMapperConfig> {
private:
  PodioInput<edm4eic::SimPulse> m_in_pulses{this};
  PodioOutput<edm4eic::SimPulse> m_out_pulses{this};

  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::string> m_channel_field{this, "channelField", config().channel_field};

  Service<DD4hep_service> m_geoSvc{this};
  std::uint64_t m_channel_mask = 0;

public:
  void Configure() {
    const auto id_spec = m_geoSvc().detector()->readout(config().readout).idSpec();

    // Loop over readout fields through the channel field.
    for (const auto& field : id_spec.fields()) {
      m_channel_mask |= id_spec.field(field.first)->mask();
      if (field.first == config().channel_field) {
        break;
      }
    }
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {

    // Loop over combined pulses.
    for (const auto pulse : *m_in_pulses()) {

      // Clear tile-level cell-ID fields.
      auto mapped_pulse = pulse.clone();
      mapped_pulse.setCellID(pulse.getCellID() & m_channel_mask);
      m_out_pulses()->push_back(mapped_pulse);
    }
  }
};

} // namespace eicrecon
