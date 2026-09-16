// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#include "NoPayloadDb_service.h"

#include <string>

NoPayloadDb_service::NoPayloadDb_service(JApplication* app)
    : m_client("EICrecon"), m_application(app) {}
