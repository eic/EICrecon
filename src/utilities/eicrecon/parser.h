// Copyright (C) 2022, 2023 Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <string>
#include <memory>

// DD4Hep
#include <Evaluator/Evaluator.h>
#include <Evaluator/detail/Evaluator.h>

namespace jana::parser {

  // parsing result template
  template<class ResultType>
    struct Result {
      bool success;
      ResultType result;
    };

  // parsing class
  class Parser {
    public:

      /* constructor
       * - `debug` to enable debugging printouts
       * - `min_precision` controls the minimum precision for re-stringified `double`s
       *   (precision can be higher than `min_precision` if the user asks for it)
       */
      Parser(bool debug=false, int min_precision=6);
      ~Parser() {}

      /* evaluate DD4hep expression `expr` and return a string
       * - if `expr` is itself a string, return `expr` silently
       * - this is useful for parsing CLI options
       * - optionally specify `key` to clarify debugging printouts
       */
      Result<std::string> dd4hep_to_string(const std::string& expr, const std::string& key="");

      /* get a constant from the internal dictionary, viz. for unit conversion
       * - to convert global unit system (specified in constructor) to preferred units,
       *   divide by the return value of this method
       *   - example: to convert 1000 global length units (`mm`, if EDM4hep) to `m`, call `1000 / units("m")`
       * - returns 1.0 upon failure (and an error message to stderr)
       */
      double units(const std::string& expr);

    private:
      std::unique_ptr<dd4hep::tools::Evaluator::Object> m_eval;
      bool m_debug;
      int m_min_precision;
  };

}
