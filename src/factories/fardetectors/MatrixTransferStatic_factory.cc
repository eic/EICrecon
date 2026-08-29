// Created by Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//

// Precompile library: explicit instantiation of MatrixTransferStatic_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "MatrixTransferStatic_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::MatrixTransferStatic_factory, eicrecon::MatrixTransferStaticConfig>;
