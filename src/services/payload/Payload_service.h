// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#pragma once

#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>
#include <nlohmann/json.hpp>
#include <nopayloadclient/nopayloadclient.hpp>
#include <string>

class Payload_service : public JService
{
public:
    explicit Payload_service(JApplication *app);
    ~Payload_service() { };

private:

    Payload_service() = default;

    nopayloadclient::NoPayloadClient m_client;

    JApplication* m_application;
};
