// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Dmitry Kalinkin

#pragma once

namespace eicrecon {

  // This allows to to have a trial of https://github.com/eic/EDM4eic/pull/35
  // Shoult it be a success, we will re-export the edm4hep::unit namespace from
  // here.

  namespace unit {

    //
    // This provides definition for a unique unit system for this EDM. It is
    // neither Geant, nor TGeo. The DD4hep, by default, uses a system matching
    // the TGeo one.
    //
    // +------------+---------------------|-----------------+
    // | Geant      | TGeo                | EDM4hep/EDM4eic |
    // +------------+---------------------|-----------------+
    // | milimeter  | centimeter          | milimeter       |
    // | nanosecond | second              | nanosecond      |
    // | MeV        | GeV                 | GeV             |
    // +------------+---------------------|-----------------+
    //

    // distance
    static constexpr double mm = 1.0; // millimeter
    static constexpr double cm = 1e1 * mm; // centimeter
    static constexpr double micrometer = 1e-3 * mm; // micrometer
    static constexpr double nm = 1e-6 * mm; // micrometer
    static constexpr double m = 1e3 * mm; // meter

    // time
    static constexpr double ns = 1.0; // nano second
    static constexpr double second = 1e9 * ns; // second
    static constexpr double milisecond = 1e6 * ns; // milisecond
    static constexpr double microsecond = 1e3 * ns; // milisecond
    static constexpr double picosecond = 1e-3 * ns; // pico second

    // energy
    static constexpr double GeV = 1.0; // giga electron volt
    static constexpr double MeV = 1e-3 * GeV; // mega electron volt
    static constexpr double keV = 1e-6 * GeV; // kilo electron volt
    static constexpr double eV = 1e-9 * GeV; // electron volt

  }

}
