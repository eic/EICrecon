// Copyright (C) 2023 Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>
#include <functional>
#include <sstream>
#include <iomanip>

#include "parser.h"

// settings
// ---------------------------------------------
const int    PRECISION      = 9;
const double DIFF_THRESHOLD = std::pow(10, -PRECISION);

// common methods
// ---------------------------------------------

// generate a test parameter name, with incrementing counter
std::string KeyName(int &i, std::string key="test:param") {
  return key + std::to_string(i++);
}

// run a test and print results
bool RunTest(
    std::shared_ptr<jana::parser::Parser> val_parser,
    std::string key,
    std::string val_in,
    std::string val_exp,
    std::function<bool(std::string,std::string)> check
    )
{
  fmt::print("TEST {}:\n", key);
  auto val_parsed = val_parser->dd4hep_to_string(val_in, key);
  if(!val_parsed.success) return false;
  auto val_out = val_parsed.result;
  auto passed  = check(val_out, val_exp);
  fmt::print("  RESULT:\n{:>20}: '{}'\n{:>20}: '{}'\n{:>20}: '{}'\n{:>20}: {}\n\n",
      "Input",       val_in,
      "Output",      val_out,
      "Expectation", val_exp,
      "Passed",      passed
      );
  return passed;
}

// tests
// ---------------------------------------------
TEST_CASE("Parameter Parsing", "[param_parser]" ) {

  // instantiate Parser
  auto m_parser = std::make_shared<jana::parser::Parser>();

  // --------------------------------------------
  SECTION("unit system is EDM4hep") {
    REQUIRE(std::abs( m_parser->units("mm")     - 1.0 ) < DIFF_THRESHOLD );
    REQUIRE(std::abs( m_parser->units("GeV")    - 1.0 ) < DIFF_THRESHOLD );
    REQUIRE(std::abs( m_parser->units("ns")     - 1.0 ) < DIFF_THRESHOLD );
    REQUIRE(std::abs( m_parser->units("radian") - 1.0 ) < DIFF_THRESHOLD );
  }

  // --------------------------------------------
  SECTION("unit conversions from EDM4hep to specified units") {
    REQUIRE(std::abs( 1.0          / m_parser->units("GeV")   - 1.0    ) < DIFF_THRESHOLD ); // [GeV] -> [GeV]
    REQUIRE(std::abs( 1.0          / m_parser->units("keV")   - 1.0e+6 ) < DIFF_THRESHOLD ); // [GeV] -> [keV]
    REQUIRE(std::abs( 1.0          / m_parser->units("s")     - 1.0e-9 ) < DIFF_THRESHOLD ); // [ns] -> [s]
    REQUIRE(std::abs( 1.0          / m_parser->units("nm")    - 1.0e+6 ) < DIFF_THRESHOLD ); // [mm] -> [nm]
    REQUIRE(std::abs( 1240/1e9/1e6 / m_parser->units("eV*nm") - 1240.0 ) < DIFF_THRESHOLD ); // [GeV*mm] -> [eV*nm]
  }

  // --------------------------------------------
  SECTION("parse numbers, units, and math") {

    // implementation: check if difference between parsed number and expected number is less than a threshold
    int    i              = 1;
    auto test_number = [&] (std::string val_in, double val_exp) {
      std::stringstream val_exp_stream;
      val_exp_stream << std::setprecision(PRECISION) << val_exp;
      auto val_exp_str = val_exp_stream.str();
      return RunTest(
          m_parser,
          KeyName(i, "test:param:number"),
          val_in,
          val_exp_str,
          [&] (std::string a, std::string b) { return std::abs(std::stod(a) - std::stod(b)) < DIFF_THRESHOLD; }
          );
    };

    // tests
    REQUIRE(test_number( "7",          7            ));
    REQUIRE(test_number( "7*6",        42           ));
    REQUIRE(test_number( "1-10/2",     -4           ));
    REQUIRE(test_number( "1*mm",       1            ));
    REQUIRE(test_number( "1*m",        1000         ));
    REQUIRE(test_number( "5*GeV",      5            ));
    REQUIRE(test_number( "5e+3*MeV",   5            ));
    REQUIRE(test_number( "5*keV",      5e-6         ));
    REQUIRE(test_number( "8*ns",       8            ));
    REQUIRE(test_number( "8*s",        8e+9         ));
    REQUIRE(test_number( "1240*eV*nm", 1240/1e9/1e6 ));

  }

  // --------------------------------------------
  SECTION("parse strings") {

    // implementation: check if the string remains the same
    int i = 1;
    auto test_string = [&] (std::string val_in) {
      return RunTest(
          m_parser,
          KeyName(i, "test:param:string"),
          val_in,
          val_in, // (val_exp, checking if it's the same)
          [&] (std::string a, std::string b) { return a==b; }
          );
    };

    // tests
    REQUIRE(test_string( "test_string"               ));
    REQUIRE(test_string( "this,string,is,not,a,list" )); // this string should not be parsed as a list of numbers
    REQUIRE(test_string( ""                          )); // empty string
    REQUIRE(test_string( "1240*ev*nm"                )); // typo in units (ev); should be parsed as string

  }

  // --------------------------------------------
  SECTION("parse lists") {

    // implementation: check if the numerical value difference of each list element is less than threshold
    int    i              = 1;
    auto test_list = [&] (std::string val_in, std::string val_exp) {
      auto compare_lists = [&] (std::string a, std::string b) {
        auto tokenize = [] (std::string str) {
          std::istringstream  token_stream(str);
          std::string         token;
          std::vector<double> token_list;
          while(getline(token_stream, token, ','))
            token_list.push_back(std::stod(token));
          return token_list;
        };
        auto a_vals = tokenize(a);
        auto b_vals = tokenize(b);
        if(a_vals.size() != b_vals.size()) {
          fmt::print(stderr,"  ERROR: lists have differing sizes\n");
          return false;
        }
        for(unsigned k=0; k<a_vals.size(); k++) {
          fmt::print("  compare element {}: {} vs. {}\n", k, a_vals.at(k), b_vals.at(k));
          if( std::abs(a_vals.at(k) - b_vals.at(k)) > DIFF_THRESHOLD )
            return false;
        }
        return true;
      };
      return RunTest(
          m_parser,
          KeyName(i, "test:param:list"),
          val_in,
          val_exp,
          compare_lists
          );
    };

    // tests
    REQUIRE(test_list( "1,2,3,4,5",           "1,2,3,4,5"    ));
    REQUIRE(test_list( "5e+3*MeV,8*s,6*7,11", "5,8e+9,42,11" ));

  }

}
