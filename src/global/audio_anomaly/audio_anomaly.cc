// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 EIC Reconstruction Contributors

#include <JANA/JApplication.h>
#include <services/audio_anomaly/AudioAnomalyDetection_service.h>
#include <factories/anomaly/AnomalyDetection_factory.h>
#include <string>

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  // Register the audio anomaly detection service
  app->ProvideService(std::make_shared<AudioAnomalyDetection_service>(app));

  // Register the anomaly detection factory
  app->Add(new eicrecon::AnomalyDetection_factory());
}
}
