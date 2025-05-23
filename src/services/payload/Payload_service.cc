// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#include "Payload_service.h"

#include <string>

Payload_service::Payload_service(JApplication* app) : m_client("EICrecon") { m_application = app; }
