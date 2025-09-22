//
// Copyright 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <map>
#include <math.h>
#include <stdio.h>

#include "G4DataInterpolation.h"

// -------------------------------------------------------------------------------------

void G4DataInterpolation::CreateLookupTable(unsigned nbins)
{
  // Just in case somebody considers to call this method twice :-);
  m_LookupTable.clear();
  
  // FIXME: could have probably just reordered the initial vector;
  std::map<double, double> buffer;

  for(auto entry: m_RawData)
    buffer[entry.first] = entry.second;

  // Sanity checks; return empty map in case do not pass them;
  if (buffer.size() < 2 || nbins < 2) return;

  double from = (*buffer.begin()).first;
  double to   = (*buffer.rbegin()).first;
  // Will be "nbins+1" equidistant entries;
  m_LookupTableStep = (to - from) / nbins;

  for(auto entry: buffer) {
    double e1 = entry.first;
    double qe1 = entry.second;

    if (!m_LookupTable.size())
      m_LookupTable.push_back(std::make_pair(e1, qe1));
    else {
      const auto &prev = m_LookupTable[m_LookupTable.size()-1];

      double e0 = prev.first;
      double qe0 = prev.second;
      double a = (qe1 - qe0) / (e1 - e0);
      double b = qe0 - a*e0;
      // FIXME: check floating point accuracy when moving to a next point; do we actually 
      // care whether the overall number of bins will be "nbins+1" or more?;
      for(double e = e0+m_LookupTableStep; e<e1; e+=m_LookupTableStep)
	m_LookupTable.push_back(std::make_pair(e, a*e + b));
    } //if
  } //for entry
  
  //for(auto entry: m_LookupTable) 
  //printf("@L@ %7.2f -> %7.2f\n", entry.first, entry.second);
} // G4DataInterpolation::CreateLookupTable()

// -------------------------------------------------------------------------------------

bool G4DataInterpolation::WithinRange(double x) const
{
  unsigned dim = m_LookupTable.size();

  // FIXME: assume >= and <=, then correct ibin for edge argument values if needed;
  return (m_LookupTableStep > 0.0 && dim >= 2 && x >= m_LookupTable[0].first && x <= m_LookupTable[dim-1].first); 
} // G4DataInterpolation::WithinRange()

// -------------------------------------------------------------------------------------

double G4DataInterpolation::GetInterpolatedValue(double x, G4DataInterpolation::order order) const
{
  // Well, in fact user is supposed to check this condition anyway;
  if (!WithinRange(x)) return 0.0;
  
  // Get the tabulated table reference; perform sanity checks;
  unsigned tdim = m_LookupTable.size();
  int ibin = (int)floor((x - m_LookupTable[0].first) / m_LookupTableStep);

  //printf("@Q@ %f vs %f, %f -> %d\n", argument, from.first, to.first, ibin);
  
  // Out of range check to fix pathological cases;
  if (ibin < 0)          ibin = 0;
  if (ibin >= int(tdim)) ibin = tdim-1;
      
  switch (order) {
  case G4DataInterpolation::ZeroOrder:
    return m_LookupTable[ibin].second;
  case G4DataInterpolation::FirstOrder:
    {
      if (ibin == tdim-1) ibin--;
      double x1 = m_LookupTable[ibin].first, dx = x - x1;
      double y1 = m_LookupTable[ibin].second, y2 = m_LookupTable[ibin+1].second;

      return y1 + (y2-y1)*(dx/m_LookupTableStep);
    }
  default:
    return 0.0;
  } //switch
} // G4DataInterpolation::GetInterpolatedValue()

// -------------------------------------------------------------------------------------

