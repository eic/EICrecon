// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Simon Gardner

#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <tuple>
#include <array>
#include <cmath>
#include "algorithms/meta/SubDivideFunctors.h"

using namespace eicrecon;

struct Dummy {
  int value;
  float fvalue;
  double dvalue;
  int getValue() const { return value; }
  float getFValue() const { return fvalue; }
  double getDValue() const { return dvalue; }
};

struct Wrapper {
  int value;
  Dummy dummy;
  int getValue() const { return value; }
  const Dummy& getDummy() const { return dummy; }
};

TEST_CASE("RangeSplit works with double values", "[SubDivideFunctors]") {
  RangeSplit<Chain<&Dummy::getDValue>> split({{0.0, 4.5}, {5.0, 10.0}});
  Dummy d1{0, 0.0f, 3.5};
  Dummy d2{0, 0.0f, 7.0};
  REQUIRE(split(d1) == std::vector<std::size_t>{0});
  REQUIRE(split(d2) == std::vector<std::size_t>{1});
}

TEST_CASE("RangeSplit excludes boundaries", "[SubDivideFunctors]") {
  RangeSplit<Chain<&Dummy::getDValue>> split({{0.0, 4.0}, {5.0, 10.0}});
  Dummy d1{0, 0.0f, 0.0};
  Dummy d2{0, 0.0f, 4.0};
  Dummy d3{0, 0.0f, 5.0};
  Dummy d4{0, 0.0f, 10.0};
  REQUIRE(split(d1).empty());
  REQUIRE(split(d2).empty());
  REQUIRE(split(d3).empty());
  REQUIRE(split(d4).empty());
}

TEST_CASE("RangeSplit works with chained accessors", "[SubDivideFunctors]") {
  RangeSplit<Chain<&Wrapper::getDummy, &Dummy::getDValue>> split({{0.0, 5.0}, {5.0, 10.0}});
  Wrapper w1{0, Dummy{0, 0.0f, 3.5}};
  Wrapper w2{0, Dummy{0, 0.0f, 7.0}};
  REQUIRE(split(w1) == std::vector<std::size_t>{0});
  REQUIRE(split(w2) == std::vector<std::size_t>{1});
}

TEST_CASE("ValueSplit with single member function", "[SubDivideFunctors]") {
  ValueSplit<&Wrapper::getValue> split({{1}, {2}});
  Wrapper o1{1, Dummy{4, 0.0f, 0.0}};
  Wrapper o2{2, Dummy{-3, 0.0f, 0.0}};
  REQUIRE(split(o1) == std::vector<std::size_t>{0});
  REQUIRE(split(o2) == std::vector<std::size_t>{1});
}

TEST_CASE("Edge cases: empty ranges and values not matching", "[SubDivideFunctors]") {
  RangeSplit<Chain<&Dummy::getDValue>> split({});
  Dummy d{0, 0.0f, 42.0};
  REQUIRE(split(d).empty());

  ValueSplit<&Wrapper::getValue> splitV({{1}, {2}});
  Wrapper w{42, Dummy{0, 0.0f, 0.0}};
  REQUIRE(splitV(w).empty());
}
