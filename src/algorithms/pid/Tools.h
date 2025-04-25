// Copyright 2023, Alexander Kiselev, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
// Common functions for PID algorithms
// Several methods ported from Juggler's JugPID `IRTAlgorithmServices`
//

#pragma once

// general
#include <map>
#include <math.h>
#include <algorithms/logger.h>

// ROOT
#include <TVector3.h>

// DD4hep
#include <Evaluator/DD4hepUnits.h>

// data model
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4hep/ParticleIDCollection.h>

namespace eicrecon {

// Tools class, filled with miscellaneous helper functions
template <class T> class Tools {
private:
  T* m_logger{nullptr};

public:
  Tools(T* logger) : m_logger(logger){};

  // -------------------------------------------------------------------------------------

  // h*c constant, for wavelength <=> energy conversion [GeV*nm]
  static constexpr double HC = dd4hep::h_Planck * dd4hep::c_light / (dd4hep::GeV * dd4hep::nm);

  // -------------------------------------------------------------------------------------
  // Radiator IDs

  std::unordered_map<int, std::string> GetRadiatorIDs() const {
    return std::unordered_map<int, std::string>{{0, "Aerogel"}, {1, "Gas"}};
  }

  std::string GetRadiatorName(int id) const {
    std::string name;
    try {
      name = GetRadiatorIDs().at(id);
    } catch (const std::out_of_range& e) {
      throw std::runtime_error(fmt::format(
          "RUNTIME ERROR: unknown radiator ID={} in algorithms/pid/Tools::GetRadiatorName", id));
    }
    return name;
  }

  int GetRadiatorID(std::string name) const {
    for (auto& [id, name_tmp] : GetRadiatorIDs())
      if (name == name_tmp)
        return id;
    throw std::runtime_error(fmt::format(
        "RUNTIME ERROR: unknown radiator '{}' in algorithms/pid/Tools::GetRadiatorID", name));
    return -1;
  }

  // -------------------------------------------------------------------------------------
  // Table rebinning and lookup

  // Rebin input table `input` to have `nbins+1` equidistant bins; returns the rebinned table
  std::vector<std::pair<double, double>>
  ApplyFineBinning(const std::vector<std::pair<double, double>>& input, unsigned nbins) const {
    std::vector<std::pair<double, double>> ret;

    // Well, could have probably just reordered the initial vector;
    std::map<double, double> buffer;

    for (auto entry : input)
      buffer[entry.first] = entry.second;

    // Sanity checks; return empty map in case do not pass them;
    if (buffer.size() < 2 || nbins < 2)
      return ret;

    double from = (*buffer.begin()).first;
    double to   = (*buffer.rbegin()).first;
    // Will be "nbins+1" equidistant entries;
    double step = (to - from) / nbins;

    for (auto entry : buffer) {
      double e1  = entry.first;
      double qe1 = entry.second;

      if (!ret.size())
        ret.push_back(std::make_pair(e1, qe1));
      else {
        const auto& prev = ret[ret.size() - 1];

        double e0  = prev.first;
        double qe0 = prev.second;
        double a   = (qe1 - qe0) / (e1 - e0);
        double b   = qe0 - a * e0;
        // FIXME: check floating point accuracy when moving to a next point; do we actually
        // care whether the overall number of bins will be "nbins+1" or more?;
        for (double e = e0 + step; e < e1; e += step)
          ret.push_back(std::make_pair(e, a * e + b));
      } //if
    }   //for entry

    return ret;
  }

  // Find the bin in table `table` that contains entry `argument` in the first column and
  // sets `entry` to the corresponding element of the second column; returns true if successful
  bool GetFinelyBinnedTableEntry(const std::vector<std::pair<double, double>>& table,
                                 double argument, double* entry) const {
    // Get the tabulated table reference; perform sanity checks;
    //const std::vector<std::pair<double, double>> &qe = u_quantumEfficiency.value();
    unsigned dim = table.size();
    if (dim < 2)
      return false;

    // Find a proper bin; no tricks, they are all equidistant;
    auto const& from = table[0];
    auto const& to   = table[dim - 1];
    double emin      = from.first;
    double emax      = to.first;
    double step      = (emax - emin) / (dim - 1);
    int ibin         = (int)floor((argument - emin) / step);

    //printf("%f vs %f, %f -> %d\n", ev, from.first, to. first, ibin);

    // Out of range check;
    if (ibin < 0 || ibin >= int(dim))
      return false;

    *entry = table[ibin].second;
    return true;
  }

  // -------------------------------------------------------------------------------------
  // convert PODIO vector datatype to ROOT TVector3
  template <class PodioVector3> TVector3 PodioVector3_to_TVector3(const PodioVector3 v) const {
    return TVector3(v.x, v.y, v.z);
  }
  // convert ROOT::Math::Vector to ROOT TVector3
  template <class MathVector3> TVector3 MathVector3_to_TVector3(MathVector3 v) const {
    return TVector3(v.x(), v.y(), v.z());
  }

  // -------------------------------------------------------------------------------------

  // printing: vectors
  template <algorithms::LogLevel lvl = algorithms::LogLevel::kTrace>
  void PrintTVector3(std::string name, TVector3 vec, int nameBuffer = 30) const {
    m_logger->template log<lvl>("{:>{}} = ( {:>10.2f} {:>10.2f} {:>10.2f} )", name, nameBuffer,
                                vec.x(), vec.y(), vec.z());
  }

  // printing: hypothesis tables
  template <algorithms::LogLevel lvl = algorithms::LogLevel::kTrace>
  void PrintHypothesisTableHead(int indent = 4) const {
    m_logger->template log<lvl>("{:{}}{:>6}  {:>10}  {:>10}", "", indent, "PDG", "Weight", "NPE");
  }
  template <algorithms::LogLevel lvl = algorithms::LogLevel::kTrace>
  void PrintHypothesisTableLine(edm4eic::CherenkovParticleIDHypothesis hyp, int indent = 4) const {
    m_logger->template log<lvl>("{:{}}{:>6}  {:>10.8}  {:>10.8}", "", indent, hyp.PDG, hyp.weight,
                                hyp.npe);
  }
  template <algorithms::LogLevel lvl = algorithms::LogLevel::kTrace>
  void PrintHypothesisTableLine(edm4hep::ParticleID hyp, int indent = 4) const {
    float npe =
        hyp.parameters_size() > 0 ? hyp.getParameters(0) : -1; // assume NPE is the first parameter
    m_logger->template log<lvl>("{:{}}{:>6}  {:>10.8}  {:>10.8}", "", indent, hyp.getPDG(),
                                hyp.getLikelihood(), npe);
  }

  // printing: Cherenkov angle estimate
  template <algorithms::LogLevel lvl = algorithms::LogLevel::kTrace>
  void PrintCherenkovEstimate(edm4eic::CherenkovParticleID pid, bool printHypotheses = true,
                              int indent = 2) const {
    if (m_logger->level() <= lvl) {
      double thetaAve = 0;
      if (pid.getNpe() > 0)
        for (const auto& [theta, phi] : pid.getThetaPhiPhotons())
          thetaAve += theta / pid.getNpe();
      m_logger->template log<lvl>("{:{}}Cherenkov Angle Estimate:", "", indent);
      m_logger->template log<lvl>("{:{}}  {:>16}:  {:>10}", "", indent, "NPE", pid.getNpe());
      m_logger->template log<lvl>("{:{}}  {:>16}:  {:>10.8} mrad", "", indent, "<theta>",
                                  thetaAve * 1e3); // [rad] -> [mrad]
      m_logger->template log<lvl>("{:{}}  {:>16}:  {:>10.8}", "", indent, "<rindex>",
                                  pid.getRefractiveIndex());
      m_logger->template log<lvl>("{:{}}  {:>16}:  {:>10.8} eV", "", indent, "<energy>",
                                  pid.getPhotonEnergy() * 1e9); // [GeV] -> [eV]
      if (printHypotheses) {
        m_logger->template log<lvl>("{:{}}Mass Hypotheses:", "", indent);
        PrintHypothesisTableHead(indent + 2);
        for (const auto& hyp : pid.getHypotheses())
          PrintHypothesisTableLine(hyp, indent + 2);
      }
    }
  }

}; // class Tools
} // namespace eicrecon
