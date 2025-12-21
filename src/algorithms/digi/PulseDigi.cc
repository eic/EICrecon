// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim
//

#include <algorithms/service.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <limits>
#include <map>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "PulseDigi.h"

class HGCROCRawSample {
	public:
		struct RawEntry {
			double adc{0};
			double toa{0};
			double tot{0};
		};

		HGCROCRawSample(std::size_t n_samp) { rawEntries.resize(n_samp); }

		void setADC(std::size_t idx, double adc) { rawEntries[idx].adc = adc; }
		void setTOA(std::size_t idx, double toa) { rawEntries[idx].toa = toa; }
		void setTOT(std::size_t idx, double tot) { rawEntries[idx].tot = tot; }

		const std::vector<RawEntry>& getEntries() const { return rawEntries; }

	private:
		std::vector<RawEntry> rawEntries;
};

namespace eicrecon {

	void PulseDigi::init() {}

	void PulseDigi::process(const PulseDigi::Input& input, const PulseDigi::Output& output) const {
		const auto [in_pulses] = input;
		auto [out_digi_hits]   = output;

		for (const auto& pulse : *in_pulses) {
			double pulse_t     = pulse.getTime();
			double pulse_dt    = pulse.getInterval();
			std::size_t n_amps = pulse.getAmplitude().size();

			// Calculate the number of samples.
			std::size_t time_stamp_first = static_cast<std::size_t>(std::ceil((pulse_t - m_cfg.adc_phase) / m_cfg.time_window));
			std::size_t time_stamp_last = static_cast<std::size_t>(std::ceil((pulse_t + (n_amps - 1) * pulse_dt - m_cfg.adc_phase) / m_cfg.time_window));
			std::size_t n_samps = time_stamp_last - time_stamp_first + 1;
			if (n_samps <= 0) continue;
			HGCROCRawSample raw_sample(n_samps);

			// For ADC, amplitude is measured with a fixed phase.
			// This was reproduced by sample_tick and adc_counter as follows.
			// Amplitude is measured whenever adc_counter reaches sample_tick.
			int sample_tick = std::llround(m_cfg.time_window / pulse_dt);
			int adc_counter =
				std::llround((std::floor(pulse_t / m_cfg.time_window) * m_cfg.time_window + m_cfg.adc_phase - pulse_t) / pulse_dt);

			bool toa_pending = false;
			bool tot_progress  = false;
			double t_upcross = 0;
			std::size_t idx_sample = 0;
			std::size_t idx_toa = 0;

			for (std::size_t i = 0; i < n_amps; i++) {
				double t = pulse_t + i * pulse_dt;
				adc_counter++;

				// Measure amplitudes for ADC
				if (adc_counter == sample_tick) {
					raw_sample.setADC(idx_sample, pulse.getAmplitude()[i]);
					adc_counter = 0;
					if (toa_pending) {
						idx_toa = idx_sample;
						raw_sample.setTOA(idx_toa, t - t_upcross);
						toa_pending = false;
					}
					idx_sample++;
				}

				// Measure up-crossing time for TOA
				if (!toa_pending && pulse.getAmplitude()[i] > m_cfg.toa_thres) {
					t_upcross = get_crossing_time(m_cfg.toa_thres, pulse_dt, t, pulse.getAmplitude()[i],
							      	      pulse.getAmplitude()[i - 1]);
					toa_pending = true;
					tot_progress = true;
				}

				// Measure down-crossing time for TOT
				if (tot_progress && pulse.getAmplitude()[i] < m_cfg.tot_thres) {
					raw_sample.setTOT(idx_toa,
							get_crossing_time(m_cfg.tot_thres, pulse_dt, t, pulse.getAmplitude()[i],
								pulse.getAmplitude()[i - 1]) - t_upcross);
					tot_progress = false;
				}
			}

			// Fill HGCROCSamples and RawHGCROCHit
			auto out_digi_hit = out_digi_hits->create();
			out_digi_hit.setCellID(pulse.getCellID());
			out_digi_hit.setSamplePhase(std::llround(m_cfg.adc_phase / m_cfg.dyRangeTOA * m_cfg.capTOA));
			out_digi_hit.setTimeStamp(time_stamp_first);

			const auto& entries = raw_sample.getEntries();

			for (const auto& entry : entries) {
				edm4eic::HGCROCSample sample;
				auto adc   = std::max(std::llround(entry.adc / m_cfg.dyRangeHighGainADC * m_cfg.capHighGainADC), 0LL);
				sample.ADC = adc > m_cfg.capHighGainADC ? m_cfg.capHighGainADC : adc;
				auto toa   = std::max(std::llround(entry.toa / m_cfg.dyRangeTOA * m_cfg.capTOA), 0LL);
				sample.timeOfArrival = toa > m_cfg.capTOA ? m_cfg.capTOA : toa;
				auto tot = std::max(std::llround(entry.tot / m_cfg.dyRangeTOT * m_cfg.capTOT), 0LL);
				sample.timeOverThreshold = tot > m_cfg.capTOT ? m_cfg.capTOT : tot;
				out_digi_hit.addToSamples(sample);
			}
		}
	} // PulseDigi:process

	double PulseDigi::get_crossing_time(double thres, double dt, double t, double amp1,
			double amp2) const {
		double numerator   = (thres - amp2) * dt;
		double denominator = amp2 - amp1;
		double added       = t;
		return (numerator / denominator) + added;
	}
} // namespace eicrecon
