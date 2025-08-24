// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#pragma once

#include <JANA/JApplication.h>
#include <JANA/JServiceFwd.h>
#include <nopayloadclient/nopayloadclient.hpp>

class Payload_service : public JService {
public:
  explicit Payload_service(JApplication* app);
  ~Payload_service(){};

private:
  Payload_service() = default;

  nopayloadclient::NoPayloadClient m_client;

  JApplication* m_application;
};
