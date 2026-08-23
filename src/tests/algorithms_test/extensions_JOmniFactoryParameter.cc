// SPDX-License-Identifier: LGPL-3.0-or-later

#include "extensions/jana/JOmniFactory.h"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
class DummyOmniFactory : public JOmniFactory<DummyOmniFactory> {
public:
  Parameter<int> m_int_parameter{this, "test_int", 42, "test integer parameter"};
  Parameter<std::string> m_string_parameter{
      this, "test_string", std::string("default"), "test string parameter"};

  void Configure() {}
};
} // namespace

TEST_CASE("JOmniFactory Parameter stores default values") {
  DummyOmniFactory factory;
  REQUIRE(factory.m_int_parameter() == 42);
  REQUIRE(factory.m_string_parameter() == "default");
}
