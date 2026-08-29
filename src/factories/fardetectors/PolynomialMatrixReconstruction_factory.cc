// Created by Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//

// Precompile library: explicit instantiation of PolynomialMatrixReconstruction_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "PolynomialMatrixReconstruction_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::PolynomialMatrixReconstruction_factory, eicrecon::PolynomialMatrixReconstructionConfig>;
