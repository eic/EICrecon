// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, Dmitry Kalinkin, Simon Gardner

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
  int getValue() { return value; }
  int getValueConst() const { return value; }
  float getFValue() const { return fvalue; }
  double getDValue() const { return dvalue; }
  bool isEven() const { return value % 2 == 0; }
  bool isPositive() const { return value > 0; }
};

struct Inner {
  int id;
  int getId() const { return id; }
  int getSign() const { return id >= 0 ? 1 : -1; }
};

struct Outer {
  int value;
  Inner inner;
  int getValue() const { return value; }
  const Inner& getInner() const { return inner; }
};

TEST_CASE("RangeSplit works with int and const/non-const member functions", "[SubDivideFunctors]") {
  RangeSplit<&Dummy::getValue> split1({{0, 4}, {5, 10}});
  RangeSplit<&Dummy::getValueConst> split2({{0, 4}, {5, 10}});
  Dummy d1{3}, d2{7};
  REQUIRE(split1(d1) == std::vector<size_t>{0});
  REQUIRE(split1(d2) == std::vector<size_t>{1});
  REQUIRE(split2(d1) == std::vector<size_t>{0});
  REQUIRE(split2(d2) == std::vector<size_t>{1});
}

TEST_CASE("RangeSplit works with float and double", "[SubDivideFunctors]") {
  RangeSplit<&Dummy::getFValue> splitF({{0.0f, 1.5f}, {2.0f, 3.0f}});
  RangeSplit<&Dummy::getDValue> splitD({{0.0, 1.5}, {2.0, 3.0}});
  Dummy d1{0, 1.0f, 2.5};
  Dummy d2{0, 2.5f, 1.0};
  REQUIRE(splitF(d1) == std::vector<size_t>{0});
  REQUIRE(splitF(d2) == std::vector<size_t>{1});
  REQUIRE(splitD(d1) == std::vector<size_t>{1});
  REQUIRE(splitD(d2) == std::vector<size_t>{0});
}

TEST_CASE("RangeSplit works with outside ranges", "[SubDivideFunctors]") {
  RangeSplit<&Dummy::getValue> split({{0, 4}, {5, 10}}, {false, true});
  Dummy d1{3}, d2{7}, d3{11};
  REQUIRE(split(d1) == std::vector<size_t>{1});
  REQUIRE(split(d2) == std::vector<size_t>{0});
  REQUIRE(split(d3) == std::vector<size_t>{0});
}

TEST_CASE("RangeSplit with bool inside array", "[SubDivideFunctors]") {
  RangeSplit<&Dummy::getValue> split({{0, 4}, {5, 10}});
  Dummy d1{3}, d2{7}, d3{11};
  REQUIRE(split(d1) == std::vector<size_t>{0});
  REQUIRE(split(d2) == std::vector<size_t>{1});
  REQUIRE(split(d3) == std::vector<size_t>{1});
}

TEST_CASE("ValueSplit works with int, float, and double", "[SubDivideFunctors]") {
  ValueSplit<&Dummy::getValue> splitI({{1}, {2}, {3}});
  ValueSplit<&Dummy::getFValue> splitF({{1.0f}, {2.5f}});
  ValueSplit<&Dummy::getDValue> splitD({{1.0}, {2.5}});
  Dummy d{2, 2.5f, 1.0};
  REQUIRE(splitI(d) == std::vector<size_t>{1});
  REQUIRE(splitF(d) == std::vector<size_t>{1});
  REQUIRE(splitD(d) == std::vector<size_t>{0});
}

TEST_CASE("RangeSplit works with explicit chain accessors", "[SubDivideFunctors]") {
  RangeSplit<Chain<&Outer::getInner, &Inner::getId>> split({{0, 5}, {5, 10}});
  Outer o1{0, {3}};
  Outer o2{0, {7}};
  REQUIRE(split(o1) == std::vector<size_t>{0});
  REQUIRE(split(o2) == std::vector<size_t>{1});
}

TEST_CASE("ValueSplit mixes chain and non-chain accessors", "[SubDivideFunctors]") {
  ValueSplit<&Outer::getValue, Chain<&Outer::getInner, &Inner::getSign>> split(
      {{1, 1}, {2, -1}});
  Outer o1{1, {4}};
  Outer o2{2, {-3}};
  REQUIRE(split(o1) == std::vector<size_t>{0});
  REQUIRE(split(o2) == std::vector<size_t>{1});
}

TEST_CASE("Edge cases: empty ranges and values not matching", "[SubDivideFunctors]") {
  RangeSplit<&Dummy::getValue> split({});
  Dummy d{42};
  REQUIRE(split(d).empty());

  ValueSplit<&Dummy::getValue> splitV({{1}, {2}});
  REQUIRE(splitV(d).empty());
}
