// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JServiceFwd.h>
#include <nopayloadclient/nopayloadclient.hpp>

class NoPayloadDb_service : public JService {
public:
  explicit NoPayloadDb_service(JApplication* app);
  ~NoPayloadDb_service() {};

private:
  NoPayloadDb_service() = default;

  nopayloadclient::NoPayloadClient m_client;
};
