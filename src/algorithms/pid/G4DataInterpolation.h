//
// Copyright 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//
// GEANT G4DataInterpolation emulation class
//

#include <vector>

#ifndef _G4_DATA_INTERPOLATION_
#define _G4_DATA_INTERPOLATION_

class G4DataInterpolation {
public:
  G4DataInterpolation(const double WL[], const double QE[], unsigned dim) : m_LookupTableStep(0.0) {
    for (unsigned iq = 0; iq < dim; iq++)
      m_RawData.push_back(std::make_pair(WL[iq], QE[iq]));
  };
  ~G4DataInterpolation() {};

  enum order { ZeroOrder, FirstOrder };

  void CreateLookupTable(unsigned nbins);

  bool WithinRange(double x) const;
  double GetInterpolatedValue(double x, G4DataInterpolation::order order) const;

private:
  std::vector<std::pair<double, double>> m_RawData;

  double m_LookupTableStep;
  std::vector<std::pair<double, double>> m_LookupTable;
};

#endif
