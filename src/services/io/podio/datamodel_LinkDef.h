// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//
// This is used when generating the ROOT dictionary file needed to define
// the vector types like "vector<edm4hep::EventHeader>".
//


#include <vector>
#ifdef __ROOTCLING__

#pragma link C++ class vector<edm4hep::*>+;

#endif
