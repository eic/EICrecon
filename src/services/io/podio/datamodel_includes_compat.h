// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck
//
// Datamodel includes compatibility header
// For podio >= 1.3, use umbrella headers. For older versions, this includes
// the Python-generated file.

#pragma once

#include <podio/podioVersion.h>

#if defined(podio_VERSION_MAJOR) && defined(podio_VERSION_MINOR)
#if (podio_VERSION_MAJOR > 1) || (podio_VERSION_MAJOR == 1 && podio_VERSION_MINOR >= 3)
#define USE_MODERN_PODIO_GLUE 1
#endif
#endif

#ifdef USE_MODERN_PODIO_GLUE
// Use umbrella headers for modern podio
#include "services/io/podio/datamodel_includes.h"
#else
// Legacy Python-generated includes
#include "services/io/podio/datamodel_includes_legacy.h"
#endif
