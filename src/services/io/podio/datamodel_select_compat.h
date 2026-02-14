// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

// Note that the JANA version components are defined by the build system,
// since they are not available for use in preprocessor directives when included
// from JANA/JVersion.h.

// Use modern implementation for JANA2 >= 2.4.3
#if (JANA_VERSION_MAJOR > 2) || (JANA_VERSION_MAJOR == 2 && JANA_VERSION_MINOR > 4) ||             \
    (JANA_VERSION_MAJOR == 2 && JANA_VERSION_MINOR == 4 && JANA_VERSION_PATCH >= 3)
#define USE_MODERN_PODIO_GLUE 1
#endif
